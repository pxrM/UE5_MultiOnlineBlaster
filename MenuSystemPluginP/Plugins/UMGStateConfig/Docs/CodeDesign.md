# UMGStateConfig 插件代码设计文档

> 更新时间：2026-05-16  
> 适用范围：当前项目内 `Plugins/UMGStateConfig` 插件代码。

## 1. 插件目标

`UMGStateConfig` 用于在 `Widget Blueprint` 编辑器中为 UMG 控件配置一组 UI 状态，并在运行时通过蓝图函数：

```cpp
UUMGStateConfigFunctionLibrary::ApplyUIState(TargetWidget, StateGroupName, StateName)
```

把指定状态应用到目标 `UUserWidget` 实例上。

当前设计重点：

- 配置数据保存在 `Widget Blueprint Extension` 中，跟随蓝图资产保存。
- 编译蓝图时把配置复制到 `UserWidget` CDO 的运行时扩展中。
- 运行时实例调用 `ApplyUIState` 时从 CDO 拷贝配置到实例扩展。
- 属性编辑 UI 复用 UE 原生 `DetailsView`，通过代理对象承载可编辑字段。
- 应用状态时会先捕获当前值，用于切换状态前恢复旧状态改动。
- 应用状态时会比较当前值和目标值，相同则跳过 `Set` 调用。

## 2. 模块划分

插件由两个模块组成：

| 模块 | 类型 | 主要职责 |
| --- | --- | --- |
| `UMGStateConfigRuntime` | Runtime | 数据结构、蓝图函数、运行时扩展、属性捕获/比较/应用 |
| `UMGStateConfigEditor` | Editor | Widget Blueprint 编辑器入口、状态配置面板、Details 代理对象、蓝图扩展保存 |

插件描述文件：

- `UMGStateConfig.uplugin`

模块构建文件：

- `Source/UMGStateConfigRuntime/UMGStateConfigRuntime.Build.cs`
- `Source/UMGStateConfigEditor/UMGStateConfigEditor.Build.cs`

## 3. 核心文件职责

### 3.1 Runtime

| 文件 | 职责 |
| --- | --- |
| `UMGStateConfigData.h` | 定义状态配置数据结构和属性类型枚举 |
| `UMGStateConfigFunctionLibrary.h/.cpp` | 暴露蓝图调用入口 `ApplyUIState`，查找或创建运行时扩展 |
| `UMGStateConfigUserWidgetExtension.h/.cpp` | 挂载到 `UUserWidget` 实例上的运行时状态应用器 |
| `UMGStateConfigPropertyRuntimeLibrary.h/.cpp` | 统一处理属性当前值捕获、目标值应用、值相等判断 |
| `UMGStateConfigRuntimeModule.cpp` | Runtime 模块声明 |

### 3.2 Editor

| 文件 | 职责 |
| --- | --- |
| `UMGStateConfigEditorModule.cpp` | 注册 Widget Blueprint 编辑器 Tab 和工具栏入口 |
| `UMGStateConfigToolbar.cpp` | 在 Widget Blueprint 工具栏增加“UI 状态配置”按钮 |
| `UMGStateConfigTabSummoner.cpp` | 注册并创建“UI 状态配置”面板 Tab |
| `SUIStateConfigPanel.h/.cpp` | 状态配置主面板，负责状态组、子状态、控件、属性行、预览和保存 |
| `UMGStateConfigBlueprintExtension.h/.cpp` | 编辑期配置保存对象，编译时复制配置到 Runtime 扩展 |
| `UMGStateConfigDetailsProxy.h/.cpp` | DetailsView 代理对象，负责编辑 UI 与配置值互转 |
| `UMGStateConfigValidator.cpp` | 当前只做基础配置检查 |

## 4. 数据模型

核心数据结构定义在：

- `Source/UMGStateConfigRuntime/Public/UMGStateConfigData.h`

### 4.1 属性类型

`EUMGStateConfigPropertyType` 当前包含：

| 类型 | 说明 | 当前状态 |
| --- | --- | --- |
| `Visibility` | 控件可见性 | 新编辑入口支持 |
| `RenderOpacity` | 控件渲染透明度 | 新编辑入口支持 |
| `Text` | `TextBlock` / `RichTextBlock` 文本内容 | 新编辑入口支持 |
| `TextColor` | 旧版文本颜色单项 | Runtime 兼容，Editor 不再主动新增 |
| `BrushImage` | 旧版 Image Brush 资源单项 | Runtime 兼容，Editor 不再主动新增 |
| `BrushTint` | 旧版 Image Color 单项 | Runtime 兼容，Editor 不再主动新增 |
| `ImageAppearance` | `Image` 外观聚合属性 | 新编辑入口支持 |
| `TextAppearance` | `TextBlock` 外观聚合属性 | 新编辑入口支持 |

### 4.2 属性值容器

`FUMGStateConfigPropertyValue` 是一个通用值容器。不同属性类型使用其中不同字段：

| 字段 | 用途 |
| --- | --- |
| `TextValue` | 文本内容 |
| `ColorValue` | 主颜色，例如 Text Color、Image ColorAndOpacity |
| `SecondaryColorValue` | 辅助颜色，例如 Text ShadowColorAndOpacity |
| `VectorValue` | 向量值，例如 Text ShadowOffset |
| `FontValue` | 文本字体 |
| `FloatValue` | 浮点值，例如 RenderOpacity |
| `ObjectValue` | 资源对象引用，旧 BrushImage 兼容和 Brush resource 缓存 |
| `BrushValue` | `ImageAppearance` 的完整 `FSlateBrush` |
| `VisibilityValue` | 控件可见性 |
| `SerializedPropertyPath` | 通用 Details 属性路径 |
| `SerializedPropertyTypeName` | 通用 Details 属性 C++ 类型名 |
| `SerializedPropertyValue` | 通用 Details 属性导出文本值 |


### 4.3 状态层级

```text
FUMGStateConfigRuntimeData
└── StateGroups: TArray<FUMGStateConfigGroup>
    ├── GroupName
    ├── DisplayName
    ├── DefaultStateName
    └── States: TArray<FUMGStateConfigState>
        ├── StateName
        ├── DisplayName
        ├── ConfiguredWidgetNames
        └── PropertyChanges: TArray<FUMGStatePropertyChange>
            ├── TargetWidgetName
            ├── ExpectedWidgetClass
            ├── EditorPath
            ├── PropertyType
            └── Value: FUMGStateConfigPropertyValue
```

`PreviewStateGroupName` 和 `PreviewStateName` 保存在 `FUMGStateConfigRuntimeData` 中，用于编辑器面板记录当前预览状态。

## 5. 配置保存设计

### 5.1 编辑期保存位置

编辑器中所有状态配置保存在：

```cpp
UUMGStateConfigBlueprintExtension::ConfigData
```

`UUMGStateConfigBlueprintExtension` 继承自 `UWidgetBlueprintExtension`，因此配置跟随 `Widget Blueprint` 资产保存。

### 5.2 写入时机

在 `SUIStateConfigPanel` 中，以下操作会修改 `ConfigData`：

- 新增/删除父状态组。
- 新增/删除子状态。
- 选择预览状态。
- 双击控件加入当前状态组。
- 右键控件添加属性配置。
- 在 DetailsView 中修改某个属性值。
- 删除某个控件或属性配置。

修改后会调用：

```cpp
MarkConfigDirty(Extension)
```

该函数会：

- 调用 `WidgetBlueprint->Modify()`。
- 调用 `WidgetBlueprint->MarkPackageDirty()`。
- 调用 `Extension->Modify()`。

这样 UE 会把配置作为蓝图资产的一部分保存。

### 5.3 编译时复制到 Runtime

`UUMGStateConfigBlueprintExtension::HandleCopyTermDefaultsToDefaultObject` 会在 Widget Blueprint 编译复制默认对象时执行：

```cpp
DefaultWidget->RemoveExtensions(UUMGStateConfigUserWidgetExtension::StaticClass());
RuntimeExtension = DefaultWidget->AddExtension(UUMGStateConfigUserWidgetExtension::StaticClass());
RuntimeExtension->RuntimeData = ConfigData;
```

也就是说：

1. 编辑期数据在 `BlueprintExtension.ConfigData`。
2. 编译后数据被复制到 Widget CDO 的 `UUMGStateConfigUserWidgetExtension.RuntimeData`。
3. 运行时实例从 CDO 上的扩展读取并拷贝配置。

## 6. 编辑器 UI 设计

### 6.1 入口注册

`FUMGStateConfigEditorModule::StartupModule` 做两件事：

- 向 UMG 编辑器注册工具栏扩展。
- 向 Widget Blueprint 编辑器注册 `UI 状态配置` Tab。

工具栏按钮由 `FUMGStateConfigToolbar` 创建，点击后调用：

```cpp
WidgetEditorPtr->GetTabManager()->TryInvokeTab(FUMGStateConfigTabSummoner::TabID)
```

Tab 内容由 `FUMGStateConfigTabSummoner::CreateTabBody` 创建：

```cpp
SNew(SUIStateConfigPanel, WidgetEditorPtr)
```

### 6.2 主面板结构

`SUIStateConfigPanel` 当前 UI 结构：

```text
UI 状态配置标题区
├── 父状态按钮区：+ 父状态 / - 父状态
├── 父状态 Tab 行
├── 子状态 Tab 行：+ 子状态 / - 子状态
├── 当前状态面包屑
├── 左侧：可加入控件列表
│   └── 右键控件选择可配置属性
└── 右侧：当前子状态已配置控件卡片
    └── 每个控件下展示已插入属性行
```

### 6.3 支持属性筛选

`GetSupportedPropertyTypes` 根据控件类型决定可添加的属性：

- 所有控件：`Visibility`、`RenderOpacity`
- `UTextBlock` / `URichTextBlock`：`Text`
- `UTextBlock`：`TextAppearance`
- `UImage`：`ImageAppearance`

旧属性 `TextColor`、`BrushImage`、`BrushTint` 保留兼容，但不再从新菜单中主动添加。

### 6.4 属性编辑复用 DetailsView

当前所有新属性编辑都走：

```cpp
SUIStateConfigPanel::BuildAppearanceDetailsValueWidget
```

流程：

1. 找到当前属性已有配置；没有则从真实 Widget 生成默认值。
2. 根据 `PropertyType` 创建对应的 `UObject` 代理对象。
3. 调用代理对象 `FromValue`，把 `FUMGStateConfigPropertyValue` 转成 Details 可编辑属性。
4. 使用 `PropertyEditorModule.CreateDetailView` 创建 UE 原生 DetailsView。
5. `DetailsView->SetObject(ProxyObject)` 展示代理对象。
6. 监听 `OnFinishedChangingProperties`。
7. 属性修改结束后调用代理对象 `ToValue` 写回 `FUMGStateConfigPropertyValue`。
8. 调用 `AddOrUpdatePropertyChange` 保存配置并刷新预览。

### 6.5 Details 代理对象

代理类定义在 `UMGStateConfigDetailsProxy.h/.cpp`：

| 代理类 | 对应属性类型 | Details 分类 |
| --- | --- | --- |
| `UUMGStateConfigVisibilityProxy` | `Visibility` | `Behavior` |
| `UUMGStateConfigRenderOpacityProxy` | `RenderOpacity` | `Rendering` |
| `UUMGStateConfigTextContentProxy` | `Text` | `Content` |
| `UUMGStateConfigImageAppearanceProxy` | `ImageAppearance` | `Appearance` |
| `UUMGStateConfigTextAppearanceProxy` | `TextAppearance` | `Appearance` |

代理对象设计目的：

- 不直接编辑真实 Widget，避免污染默认态。
- 复用 UE 原生 Details 的展开、类型编辑器、资源选择器、颜色选择器、字体编辑器等表现。
- 让新增属性时只需要新增一个代理对象和一组值转换逻辑。

### 6.6 冗余属性收敛

`NormalizeRedundantPropertyChanges` 会清理旧冗余配置：

- 同一控件已有 `ImageAppearance` 时，移除 `BrushImage` 和 `BrushTint`。
- 同一控件已有 `TextAppearance` 时，移除 `TextColor`。

`AddOrUpdatePropertyChange` 也会在添加聚合属性时主动移除对应旧属性。

## 7. 编辑器预览设计

当切换状态、修改属性、删除属性时，会调用：

```cpp
ApplyPreviewState()
```

当前预览逻辑：

1. 从 `Extension->ConfigData.PreviewStateGroupName` 找状态组。
2. 从 `PreviewStateName` 找子状态。
3. 调用 `Editor->RefreshPreview()` 重建预览 Widget。
4. 遍历 `State.PropertyChanges`。
5. 在预览 `WidgetTree` 中按 `TargetWidgetName` 查找控件。
6. 调用 `ApplyPropertyValueToDesignerWidget` 把配置值应用到预览控件。
7. 调用 `Editor->InvalidatePreview(true)` 和 `FSlateApplication::InvalidateAllWidgets(false)` 刷新显示。

关闭 Tab 时会调用 `RefreshPreview`，用于重置设计器预览。

## 8. Runtime 调用设计

### 8.1 蓝图入口

运行时入口：

```cpp
UUMGStateConfigFunctionLibrary::ApplyUIState(UUserWidget* TargetWidget, FName StateGroupName, FName StateName)
```

内部先调用：

```cpp
FindOrCreateStateConfigExtension(TargetWidget)
```

该函数逻辑：

1. 如果实例上已有 `UUMGStateConfigUserWidgetExtension`，直接返回。
2. 如果实例没有扩展，从 `TargetWidget->GetClass()->GetDefaultObject()` 获取 CDO。
3. 从 CDO 上查找编译期写入的默认扩展。
4. 如果 CDO 扩展存在，则给当前实例 `AddExtension`。
5. 把 `DefaultExtension->RuntimeData` 拷贝到实例扩展。
6. 返回实例扩展。

### 8.2 状态应用主流程

`UUMGStateConfigUserWidgetExtension::ApplyUIState` 流程：

```text
ApplyUIState(StateGroupName, StateName)
├── 获取所属 UUserWidget
├── FindStateGroup(StateGroupName)
│   └── 使用 StateGroupIndexByName 缓存
├── 计算 EffectiveStateName
│   └── 如果传入 StateName 为空，使用 Group.DefaultStateName
├── FindState(Group, EffectiveStateName)
│   └── 使用 StateIndexByGroupName 缓存
├── RestorePreviousValues(TargetUserWidget)
│   └── 恢复上一次状态应用前捕获的旧值
├── 遍历 TargetState.PropertyChanges
│   └── ApplyPropertyChange(TargetUserWidget, Change)
└── 返回是否全部应用成功
```

### 8.3 单个属性应用流程

`ApplyPropertyChange` 流程：

```text
ApplyPropertyChange(TargetUserWidget, Change)
├── 校验 TargetWidgetName
├── ResolveWidget(TargetUserWidget, TargetWidgetName)
│   └── 优先使用 WidgetCache，失效再 GetWidgetFromName
├── 校验 ExpectedWidgetClass
├── CaptureCurrentValue(TargetWidget, PropertyType, CurrentValue)
├── ArePropertyValuesEqual(PropertyType, CurrentValue, Change.Value)
│   └── 如果相同，直接返回 true，不调用 Set
├── 生成 ChangeKey(TargetWidgetName, PropertyType)
├── 如果 RestoreValues 中没有该 Key，则保存 CurrentValue
├── ActiveChangeKeys 记录该 Key
└── ApplyValue(TargetWidget, PropertyType, Change.Value)
```

### 8.4 状态恢复设计

每次应用新状态前会先调用 `RestorePreviousValues`。

它会遍历上一次状态应用时记录的 `ActiveChangeKeys`，再从 `RestoreValues` 取出应用状态前的原始值，并调用：

```cpp
FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(TargetWidget, PropertyType, RestoreValue)
```

恢复完成后清空：

- `ActiveChangeKeys`
- `RestoreValues`

这样可以避免状态 A 改过的属性残留到状态 B。

### 8.5 性能优化点

当前 Runtime 已做的优化：

1. **状态组查找缓存**  
   `StateGroupIndexByName: TMap<FName, int32>`，避免每次线性查找状态组。

2. **子状态查找缓存**  
   `StateIndexByGroupName: TMap<FName, TMap<FName, int32>>`，避免每次线性查找子状态。

3. **控件查找缓存**  
   `WidgetCache: TMap<FName, TWeakObjectPtr<UWidget>>`，避免重复 `GetWidgetFromName`。

4. **结构化恢复 Key**  
   `FUMGStateConfigChangeKey` 使用 `TargetWidgetName + PropertyType`，避免字符串拼接 Key。

5. **重复 Set 跳过**  
   应用属性前先捕获当前值并比较；相同则跳过 `ApplyValue`。

6. **聚合属性子项差异 Set**  
   `ImageAppearance` 和 `TextAppearance` 内部也会逐子项比较，只对实际变化的子项调用 Set。

## 9. 属性运行时处理库

统一入口在：

```cpp
FUMGStateConfigPropertyRuntimeLibrary
```

### 9.1 `CaptureCurrentValue`

从真实 Widget 捕获当前值，用于：

- 应用前比较是否已经等于目标值。
- 保存恢复值，便于切换状态时还原。

当前支持：

- `Visibility`
- `RenderOpacity`
- `Text`
- `TextAppearance`
- `TextColor`
- `BrushImage`
- `BrushTint`
- `ImageAppearance`

### 9.2 `ArePropertyValuesEqual`

按属性类型比较两个 `FUMGStateConfigPropertyValue` 是否等价：

- 浮点使用 `KINDA_SMALL_NUMBER`。
- 颜色使用 `FLinearColor::Equals`。
- 文本使用 `FText::EqualTo`。
- Brush 比较资源对象、尺寸、Margin、DrawAs、Tiling、Mirroring、TintColor。

### 9.3 `ApplyValue`

把配置值应用到真实 Widget：

- 通用控件：`SetVisibility`、`SetRenderOpacity`
- 文本控件：`SetText`、`SetColorAndOpacity`、`SetFont`、`SetShadowOffset`、`SetShadowColorAndOpacity`
- 图片控件：`SetBrush`、`SetColorAndOpacity`
- 旧 BrushImage 兼容：根据资源类型调用 `SetBrushFromTexture`、`SetBrushFromMaterial`、`SetBrushFromAsset` 或 `SetBrushResourceObject`

`ApplyValue` 内部也会做二次差异判断，避免重复调用 Set。

## 10. 当前支持能力

| 控件类型 | 支持配置 |
| --- | --- |
| `UWidget` 通用 | `Visibility`、`RenderOpacity` |
| `UTextBlock` | `Content / Text`、`Appearance` |
| `URichTextBlock` | `Content / Text` |
| `UImage` | `Appearance`，包含 `Brush` 和 `Color and Opacity` |
| 任意 `UWidget` | 从 Details 添加属性，按顶层属性路径序列化保存 |


## 11. 新增属性的推荐开发流程

当前代码有两条扩展路线：

```text
高频稳定属性：Native PropertyType + Details Proxy + RuntimeLibrary 特化 Apply
更多 Details 属性：SerializedProperty + 属性路径 + FProperty Export/Import
```

优先推荐：普通 Details 属性先走“从 Details 添加属性”的序列化路线；只有发现反射写回无法刷新 Slate、需要调用 Setter、或需要更细粒度差异 Set 时，再升级为 Native 特化属性。

Native 特化属性步骤：


1. 在 `EUMGStateConfigPropertyType` 增加新枚举。
2. 判断 `FUMGStateConfigPropertyValue` 是否已有合适字段；没有则增加字段。
3. 在 `UMGStateConfigDetailsProxy` 新增代理类：
   - 暴露 `UPROPERTY(EditAnywhere)`。
   - 实现 `FromValue`。
   - 实现 `ToValue`。
4. 在 `SUIStateConfigPanel::GetSupportedPropertyTypes` 中按控件类型添加新属性。
5. 在 `SUIStateConfigPanel::GetPropertyLabel` 中添加显示名。
6. 在 `SUIStateConfigPanel::GetExpectedWidgetClass` 中声明期望控件类型。
7. 在 `SUIStateConfigPanel::BuildAppearanceDetailsValueWidget` 中创建新代理对象。
8. 在 `SUIStateConfigPanel::OnAppearanceDetailsFinishedChanging` 中把代理对象转回配置值。
9. 在 `SUIStateConfigPanel::MakeDefaultValueForWidget` 中从真实 Widget 读取默认值。
10. 在 `FUMGStateConfigPropertyRuntimeLibrary` 中补齐：
    - `CaptureCurrentValue`
    - `ArePropertyValuesEqual`
    - `ApplyValue`

编辑器预览已统一复用 `FUMGStateConfigPropertyRuntimeLibrary::ApplyValue`，不再需要单独维护预览 Set 分支。推荐继续保持“代理对象 + DetailsView”的路线，避免重新手写一套仿 Details 的 Slate 属性面板。


## 12. 当前设计限制和后续可优化点

### 12.1 编辑器预览和 Runtime 应用逻辑已复用

编辑器预览现在直接调用：

```cpp
FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(PreviewChildWidget, Change.PropertyType, Change.Value)
```

不再维护 `SUIStateConfigPanel` 内部的重复属性 Set 分支。这样新增或调整属性应用逻辑时，只需要维护 RuntimeLibrary，编辑器预览会同步受益。

### 12.2 Details 代理创建逻辑已描述化

`BuildAppearanceDetailsValueWidget` 和 `OnAppearanceDetailsFinishedChanging` 不再逐类型写大量 `if / else` 创建和写回代理对象。

当前抽象为：

```text
PropertyType → ProxyClass → DefaultExpectedClass → bUseWidgetSpecificExpectedClass
```

代理对象统一继承 `UUMGStateConfigDetailsProxyBase`，面板通过描述信息创建代理，再调用统一的 `FromValue / ToValue` 完成配置值互转。新增属性时仍需要注册描述项，但改动点更集中。


### 12.3 已进入 Details 属性路径序列化方案

当前已增加 `SerializedProperty` 通用属性类型。除 Native 特化属性外，用户可以在控件右键菜单中选择“从 Details 添加属性”，打开该控件的临时 Details 面板并修改属性。

保存数据写入 `FUMGStateConfigPropertyValue`：

- `SerializedPropertyPath`：属性路径。当前实现优先记录 `PropertyChangedEvent.MemberProperty`，也就是嵌套字段按顶层属性保存。
- `SerializedPropertyTypeName`：属性 C++ 类型名。
- `SerializedPropertyValue`：通过 `FProperty::ExportTextItem_Direct` 导出的文本值。

运行时通过 `FProperty` 反射执行：

```text
SerializedPropertyPath
→ ResolveSerializedProperty
→ ExportTextItem_Direct 捕获当前值
→ 文本值比较，避免重复写
→ ImportText_Direct 写回目标值
→ SynchronizeProperties / InvalidateLayoutAndVolatility 刷新控件
```

`Visibility`、`RenderOpacity`、`Text`、`ImageAppearance`、`TextAppearance` 等高频属性仍保留 Native 特化通道；通用 Details 属性作为扩展覆盖层，用于减少后续新增属性时的手写代理成本。


### 12.4 配置校验已增强

`FUMGStateConfigValidator` 当前会检查：

- 当前 Widget Blueprint / WidgetTree 是否有效。
- 是否存在配置和状态组。
- 状态组名称是否为空或重复。
- 子状态名称是否为空或重复。
- 默认状态是否存在。
- `ConfiguredWidgetNames` 中的控件是否仍存在。
- `PropertyChanges` 的目标控件是否仍存在。
- `ExpectedWidgetClass` 是否匹配真实控件类型。
- 同一状态内同一控件的同一属性是否重复配置。
- 旧版 `BrushImage` 配置是否缺少有效资源。

面板底部汇总现在会调用该校验器，显示真实错误、警告、提示数量。


## 13. 总体调用链总结

### 13.1 编辑保存链路

```text
用户打开 Widget Blueprint
→ 点击 UI 状态配置按钮
→ SUIStateConfigPanel 读取/创建 UUMGStateConfigBlueprintExtension
→ 用户编辑状态/属性
→ Details Proxy ToValue 写回 FUMGStateConfigPropertyValue
→ AddOrUpdatePropertyChange 写入 ConfigData.StateGroups[*].States[*].PropertyChanges
→ MarkConfigDirty 标记蓝图资产脏
→ 保存蓝图资产
```

### 13.2 编译链路

```text
Widget Blueprint 编译
→ UUMGStateConfigBlueprintExtension::HandleCopyTermDefaultsToDefaultObject
→ 清理 CDO 上旧 Runtime Extension
→ 添加新的 UUMGStateConfigUserWidgetExtension
→ RuntimeExtension.RuntimeData = ConfigData
```

### 13.3 运行时链路

```text
蓝图/代码调用 ApplyUIState(TargetWidget, GroupName, StateName)
→ FindOrCreateStateConfigExtension(TargetWidget)
→ 从实例扩展或 CDO 扩展取得 RuntimeData
→ Extension.ApplyUIState(GroupName, StateName)
→ 查找状态组和状态
→ 恢复上次状态改动
→ 遍历目标状态 PropertyChanges
→ ResolveWidget
→ CaptureCurrentValue
→ 当前值与目标值相同则跳过
→ 保存恢复值
→ ApplyValue 应用变化
```
