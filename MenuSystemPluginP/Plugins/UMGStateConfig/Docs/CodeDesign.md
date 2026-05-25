# UMGStateConfig 插件代码设计文档

> 更新时间：2026-05-25  
> 适用范围：`Plugins/UMGStateConfig`  
> 文档定位：本文解释当前实现结构和关键设计。需求 / 验收口径见 `openspec/specs/cpp-umg-state-config/spec.md`。

## 1. 插件目标

`UMGStateConfig` 用于在 `Widget Blueprint` 编辑器里为 UMG 控件配置 UI 状态，并在运行时通过：

```cpp
UUMGStateConfigFunctionLibrary::ApplyUIState(TargetWidget, StateGroupName, StateName)
```

把指定状态应用到目标 `UUserWidget` 实例。

当前实现的核心思路：

- 配置数据保存在 `Widget Blueprint Extension` 中，随蓝图资产保存。
- 蓝图编译时，把编辑期配置复制到 `UserWidget` CDO 上的运行时扩展。
- 运行时实例首次调用时，从 CDO 扩展拷贝配置到实例扩展。
- 属性添加和编辑复用 UE 原生 `DetailsView`。
- 属性保存统一走 `SerializedProperty`：保存属性路径、属性类型名、导出文本值和资源软引用。
- 状态切换时先恢复到原始基线，再按活跃状态组重新应用，避免多个状态组互相覆盖后无法恢复。

## 2. 模块划分

| 模块 | 类型 | 主要职责 |
| --- | --- | --- |
| `UMGStateConfigRuntime` | Runtime | 数据结构、蓝图函数、运行时扩展、属性捕获 / 比较 / 应用、资源预加载 |
| `UMGStateConfigEditor` | Editor | Widget Blueprint 入口、状态配置面板、Details 属性捕获、配置保存、校验 |

关键文件：

```text
Plugins/UMGStateConfig/
├── UMGStateConfig.uplugin
├── Docs/CodeDesign.md
└── Source/
    ├── UMGStateConfigRuntime/
    │   ├── Public/
    │   │   ├── UMGStateConfigData.h
    │   │   ├── UMGStateConfigFunctionLibrary.h
    │   │   ├── UMGStateConfigPropertyRuntimeLibrary.h
    │   │   └── UMGStateConfigUserWidgetExtension.h
    │   └── Private/
    │       ├── UMGStateConfigFunctionLibrary.cpp
    │       ├── UMGStateConfigPropertyRuntimeLibrary.cpp
    │       ├── UMGStateConfigRuntimeModule.cpp
    │       ├── UMGStateConfigUserWidgetExtension.cpp
    │       └── Tests/UMGStateConfigRuntimeTests.cpp
    └── UMGStateConfigEditor/
        ├── Public/UMGStateConfigEditorModule.h
        └── Private/
            ├── SUIStateConfigPanel.h/.cpp
            ├── UMGStateConfigBlueprintExtension.h/.cpp
            ├── UMGStateConfigEditorModule.cpp
            ├── UMGStateConfigTabSummoner.h/.cpp
            ├── UMGStateConfigToolbar.h/.cpp
            └── UMGStateConfigValidator.h/.cpp
```

## 3. 数据模型

核心数据定义在 `UMGStateConfigData.h`。

```text
FUMGStateConfigRuntimeData
├── SchemaVersion
├── StateGroups: TArray<FUMGStateConfigGroup>
├── PreviewStateGroupName
└── PreviewStateName

FUMGStateConfigGroup
├── GroupName
├── DisplayName
├── DefaultStateName
├── Priority
├── bExclusiveGroup
└── States: TArray<FUMGStateConfigState>

FUMGStateConfigState
├── StateName
├── DisplayName
├── ConfiguredWidgetNames
└── PropertyChanges: TArray<FUMGStatePropertyChange>

FUMGStatePropertyChange
├── TargetWidgetName
├── ExpectedWidgetClass
├── EditorPath
├── PropertyType = SerializedProperty
└── Value: FUMGStateConfigPropertyValue

FUMGStateConfigPropertyValue
├── SerializedPropertyPath
├── SerializedPropertyTypeName
├── SerializedPropertyValue
└── SerializedReferencedAssets
```

### 3.1 `SerializedProperty`

当前唯一新增配置入口是 `SerializedProperty`。

它表示一条从 UE `DetailsView` 捕获的属性修改：

| 字段 | 含义 |
| --- | --- |
| `SerializedPropertyPath` | 属性路径，例如 `Brush.ResourceObject`、`Font.Size`、`RenderOpacity` |
| `SerializedPropertyTypeName` | 属性 C++ 类型名，用于排查和后续迁移 |
| `SerializedPropertyValue` | `FProperty::ExportTextItem_Direct` 导出的文本值 |
| `SerializedReferencedAssets` | 捕获值中出现的对象 / 软对象资源引用，用于运行时预加载 |

### 3.2 状态组字段

- `Priority`：多个状态组同时活跃时，运行时按数值从低到高重新应用，数值越大越晚覆盖。
- `bExclusiveGroup`：当前是预留字段，尚未参与运行时互斥逻辑。
- `SchemaVersion`：当前版本为 `1`，用于后续数据迁移。

## 4. 编辑期保存与编译复制

### 4.1 编辑期保存

编辑器配置保存在：

```cpp
UUMGStateConfigBlueprintExtension::ConfigData
```

`UUMGStateConfigBlueprintExtension` 继承自 `UWidgetBlueprintExtension`，因此配置跟随 `Widget Blueprint` 资产保存。

面板中修改配置后统一调用：

```cpp
MarkConfigDirty(Extension)
```

该函数负责标记蓝图和扩展对象为已修改、标记包脏，并触发配置面板刷新。

### 4.2 编译复制

`UUMGStateConfigBlueprintExtension::HandleCopyTermDefaultsToDefaultObject` 在 Widget Blueprint 编译复制默认对象时执行：

```text
DefaultWidget
→ RemoveExtensions(UUMGStateConfigUserWidgetExtension)
→ AddExtension(UUMGStateConfigUserWidgetExtension)
→ RuntimeExtension.SetRuntimeData(ConfigData)
```

因此运行时数据来源链路是：

```text
Widget Blueprint 编辑期 ConfigData
→ 编译写入 UserWidget CDO Runtime Extension
→ 运行时实例首次调用时从 CDO Extension 拷贝 RuntimeData
```

## 5. 编辑器 UI 设计

### 5.1 入口

`UMGStateConfigEditor` 在 Widget Blueprint 编辑器里提供：

- 工具栏按钮：打开 `UI 状态配置` Tab。
- Tab 面板：由 `SUIStateConfigPanel` 构建。

### 5.2 主面板结构

```text
UI 状态配置标题区
├── 父状态按钮区：+ 父状态 / 复制父状态 / - 父状态
├── 父状态 Tab 行
├── 子状态按钮区：+ 子状态 / 复制子状态 / - 子状态 / 清理无效
├── 子状态 Tab 行
├── 当前状态面包屑
├── 左侧：可加入控件列表
│   └── 右键控件选择属性添加入口
└── 右侧：当前子状态已配置控件卡片
    └── 每个控件下展示已插入的 SerializedProperty 属性行
```

### 5.3 属性添加入口

控件右键菜单提供两个入口：

| 入口 | 面板显示范围 | 捕获范围 | 定位 |
| --- | --- | --- | --- |
| `从常用 Details 添加属性` | 只显示常用 UI 外观属性 | 常用属性子集 | 日常配置入口 |
| `从全部 Details 添加属性` | 显示完整 Details 面板 | Runtime 白名单允许的属性 | 高级配置入口 |

两者底层流程一致，都是“临时 Details 面板 + 快照 Diff + 叶子属性保存 + 噪音过滤”。区别只在可见属性和快照捕获范围。

常用模式当前覆盖：

- `Visibility`
- `RenderOpacity`
- `Text`
- `Brush.ResourceObject`
- `ColorAndOpacity.*`
- `Brush.TintColor.*`
- `Brush.ImageSize.*`
- `Font.Size`
- `RenderTransform.*`

### 5.4 Details 捕获流程

```text
右键控件选择 Details 入口
→ DuplicateObject 复制真实 Widget 到 transient proxy
→ 对 proxy 应用当前状态里已保存的属性
→ 捕获修改前快照
→ 创建 IDetailsView 并 SetObject(proxy)
→ 常用模式时挂 SetIsPropertyVisibleDelegate 隐藏非常用属性
→ 用户修改 Details 属性
→ OnFinishedChangingProperties
→ 捕获修改后快照
→ Diff 修改前 / 修改后快照
→ 只保留真实变化的叶子属性
→ FilterNoisyPropertyChanges 过滤联动噪音
→ AddOrUpdateSerializedPropertyChange 写入状态配置
→ ApplyPreviewState 刷新设计器预览
```

噪音过滤的目标是避免用户只改一个主属性，却保存一批 UE 内部联动字段。例如：

- 改 `Brush.ResourceObject` 时，过滤 `Brush.DrawAs`、`Brush.ImageType`、`Brush.ResourceName`。
- 改 `Font.FontObject` 时，过滤 `Font.TypefaceFontName`。
- 改 `SpecifiedColor` 时，过滤 `ColorUseRule`。

### 5.5 属性行展示和编辑

右侧控件卡片只展示已经插入的属性行。每行包含：

- 删除按钮。
- 属性友好显示名。
- 当前序列化值文本。
- `编辑 Details` 按钮。

`编辑 Details` 会重新打开完整 Details 模式，便于继续修改该控件属性。

## 6. 编辑器预览

`ApplyPreviewState()` 用于编辑器内实时预览：

```text
读取 PreviewStateGroupName / PreviewStateName
→ Editor->RefreshPreview()
→ 遍历当前子状态 PropertyChanges
→ 在 Preview WidgetTree 中按 TargetWidgetName 找控件
→ 调用 FUMGStateConfigPropertyRuntimeLibrary::ApplyValue
→ Editor->InvalidatePreview(true)
→ FSlateApplication::InvalidateAllWidgets(false)
```

设计原则：编辑器预览直接复用 Runtime 属性应用库，避免编辑器和运行时维护两套 Set 逻辑。

## 7. Runtime 调用设计

### 7.1 蓝图入口

```cpp
UUMGStateConfigFunctionLibrary::ApplyUIState(UUserWidget* TargetWidget, FName StateGroupName, FName StateName)
```

内部通过 `FindOrCreateStateConfigExtension` 获取实例扩展：

```text
实例已有 UUMGStateConfigUserWidgetExtension
→ 直接使用

实例没有扩展
→ 从 TargetWidget CDO 查找默认 Runtime Extension
→ 当前实例 AddExtension
→ SetRuntimeData(DefaultExtension->RuntimeData)
```

### 7.2 状态应用主流程

```text
UUMGStateConfigUserWidgetExtension::ApplyUIState(GroupName, StateName)
├── 获取所属 UUserWidget
├── 查找状态组和子状态
├── RestoreGlobalValues：恢复所有已活跃状态组改过的属性到原始基线
├── 清空各活跃状态组的临时 RestoreValues / ActiveChangeKeys
├── 更新目标状态组的 ActiveStateName
├── ReapplyActiveStates：按 Priority 从低到高重新应用全部活跃状态组
├── FlushPendingWidgetRefreshes：合并刷新被写入的 Widget
└── 返回是否全部属性应用成功
```

### 7.3 单个属性应用流程

```text
ApplyPropertyChange
├── 校验 TargetWidgetName
├── ResolveWidget：优先 WidgetCache，失效再 GetWidgetFromName
├── 校验 ExpectedWidgetClass
├── CaptureCurrentValue
├── 当前值与目标值相同则跳过 ApplyValue
├── 首次修改该属性时保存 GlobalRestoreValues 原始基线
├── 记录当前状态组的 ActiveChangeKeys
├── ApplyValue(..., bRefreshAfterApply=false)
└── QueueWidgetRefresh
```

### 7.4 多状态组重算

运行时不是只恢复当前状态组，而是：

```text
恢复所有活跃状态组修改过的属性到原始基线
→ 更新本次调用的状态组活跃状态
→ 按 Priority 重放全部活跃状态组
```

这样能避免 A 组切换时把 B 组仍然生效的属性永久清掉。

如果多个状态组写同一个 `TargetWidgetName + PropertyType + SerializedPropertyPath`：

- 运行时最终结果由 `Priority` 决定，数值大的状态组后应用。
- 同优先级时当前实现没有显式稳定冲突策略，配置阶段会给出跨组冲突警告，建议避免同优先级冲突。

## 8. `SerializedProperty` Runtime 处理

核心类：

```cpp
FUMGStateConfigPropertyRuntimeLibrary
```

### 8.1 捕获

`CaptureCurrentValue`：

```text
校验 TargetWidget 和 PropertyType
→ 校验属性路径白名单
→ ResolveSerializedProperty
→ ExportTextItem_Direct
→ 收集 SerializedReferencedAssets
```

### 8.2 比较

`ArePropertyValuesEqual` 当前比较：

- `SerializedPropertyPath`
- `SerializedPropertyValue`

### 8.3 应用

`ApplyValue`：

```text
校验 TargetWidget 和 PropertyType
→ 校验属性路径白名单
→ ResolveSerializedProperty
→ PreloadReferencedAssets
→ ImportText_Direct
→ 如 bRefreshAfterApply 为 true，则 SynchronizeProperties + InvalidateLayoutAndVolatility
```

运行时状态应用会传 `bRefreshAfterApply=false`，最后由 `FlushPendingWidgetRefreshes` 统一刷新，避免同一个 Widget 多条属性重复刷新。

### 8.4 属性路径白名单

当前白名单：

| 控件 | 允许路径 |
| --- | --- |
| 任意 `UWidget` | `Visibility`、`RenderOpacity`、`RenderTransform.*` |
| `UImage` | `Brush.*`、`ColorAndOpacity.*` |
| `UTextBlock` | `Text`、`ColorAndOpacity.*`、`Font.*`、`ShadowOffset.*`、`ShadowColorAndOpacity.*`、`StrikeBrush.*` |
| `URichTextBlock` | `Text` |

### 8.5 属性黑名单

反射解析过程中会拒绝：

- `Transient`
- `EditConst`
- `Deprecated`
- `DisableEditOnInstance`
- Delegate / Multicast Delegate
- Array
- Map
- Set

## 9. 配置校验

`FUMGStateConfigValidator` 当前覆盖：

- 当前 `Widget Blueprint` / `WidgetTree` 是否有效。
- 是否存在状态组。
- 状态组名称是否为空或重复。
- 状态组是否没有子状态。
- 子状态名称是否为空或重复。
- 默认状态是否存在。
- `ConfiguredWidgetNames` 是否引用空控件或缺失控件。
- `PropertyChanges` 的目标控件是否为空或缺失。
- `ExpectedWidgetClass` 是否匹配真实控件类型。
- 同一状态内同一控件同一属性路径是否重复配置。
- 跨状态组是否配置了同一控件同一属性路径。
- `SerializedPropertyPath` 是否为空。
- 属性配置存在但控件未列入 `ConfiguredWidgetNames` 时给出提示。

面板底部汇总会展示错误、警告、提示数量，以及最近一次 Details 捕获结果。

## 10. 自动化测试

当前 Runtime 自动化测试文件：

```text
Source/UMGStateConfigRuntime/Private/Tests/UMGStateConfigRuntimeTests.cpp
```

已覆盖：

- `SerializedProperty` 值比较。
- 属性路径白名单。
- `FUMGStateConfigChangeKey` 相等和 hash。

这些测试证明底层纯逻辑可用，但不覆盖真实 Widget Blueprint 面板交互、Details 捕获 UI、设计器预览和运行时视觉效果。

## 11. 当前限制与后续方向

### 11.1 当前限制

- `bExclusiveGroup` 目前只是数据字段，运行时尚未实现互斥组语义。
- `SerializedProperty` 不支持数组、Map、Set、委托等复杂属性。
- Slot 属性、对象子属性、数组元素路径等复杂 Details 路径尚未系统验证。
- 反射写回后并非所有 UE 属性都一定能正确刷新 Slate，因此新增白名单属性需要实际验证。
- 跨状态组同属性冲突目前只警告，不强制阻止保存。

### 11.2 后续方向

1. 扩展更多控件类型的安全属性白名单。
2. 为常用属性补充更友好的显示名和分组。
3. 对 Slot 属性、字体资源、复杂 Brush、材质参数等典型场景补手动验收用例。
4. 明确 `bExclusiveGroup` 语义：同组互斥、跨组共存、还是按调用覆盖。
5. 补 Editor 侧自动化或半自动验收：Details 捕获、常用模式隐藏、预览刷新、跨状态组冲突提示。
6. 增加数据迁移入口：当 `SchemaVersion` 提升时统一迁移旧配置。
