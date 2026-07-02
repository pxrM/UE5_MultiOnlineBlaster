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
│   ├── UIFrameworkWidgets/       # Runtime, 依赖 UMG/CommonUI (控件基类 + Root)
│   │   ├── UIFrameworkWidgets.Build.cs
│   │   ├── Public/
│   │   └── Private/
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

- **层 (Layer)**：固定顺序，决定谁盖谁。HUD 底，Menu 中，Popup/Modal 顶
- **栈 (Stack)**：层内动态 push/pop，管页面前后 + 激活/返回/焦点

层定 Z 顺序（固定），栈定层内先后（动态）。

### Root WBP 结构

单一根 WBP 作全局入口，内部每层一个容器，外层用 Overlay 靠子顺序自动叠：

```
WBP_Root (继承 C++ 基类 UUIRootWidget)
└── Overlay (根容器, 撑满屏)
    ├── Layer_GameHUD    ← 层0  底  血条/准星/小地图
    ├── Layer_Menu       ← 层1      菜单/背包/设置
    ├── Layer_Popup      ← 层2      提示/Toast
    └── Layer_Modal      ← 层3  顶  确认框/加载遮罩
```

设计要点：

- **层间容器用 Overlay**：子控件按添加顺序叠，后加盖前面。拖动排序即定层级，不手填 ZOrder
- **层内摆控件用 Canvas**：同层内 tooltip 盖按钮等，用 CanvasPanelSlot ZOrder 微调
- **每层是一个栈容器**：页面不直接 AddChild，经栈逻辑 push/pop，自动管激活/返回/焦点
  - 上 CommonUI：每层用 `CommonActivatableWidgetStack`（自带 push/pop/焦点/返回）
  - 自造轻量：每层 Canvas + Subsystem 管 `TMap<层, 栈>`

### 层枚举

```
UI.Layer.GameHUD
UI.Layer.Menu
UI.Layer.Popup
UI.Layer.Modal
```

自造用 UENUM，CommonUI 方案用 GameplayTag。层值留间隔（0/10/20/30）好插新层。

### C++ 基类接口（UUIRootWidget）

Root WBP 继承此基类，基类 `BindWidget` 拿各层容器引用，暴露：

```
PushToLayer(ELayer Layer, TSubclassOf<UUserWidget> WidgetClass) -> UUserWidget*
PopFromLayer(ELayer Layer)
PopAll(ELayer Layer)
HandleBackAction()          // 返回键：顶层栈 pop
```

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
- [x] `UUIRootWidget` C++ 基类（层引用 + push/pop + 返回处理）
- [x] `UUILayerSubsystem`（自造方案，单 viewport；分屏可升 LocalPlayerSubsystem）
- [x] 样式 DataAsset 类（Button/Text/Panel/Theme）
- [x] 控件基类 Text / Button（读样式 + PreConstruct 预览）
- [x] 控件基类 Dialog（推 Modal 层 + OnConfirmed/OnCancelled + 自动弹栈）
- [ ] 控件基类 List（用 UListView 虚拟化）
- [ ] WBP 配对（WBP_Root / WBP_Text / WBP_Button / WBP_Dialog）
- [ ] 默认主题 DA_Theme_Default + 样式实例
- [ ] 示例场景
- [ ] 控件池（仅高频增删场景需要；UListView 覆盖则免）
