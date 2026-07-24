# UIFramework 实施指南

本文结合 `Framework分析报告.md` 中的 Lua UI 框架职责，说明如何在 UE 5.6 中实现为可复用 C++ 插件。
目标不是逐文件翻译 Lua，而是保留其架构价值，并使用 UE 原生的生命周期、反射、异步资源和输入系统。

## 1. 迁移结论

| Lua 框架职责 | UE 插件实现 | 当前落点 |
|---|---|---|
| `UIManager` 中央调度 | `UUIManagerSubsystem` | 已实现 |
| `RootUI` 分层画布 | `UUIRootWidget` + Root WBP | C++ 已实现，WBP 待制作 |
| `UIStack` 单栈、覆盖规则 | `UUILayerStack` + `UUICoverageConfig` | 已实现 |
| `ResLoader` 同步/异步加载 | Soft Class + `FStreamableManager` | 已实现 |
| UI 关闭缓存、阶段缓存 | `UUIWidgetCache` + `EUICachePolicy` | 已实现 |
| View / Logic 的 MVVM | `UUIViewModelBase` + UE MVVM | 已实现基础设施 |
| 控件/全局事件 | UMG 委托 + `UUIEventBusSubsystem` | 已实现基础设施 |
| 动态子 UI | Manager/Layer Push，复杂组合由业务 WBP 完成 | 基础能力已具备 |
| 列表复用 | `UListView` + `UUIListBase` | 已实现 |
| 高频临时控件复用 | `UUIWidgetPool` | 已实现 |
| Android 返回/手柄输入 | CommonUI Action Router + `HandleBackAction` | 已实现机制，项目需绑定输入 |
| 红点、埋点、拍脸队列、3D 展示 | 独立可选模块 | 不应进入首版核心 |

Lua 的 `Class.lua`、元表属性拦截和手写对象生命周期不应照搬。C++ 中分别由 `UCLASS`、UE MVVM
Field Notification、`UObject/Subsystem/UserWidget` 生命周期代替，能得到编辑器支持、GC 安全和蓝图可见性。

## 2. 模块边界

依赖方向固定为：

```text
Game Project -> UIFrameworkWidgets -> UIFrameworkCore
                    ^
                    |
             UIFrameworkEditor
```

- `UIFrameworkCore`：不依赖 UMG，只放 GameplayTag 事件总线、纯类型和未来可测试的规则。
- `UIFrameworkWidgets`：放 UMG、CommonUI、MVVM、层栈、管理器、缓存、池和通用控件。
- `UIFrameworkEditor`：只放校验器、资产创建器和编辑器面板，Shipping 不加载。
- 插件不得 include 项目 Game Module；业务通过 DataAsset、GameplayTag、继承和委托接入。

## 3. 运行时主链路

```text
OpenUI(Tag)
  -> Registry 查 FUIWidgetEntry
  -> WidgetCache 查实例/类
  -> SoftClass 同步或异步加载
  -> EnsureRoot 创建 Root WBP
  -> LayerSubsystem 转发
  -> RootWidget 选择层容器
  -> LayerStack 插帧、计算覆盖、激活 CommonUI 屏

CloseUI(Tag) / CloseWidget(Instance)
  -> LayerStack 精确移除
  -> LayerStack 统一广播移除（也覆盖 Back/Pop/Clear/teardown）
  -> Manager 清理打开记录
  -> WidgetCache 按策略销毁或保留
  -> LayerStack 恢复可见性、焦点和输入
```

业务层日常只调用 `UUIManagerSubsystem`。`UUILayerSubsystem` 是低层机制，适合无注册表的临时控件；
同一个 UI 不应混用两种入口，否则 Manager 无法拥有完整生命周期。

## 4. 项目接入步骤

1. 启用 `UIFramework`，确认 CommonUI 和 ModelViewViewModel 随插件启用。
2. 创建 `WBP_Root`，父类设为 `UUIRootWidget`。
3. 在 Root 中创建九个全屏 Panel，按从低到高命名并排列：`Layer_PersistentSystem`、
   `Layer_Loading`、`Layer_Background`、`Layer_Dock`、`Layer_FullWindow`、
   `Layer_PopupWindow`、`Layer_Guide`、`Layer_Notification`、`Layer_Tips`。
   `EUILayer::Max` 是哨兵，不创建对应 Panel。
4. 创建 `DA_UIRegistry`，用 GameplayTag 配置 WidgetClass、Layer、重复规则和 CachePolicy。
5. 创建 `DA_UICoverage`，例如 FullWindow 覆盖 Dock；未配置的组合保持同时可见。
6. 在 Project Settings > Game > UI Framework 中指定 Root、Registry 和 CoverageConfig。
7. 页面 WBP 继承 `UUIActivatableScreenBase`，叶子控件继承 `UUIViewModelWidgetBase`。
8. 页面关闭调用 `CloseUI(Tag)`；控件不知道自身 Tag 时调用 `CloseWidget(this)`。
9. 将返回输入绑定到 Manager/Layer 的 `HandleBackAction`，不要同时再绑定 CommonUI 的第二套返回处理。

C++ 打开页面：

```cpp
if (UUIManagerSubsystem* UI = UUIManagerSubsystem::Get(this))
{
    UI->OpenUI(TAG_UI_Inventory);
}
```

携带参数并接收关闭结果：

```cpp
UUserWidget* Widget = UI->OpenUI(TAG_UI_Inventory, OpenPayload);
UI->CloseWidget(Widget, CloseResult);
```

继承 `UUIActivatableScreenBase` 或 `UUIViewModelWidgetBase` 后，可在 C++/蓝图覆盖：

```text
OnUIRestoredFromCache
OnUIOpening / OnUIOpened
OnUIActivated / OnUIDeactivated
OnUIClosing / OnUIClosed
```

不要在这些回调中直接 `RemoveFromParent`。关闭必须回到 Manager，才能保留 Result、关闭原因、
栈激活切换和缓存一致性。

蓝图使用对应的 `Get UIManagerSubsystem`、`Open UI`、`Close UI` 节点即可。

## 5. MVVM 使用约束

- ViewModel 只保存展示状态与命令，不直接查找 WBP 控件。
- Model/业务系统通过接口、事件总线或 ViewModel 初始化参数提供数据，不让插件反向依赖项目类型。
- 简单一次性 UI 可不用 MVVM；跨多个状态、需要测试或频繁更新的页面再使用。
- ViewModel 变更使用 UE Field Notification，避免 Tick 轮询和 Lua 式全局元表拦截。
- Widget 销毁或进入缓存前必须停止外部监听；恢复时重新同步状态，避免缓存实例持有过期业务对象。
- 框架在 Widget `NativeDestruct` 时 Shutdown ViewModel，缓存实例重新 Construct 时会重新注入并
  Initialize 同一个 ViewModel，从而保留状态并恢复订阅。

## 6. 缓存策略

| 策略 | 适用对象 | 行为 |
|---|---|---|
| `Transient` | 低频大页面 | 关闭后释放实例和类引用 |
| `CacheClass` | 偶尔重开页面 | 保留类，重建实例 |
| `KeepUntilIdle` | 短时间频繁开关 | 保留实例，TTL 到期释放 |
| `KeepUntilSceneChange` | 当前关卡高频 UI | 切图时释放 |
| `KeepPersistent` | 全会话核心 UI | 仅会话结束释放 |

不要直接复制参考框架的固定 100 个 LRU 容量。UE Widget 的资源体积差异很大，按用途显式配置比按数量
驱逐更可控；后续如需内存压力回收，应基于预算和 `FCoreDelegates` 内存告警统一处理。

## 7. 首个可用版本的验收标准

- Editor 与 UnrealGame 两个目标均能独立 `BuildPlugin`，证明不依赖当前 Game Module。
- Root 未配置、Tag 未注册、SoftClass 加载失败时记录明确日志且不崩溃。
- Menu 覆盖 HUD、Modal 覆盖 Menu 后，可见性、焦点、手柄输入和返回顺序一致。
- 同一 Tag 的单实例页面不会重复创建；关闭后 Manager 记录与 Cache 一致。
- `Back`、直接 Layer Pop、清层和 Root teardown 后，Manager 记录同样保持一致。
- 重复异步打开共享一次加载；取消后不会在回调到达时重新弹出页面。
- 切图时 `KeepUntilSceneChange` 被释放，`KeepPersistent` 保留。
- 100+ 条列表使用 `UListView` 虚拟化，不用 ScrollBox 创建全部条目。

## 8. 后续扩展顺序

1. 先制作 Root/通用控件 WBP、默认 Theme/DataAsset 和示例地图，完成端到端可运行样例。
2. 继续给 Manager 的 GameInstance 集成路径增加 Automation Tests；覆盖矩阵、层栈和 Cache 已有基础测试。
3. Editor 模块增加 Registry 校验：重复/无效 Tag、空 WidgetClass、层与基类不匹配。
4. 业务确有需求后，再以独立模块加入弹窗队列、红点/埋点和 3D UI，不污染核心依赖。

参考报告中最值得保留的是“中央调度、明确生命周期、层栈、数据驱动、异步加载和缓存策略”；
红点、拍脸、3D 场景锁、Lua 沙箱属于具体项目规模下的扩展，不应成为通用插件的首版前置条件。
