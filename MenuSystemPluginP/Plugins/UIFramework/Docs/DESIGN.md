# UIFramework 设计文档

生产级、可复用的 UMG UI 框架插件。不局限单一项目，拷入任意 UE5 项目 `Plugins/` 即可用。

---

## 1. 目标与原则

| 原则 | 说明 |
|------|------|
| 可复用 | Build.cs 只依赖引擎模块 + 插件内模块，绝不引用 game module |
| 解耦 | View / ViewModel / Model 分离，UI 不直接引游戏逻辑 |
| 数据驱动 | 样式、布局走 DataAsset / 配置，不硬编码 |
| 生产健壮 | 绑定失败降级、资源缺失兜底、空指针防护、专用日志 |
| 高性能 | 列表虚拟化、控件池、Invalidation Box |
| 可扩展 | 基类虚函数暴露、样式覆盖、继承扩展 |

依赖不从零造：以 CommonUI 为骨架，MVVM 做绑定，参考 Lyra 架构。

---

## 2. 分层架构

```
表现层 (View)        纯渲染，无逻辑。WBP 控件
逻辑层 (ViewModel)   状态 + 命令。MVVM 双向绑定
数据层 (Model)       数据源。网络/本地/配置
框架层 (Core)        生命周期、事件总线、UI 层级管理、DI
```

分层理由：View 换皮不动逻辑；逻辑单测不依赖渲染；多人协作程序/美术不抢文件。

---

## 3. 插件结构

```
Plugins/UIFramework/
├── UIFramework.uplugin
├── Source/
│   ├── UIFrameworkCore/          # Runtime, 无 UMG 依赖 (事件总线/层级栈/DI)
│   │   ├── UIFrameworkCore.Build.cs
│   │   ├── Public/
│   │   └── Private/
│   ├── UIFrameworkWidgets/       # Runtime, 依赖 UMG/CommonUI/CommonInput/MVVM
│   │   ├── UIFrameworkWidgets.Build.cs
│   │   ├── Public/  (Private/ 镜像同结构)
│   │   │   ├── Layers/        # UIRootWidget, UILayerSubsystem
│   │   │   ├── Config/        # UISettings, UIWidgetRegistry (配置 schema, app 填值)
│   │   │   ├── Management/    # UIManagerSubsystem, UIWidgetCache (运行时编排)
│   │   │   ├── ViewModels/    # UIViewModelBase, UIViewModelSupport, UIViewModelWidgetBase
│   │   │   ├── Pool/          # UIWidgetPool, UIPoolableWidget
│   │   │   ├── Styles/        # UITextStyle, UIButtonStyle, UIPanelStyle, UITheme
│   │   │   └── Widgets/       # 控件: Text/Button/Dialog/List/ActivatableScreen/Typewriter
│   └── UIFrameworkEditor/        # Editor (可视化工具, 可选)
│       ├── UIFrameworkEditor.Build.cs
│       ├── Public/
│       └── Private/
├── Content/
│   ├── Widgets/                  # WBP 通用控件 (Atoms/Molecules/Organisms/Layouts)
│   ├── Themes/                   # 默认主题 DataAsset
│   └── Examples/                 # 示例，可删不影响核心
└── Config/
    └── FilterPlugin.ini
```

### .uplugin 要点

- `CanContainContent: true` —— 放 WBP 必须开
- `Plugins` 字段声明依赖 CommonUI + ModelViewViewModel，引擎自动启用
- 模块类型：Core/Widgets 为 `Runtime`，Editor 为 `Editor`（打包不进游戏）

### Build.cs 复用铁律

| 规则 | 原因 |
|------|------|
| 只引擎模块 + 插件内模块 | 引 game module 则不可复用 |
| 不 include 项目代码 | 同上 |
| 配置走 ini/DataAsset，不写死路径 | 换项目路径不同 |
| 依赖走 .uplugin Plugins 字段 | 引擎自动拉起 |
| 公共类加 `MODULENAME_API` 宏 | 跨模块/项目导出 |
| 资源引用用 Soft Reference | 硬引用跨项目易断 |

模块依赖方向单向：`Editor → Widgets → Core`。Core 无 UMG，可单测。

---

## 4. UI 层级管理（核心设计）

### 概念

UI 分层，别都堆一个 Canvas 手调 ZOrder。

- **层 (Layer)**：固定顺序，决定谁盖谁。PersistentSystem 最底，Tips 最高
- **栈 (Stack)**：层内动态 push/pop，管页面前后 + 激活/返回/焦点

层定 Z 顺序（固定），栈定层内先后（动态）。

### Root WBP 结构

单一根 WBP 作全局入口，内部每层一个容器，外层用 Overlay 靠子顺序自动叠：

```
WBP_Root (继承 C++ 基类 UUIRootWidget)
└── Overlay (根容器, 撑满屏)
    ├── Layer_PersistentSystem  ← 层0  系统常驻
    ├── Layer_Loading           ← 层1  加载
    ├── Layer_Background        ← 层2  背景
    ├── Layer_Dock              ← 层3  停靠 UI
    ├── Layer_FullWindow        ← 层4  全屏窗口
    ├── Layer_PopupWindow       ← 层5  弹窗
    ├── Layer_Guide             ← 层6  引导
    ├── Layer_Notification      ← 层7  通知
    └── Layer_Tips              ← 层8  顶层提示
```

设计要点：

- **层间容器用 Overlay**：子控件按添加顺序叠，后加盖前面。拖动排序即定层级，不手填 ZOrder
- **层内摆控件用 Canvas**：同层内 tooltip 盖按钮等，用 CanvasPanelSlot ZOrder 微调
- **每层是一个栈容器**：页面不直接 AddChild，经栈逻辑 push/pop，自动管激活/返回/焦点
  - 上 CommonUI：每层用 `CommonActivatableWidgetStack`（自带 push/pop/焦点/返回）
  - 当前实现：各层 Panel + `UUILayerStack` 单数组统一管理

### 层枚举

```
PersistentSystem
Loading
Background
Dock
FullWindow
PopupWindow
Guide
Notification
Tips
Max (仅哨兵，不可入栈)
```

使用 `EUILayer` UENUM，枚举声明顺序就是从低到高的绘制和栈优先级。

### C++ 基类接口（UUIRootWidget）

Root WBP 继承此基类，基类 `BindWidget` 拿各层容器引用，暴露：

```
PushToLayer(ELayer Layer, TSubclassOf<UUserWidget> WidgetClass) -> UUserWidget*
PopFromLayer(ELayer Layer)
PopAll(ELayer Layer)
HandleBackAction()          // 返回键：顶层栈 pop
```

### 运行时架构（已实现）

职责拆两层 subsystem，避免「管层的东西又管业务开关」：

```
调用方
  ├─ 日常按 tag 开关   → UUIManagerSubsystem::OpenUI(Tag) / CloseUI(Tag)   [策略]
  └─ 手推特定 class    → UUILayerSubsystem::PushToLayer(Layer, Class)      [机制]
                              ↓
                          UUIRootWidget (层容器 + 每层栈)
```

| 类 | 层次 | 职责 | 位置 |
|----|------|------|------|
| `EUILayer` | 类型 | 层枚举（纯 enum，无 UMG） | **UIFrameworkCore** |
| `UUILayerSubsystem` | 机制 | Root 生命周期 + push/pop/remove/back | UIFrameworkWidgets |
| `UUIManagerSubsystem` | 策略 | 查注册表 + 软加载 + tag 开关 + 追踪 | UIFrameworkWidgets |
| `UUIRootWidget` | 表现 | 各层容器 + 转发到 `UUILayerStack` 单栈 | UIFrameworkWidgets |

- **EUILayer 在 Core**：纯枚举下沉，Core 的事件总线/VM 可引用层而不反依赖 Widgets。依赖单向 `Widgets → Core`
- **Layer=机制**：不认 GameplayTag/注册表/设置，只干搬控件进出层。可单独复用
- **Manager=策略**：`OpenUI(Tag)` 查 `UUIWidgetRegistry`（tag→{class, layer, bAllowMultiple}）→ 软加载 → `EnsureRoot`（首帧按 `UUISettings::DefaultRootClass` 懒建 Root，避开 viewport 时机坑）→ 委托 Layer push。换策略（过渡动画/权限）不碰机制

运行时生命周期只有一个出口：`UUILayerStack` 在 Pop、Back、Remove、Clear、Root teardown
时统一广播移除事件，经 Root 和 LayerSubsystem 转发给 Manager。Manager 在该回调中清理打开记录并
执行缓存策略，因此业务即使从低层触发返回，也不会留下脏记录。重复 `OpenUIAsync` 会共享同一
加载句柄；`CancelOpenUI`、`CloseUI`（尚未打开时）和 `CloseAllUI` 都可取消待完成的加载。

### Managed Widget 生命周期协议

通过 `UUIManagerSubsystem` 打开的 Widget 若实现 `IUIManagedWidget`，会收到统一协议：

```text
新实例: Opening -> Opened -> Activated
缓存实例: RestoredFromCache -> Opening -> Opened -> Activated
被新页面盖住: Deactivated
重新成为栈顶: Activated
关闭: Closing -> Deactivated(若当前激活) -> Closed
关闭完成后: Manager 按 CachePolicy 决定缓存或释放
```

- `FUIOpenContext`：`Key / Layer / Payload / bRestoredFromCache`
- `FUICloseContext`：`Key / Layer / Reason / Result / bWillBeCached`
- `EUIWidgetCloseReason`：区分 Requested、Back、LayerPop、LayerClear、RootTeardown、SceneChange、SubsystemShutdown
- `OpenUI(Tag, Payload)` 和 `CloseUI(Tag, Result)` 负责参数与结果传递
- 异步打开会强引用 Payload 直到完成或取消；同 Tag 单实例的并发请求共享首个请求的 Payload
- `UUIActivatableScreenBase` 和 `UUIViewModelWidgetBase` 默认实现接口，蓝图子类可直接 Override

Manager 使用 deferred stack refresh，保证 Widget 已 Construct 且 `Opened` 完成后才触发首次
`Activated`。关闭事件同样由栈的 Removing/ActivationChanged/Removed 三阶段保证顺序。

配置驱动（换项目只改配置，零硬编码 class）：
- `UUISettings`（DeveloperSettings，项目设置 > Game > UI Framework）：`DefaultRootClass` + `Registry`
- `UUIWidgetRegistry`（DataAsset）：`TMap<GameplayTag, FUIWidgetEntry>`

`UUIDialogBase::Close()` 会优先调用 `UUIManagerSubsystem::CloseWidget(this)`，保证
Manager 的打开记录和实例缓存同步；若 Dialog 是直接通过 Layer 机制推入，则回退为
`RemoveWidget(PopupWindow, this)`，避免误弹出后来压在它上面的其他弹窗。

### 栈管理类 UUILayerStack + 单栈可见性（跨层遮挡）

栈逻辑从 `UUIRootWidget` 抽出独立类，按 Lua UIStack 的**单数组 + CheckShow 两阶段**实现。

- **职责拆分**：`UUIRootWidget` = 视图（持 9 个层容器 + 转发）；`UUILayerStack` = 帧序 + 可见性 + 激活策略。延续 Layer/Manager/Cache 的单一职责
- **单数组** `Frames`（bottom→top），`EUILayer` 当优先级 tier。`FindInsertIndex` 按 tier 插入、同层 FILO（等价 UIStack 的 `CheckPushUPAble` 优先级比较）。不再 `TMap<层,栈>`
- **帧** `FUIStackFrame { Widget, Layer }`
- **CheckShow + 覆盖矩阵**（UIStack 6.3 + 4.1）：每帧检查其上方所有**可见**帧，若某上方帧的层被配置为可覆盖本帧的层 → 隐藏本帧。**跨层生效**
- **覆盖矩阵** `UUICoverageConfig`（DataAsset，Config/）：`Rules[]` 每条 `{CoveredLayer, CoveringLayer}`，即 UIStack 的 `UIStackFrameTypeCoveredableCfg[被覆盖][覆盖]`。可配、集中。项目设置 `CoverageConfig` 指定；未设=不跨层隐藏
  - 例：`{Covered=Dock, Covering=FullWindow}` → 全屏窗口隐藏停靠 UI；未配置的层组合保持同时可见
- **返回键** = 弹全栈最顶帧（UIStack 单栈语义），不再「按最高层弹」

> 来源：参考 Lua UIStack（优先级单栈 + 覆盖矩阵 + CheckShow + 拍脸 FIFO + 溢出驱逐）。采用其单栈 + CheckShow + **可配覆盖矩阵**（层×层）；**未采用**拍脸 FIFO（无需求）、溢出驱逐（UUIWidgetCache 已管生命周期）。层的固定 Z 仍由 WBP 容器保证，Stack 只决定该序内的可见性与激活。

### CommonUI 选择性接入（手柄/主机支持）

框架**保留自造层栈**（UUIRootWidget + UUILayerStack 单数组），但为手柄导航/输入路由/返回键**选择性借用 CommonUI**——不使用其 ActivatableWidgetStack。

关键：CommonUI 的输入能力来自 **Action Router**，它追踪的是**激活的 `UCommonActivatableWidget`**，与是否用 Stack widget 无关。

```
屏基类:  UUIActivatableScreenBase : UCommonActivatableWidget   ← 激活/返回/焦点/输入配置
层容器:  UUIRootWidget + UUILayerStack 单栈                     ← push/pop 时调 Activate/Deactivate
Action Router (CommonUI 自动, 每 LocalPlayer 一个):            ← 追激活的屏 → 手柄导航 + 输入路由
```

- `PushToLayer`：盖住旧顶前 `DeactivateWidget`，新屏 `ActivateWidget`；普通 Widget 仅在存在 OwningPlayer 时设置焦点
- `PopFromLayer`：弹顶后重新 `ActivateWidget` 新顶
- 屏用**继承自 CommonActivatable** 的 `DesiredFocusWidget`（WBP 里同名绑定，手柄焦点入口）+ 覆盖 `GetDesiredInputConfig`（输入模式 Menu/Game/All）
- **叶子控件**（金币/血条）用 `UUIViewModelWidgetBase`（普通 UUserWidget），不激活；**屏**用 `UUIActivatableScreenBase`。VM 注入逻辑经 `UIFrameworkVM::Inject` 共享，不随基类重复
- 未动：Manager / Registry / 样式 / 事件总线 / 池 / 层栈结构

> 返回键：绑 Enhanced Input → `Manager/Layer HandleBackAction`（弹顶层栈）。屏可在 CommonUI 侧拦截（`bIsBackHandler`）。二者择一，避免双重处理。

### 为什么不用纯 ZOrder 手调

| ZOrder 手调 | Layer + Stack |
|------------|---------------|
| 数字满天飞、冲突 | 层枚举清晰 |
| 返回逻辑各写各 | 栈自动 pop |
| 焦点/输入乱 | 框架自动管 |
| 弹窗盖不住/被盖 | 层保证顺序 |

ZOrder 只在同层内微调。

---

## 5. 样式与主题系统

样式数据存 DataAsset，控件运行时读，不硬编码。改资源 → 全 UI 生效。换肤 = 换 DataAsset。

```
样式 DataAsset (UUIButtonStyle / UUITextStyle / UUIPanelStyle)
    ├── 颜色 / 字号 / 间距 / 圆角 ...
主题 DataAsset (UUITheme)
    ├── 引用各样式 DataAsset
    └── 色板 token (Primary / Secondary ...)
```

- 运行时切 `UUITheme` 实例 → 所有控件重读（DA_Theme_Dark / DA_Theme_Light）
- 可扩展 CommonUI 自带 `UCommonButtonStyle` / `UCommonTextStyle`
- 本项目已有 UMGStateConfig 插件做状态→样式映射，可当样式层整合

---

## 6. 控件库（Content/Widgets）

按 Atomic Design 分层，只放通用控件，不塞业务（背包格/技能栏属项目 Game）。

```
Widgets/
├── Atoms/        WBP_Button / WBP_Text / WBP_Image / WBP_Slider / WBP_Toggle
├── Molecules/    WBP_LabeledInput / WBP_IconButton / WBP_Card
├── Organisms/    WBP_ListView / WBP_TabPanel / WBP_Dialog / WBP_Dropdown
└── Layouts/      WBP_Root / WBP_Window / WBP_Popup
```

C++ 基类 + WBP 皮分工：逻辑在 `Source/`（可单测/可维护），外观在 WBP（可视化调）。WBP Reparent 到 C++ 基类。

### 资源归属

| 内容 | 放哪 | 判断 |
|------|------|------|
| 通用控件、默认主题 | 插件 Content | 换项目还用得上 |
| 业务控件、定制皮肤 | 项目 Game | 只属此游戏 |

依赖单向：Game → Plugin，永不反向。用户扩展走继承覆盖 / 换主题 DataAsset，不改插件内资源。

---

## 7. 性能三件套（商业项目必做）

1. **列表虚拟化**：`UListView` / `UTileView`，不用 ScrollBox 塞几百控件
2. **Invalidation Box**：静态 UI 包起来，不每帧重画
3. **控件池**：频繁增删的（伤害数字/列表项）复用，不 Create/Destroy

不做这三样，UI 一多就掉帧。这是「商业项目能用」与「demo 能用」的分界。

---

## 8. 健壮性

```cpp
if (!ensure(Style))
{
    UE_LOG(LogUIFramework, Warning, TEXT("Style missing, use default"));
    Style = GetDefaultStyle();
}
```

- 资源用 `TSoftObjectPtr` 异步加载，失败有兜底
- 委托绑定前判空
- 自定义 `LogUIFramework` 日志分类

---

## 9. 部署到其他项目

三选一：
1. 拷 `Plugins/UIFramework/` 到目标项目 `Plugins/`
2. 放引擎 `Engine/Plugins/` → 所有项目共享
3. Git submodule → 多项目共用同源

Build.cs 无项目耦合，目标项目启用插件后直接编译过。

---

## 10. 待实现清单

- [x] `.uplugin` + 3 个 Build.cs + 各模块入口 cpp
- [x] `EUILayer` 层枚举下沉 Core（UILayerTypes.h）
- [x] `UUIEventBusSubsystem`（Core 纯逻辑：tag 频道 pub/sub，UI↔逻辑解耦）
- [x] `UUIRootWidget` C++ 基类（层引用 + push/pop/remove + 返回处理）
- [x] `UUILayerSubsystem`（纯机制；单 viewport，分屏可升 LocalPlayerSubsystem）
- [x] `UUIManagerSubsystem`（策略：OpenUI/CloseUI 查表 + 软加载 + EnsureRoot）
- [x] `UUISettings`（DeveloperSettings）+ `UUIWidgetRegistry`（tag→entry）
- [x] 样式 DataAsset 类（Button/Text/Panel/Theme）
- [x] 控件基类 Text / Button（读样式 + PreConstruct 预览）
- [x] 控件基类 Dialog（推 Modal 层 + OnConfirmed/OnCancelled + 自动弹栈）
- [x] 编译验证 3 模块
- [x] 控件基类 List（UListView 虚拟化 + UUIListEntryBase 条目接口）
- [x] `UUIViewModelBase`（MVVM 逻辑层，继承 UMVVMViewModelBase + 生命周期钩子）
- [x] `UUIViewModelWidgetBase`（叶子控件：持 VM 类，自动建/注入/Initialize/Shutdown）
- [x] `UUIActivatableScreenBase`（屏基类：CommonActivatable + VM + 焦点 + 输入模式）
- [x] `UUIWidgetPool`（控件复用，按类分桶 + Prewarm + TTL 回收）
- [x] `UUIWidgetCache`（独立缓存类：类缓存 + 实例缓存 + 生命周期管理）
- [x] 缓存策略（`EUICachePolicy`：Transient / CacheClass / KeepUntilIdle(TTL) / KeepUntilSceneChange / KeepPersistent）
- [x] `OpenUIAsync`（StreamableManager 异步流式加载 + 回调）
- [x] 异步请求按 Tag 合并、取消与 Deinitialize 清理
- [x] Layer → Manager 统一移除通知（Pop / Back / Clear / teardown）
- [x] 多实例打开追踪 + 多实例关闭缓存
- [x] 缓存 Widget 的 ViewModel 重建生命周期
- [x] Core Automation Tests（CoverageConfig / LayerStack / WidgetCache）
- [x] `IUIManagedWidget` 生命周期协议 + Open Payload / Close Result
- [ ] WBP 配对（WBP_Root / WBP_Text / WBP_Button / WBP_Dialog）
- [ ] 默认主题 DA_Theme_Default + 样式实例
- [ ] 示例场景
- [x] Dialog Close 与 Manager 追踪一致性（见 §4）

> 放置纠正：`UUIViewModelBase` 与 `UUIWidgetPool` 均落 **Widgets** 而非 Core。
> VM 继承 `UMVVMViewModelBase`（模块 ModelViewViewModel 依赖 UMG）；池的是
> `UUserWidget`（UMG 类型）。放 Core 会破坏「Core 无 UMG」铁律。模块归属≠逻辑耦合。
