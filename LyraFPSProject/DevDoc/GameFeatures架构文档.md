# Lyra GameFeatures 架构详解

## 目录

- [Lyra GameFeatures 架构详解](#lyra-gamefeatures-架构详解)
  - [目录](#目录)
  - [1. 架构概览](#1-架构概览)
  - [2. 引擎层：GameFeatures 框架](#2-引擎层gamefeatures-框架)
    - [2.1 UGameFeaturesSubsystem](#21-ugamefeaturessubsystem)
    - [2.2 UGameFeatureData](#22-ugamefeaturedata)
    - [2.3 UGameFeatureAction (基类)](#23-ugamefeatureaction-基类)
    - [2.4 UGameFeaturesProjectPolicies (策略基类)](#24-ugamefeaturesprojectpolicies-策略基类)
    - [2.5 IGameFeatureStateChangeObserver](#25-igamefeaturestatechangeobserver)
  - [3. 项目配置层：LyraGameFeaturePolicy](#3-项目配置层lyragamefeaturepolicy)
    - [3.1 类定义](#31-类定义)
    - [3.2 两个内建 Observer](#32-两个内建-observer)
      - [Observer 1: ULyraGameFeature\_HotfixManager](#observer-1-ulyragamefeature_hotfixmanager)
      - [Observer 2: ULyraGameFeature\_AddGameplayCuePaths](#observer-2-ulyragamefeature_addgameplaycuepaths)
    - [3.3 客户端/服务器加载策略](#33-客户端服务器加载策略)
  - [4. 项目执行层：GameFeatureAction 体系](#4-项目执行层gamefeatureaction-体系)
    - [4.1 UGameFeatureAction\_WorldActionBase（所有 Action 的公共基类）](#41-ugamefeatureaction_worldactionbase所有-action-的公共基类)
    - [4.2 GameFeatureAction\_AddAbilities](#42-gamefeatureaction_addabilities)
    - [4.3 GameFeatureAction\_AddInputContextMapping](#43-gamefeatureaction_addinputcontextmapping)
    - [4.4 GameFeatureAction\_AddInputBinding](#44-gamefeatureaction_addinputbinding)
    - [4.5 GameFeatureAction\_AddWidgets](#45-gamefeatureaction_addwidgets)
    - [4.6 GameFeatureAction\_AddGameplayCuePath](#46-gamefeatureaction_addgameplaycuepath)
    - [4.7 GameFeatureAction\_SplitscreenConfig](#47-gamefeatureaction_splitscreenconfig)
  - [5. 内容层：GameFeature 插件](#5-内容层gamefeature-插件)
    - [5.1 与普通插件的区别](#51-与普通插件的区别)
    - [5.2 ShooterCore（射击核心）](#52-shootercore射击核心)
    - [5.3 LyraFPSGame（你的自定义 FPS 插件）](#53-lyrafpsgame你的自定义-fps-插件)
    - [5.4 其他 GF 插件](#54-其他-gf-插件)
  - [6. 运行时流程：从启动到游戏运行](#6-运行时流程从启动到游戏运行)
  - [7. Experience 系统：GameFeature 的调度者](#7-experience-系统gamefeature-的调度者)
    - [7.1 核心数据结构](#71-核心数据结构)
    - [7.2 层级关系](#72-层级关系)
    - [7.3 Experience 与 GameFeature 的关系](#73-experience-与-gamefeature-的关系)
  - [8. 完整时序图](#8-完整时序图)
  - [9. 如何添加新的 GameFeature 插件](#9-如何添加新的-gamefeature-插件)
    - [9.1 插件目录结构](#91-插件目录结构)
    - [9.2 .uplugin 编写要点](#92-uplugin-编写要点)
    - [9.3 GameFeatureData 资产配置](#93-gamefeaturedata-资产配置)
    - [9.4 Experience 引用与激活机制](#94-experience-引用与激活机制)
  - [附录](#附录)
    - [关键类继承关系](#关键类继承关系)
    - [相关文件索引](#相关文件索引)

---


## 1. 架构概览

整个 GameFeatures 系统分为四层，从上到下：框架 → 策略 → Action → 内容。

```
第一层：引擎 GameFeatures 插件 (框架层)
路径: Engine/Plugins/Runtime/GameFeatures/
├── UGameFeaturesSubsystem        — 插件生命周期 (加载/激活/卸载)
├── UGameFeatureData              — 插件数据资产基类 (PrimaryAsset)
├── UGameFeatureAction            — 激活时执行的动作基类
├── UGameFeaturesProjectPolicies  — 项目策略基类
└── IGameFeatureStateChangeObserver — 状态变化观察者接口
    │
    │  继承 / 实现
    ▼
第二层：项目策略
路径: Source/LyraGame/GameFeatures/LyraGameFeaturePolicy
├── ULyraGameFeaturePolicy     — 决定插件白名单、预加载策略
├── Observer: HotfixManager    — 插件加载时触发热修复
└── Observer: GameplayCuePaths — 自动注册/注销 GameplayCue 路径
    │
    │  使用 Action
    ▼
第三层：自定义 Action
路径: Source/LyraGame/GameFeatures/GameFeatureAction_*.h/.cpp
├── WorldActionBase             — 所有 Action 的公共基类 (监听 World)
├── AddAbilities                — 授予技能 / AttributeSet / AbilitySet
├── AddInputContextMapping      — 注入 EnhancedInput MappingContext
├── AddInputBinding             — 绑定 LyraInputConfig
├── AddWidgets                  — 添加 HUD 布局和控件
├── AddGameplayCuePath          — 注册 GameplayCue 搜索路径
└── SplitscreenConfig           — 控制分屏开关 (投票机制)
    │
    │  数据引用 (由 GameFeatureData 资产配置)
    ▼
第四层：GF 插件内容
路径: Plugins/GameFeatures/
├── ShooterCore      — 射击核心玩法 (Registered, 按需激活)
├── LyraFPSGame      — FPS 自定义内容 (Active, 启动即激活) ★
├── ShooterMaps      — 射击关卡地图
├── ShooterExplorer  — 冒险扩展
├── ShooterTests     — 自动化测试 (Shipping 不编译)
└── TopDownArena     — 俯视角竞技场 (完全独立的另一套玩法)
```

---

## 2. 引擎层：GameFeatures 框架

### 2.1 UGameFeaturesSubsystem

**职责**：全局单例，管理所有 GameFeature 插件的生命周期。

项目中实际使用的 API（验证自项目源代码）：

| 方法 | 作用 | 调用位置 |
|---|---|---|
| `Get()` | 获取全局单例 | 多处 |
| `LoadAndActivateGameFeaturePlugin(PluginURL, CompleteDelegate)` | 加载并激活插件 | `LyraExperienceManagerComponent.cpp:269` |
| `DeactivateGameFeaturePlugin(PluginURL)` | 反激活插件 | `LyraExperienceManagerComponent.cpp:390` |
| `GetPluginURLByName(PluginName, PluginURL)` | 通过插件名查找插件 URL | `LyraExperienceManagerComponent.cpp:231` |
| `GetPolicy<TPolicy>()` | 获取项目策略实例 | `LyraGameFeaturePolicy.cpp:16` |
| `AddObserver(Observer)` / `RemoveObserver(Observer)` | 添加/移除状态变化观察者 | `LyraGameFeaturePolicy.cpp:27,39` |
| `FixPluginPackagePath(MutablePath, PluginRootPath, ...)` | 将相对路径修正为插件绝对路径 (静态方法) | `LyraGameFeaturePolicy.cpp:110,145` |

> 注：引擎文件路径未在项目中可验证，此处不再列出。插件发现/扫描由 Subsystem 在 Initialize 阶段内部完成，未暴露独立函数给项目调用。

### 2.2 UGameFeatureData

**职责**：每个 GF 插件在 Content 目录下有一个 `GameFeatureData` 资产（PrimaryAsset），它引用该插件要执行的 Action 列表。引擎会自动查找插件根目录下的 `GameFeatureData` 资产。

```
GameFeatureData
  ├── GameFeaturesToEnable[]  — 子插件列表
  └── Actions[]               — UGameFeatureAction 实例列表
        ├── AddAbilities
        ├── AddInputMapping
        ├── AddWidgets
        └── ...
```

### 2.3 UGameFeatureAction (基类)

**职责**：所有 Action 的抽象基类，提供虚函数让子类重写。以下生命周期方法均验证自项目代码：

| 方法 | 触发时机 |
|---|---|
| `OnGameFeatureRegistering()` | 插件注册时 |
| `OnGameFeatureLoading()` | 插件内容加载时 |
| `OnGameFeatureActivating(Context)` | 插件激活时（主入口） |
| `OnGameFeatureDeactivating(Context)` | 插件反激活时 |
| `OnGameFeatureUnregistering()` | 插件注销时 |

### 2.4 UGameFeaturesProjectPolicies (策略基类)

**职责**：允许项目定义自定义策略。以下方法均被 `ULyraGameFeaturePolicy` 重写，验证自 `LyraGameFeaturePolicy.h`：

| 方法 | 作用 |
|---|---|
| `InitGameFeatureManager()` | 初始化 GF 管理器 |
| `ShutdownGameFeatureManager()` | 关闭 GF 管理器 |
| `GetGameFeatureLoadingMode(bLoadClient, bLoadServer)` | 决定客户端/服务器各加载哪些数据 |
| `GetPreloadAssetListForGameFeature(GFData)` | 预加载资源列表 |
| `IsPluginAllowed(PluginURL, OutReason)` | 插件白名单检查 |
| `GetPreloadBundleStateForGameFeature()` | 预加载 Bundle 状态 |

### 2.5 IGameFeatureStateChangeObserver

**职责**：观察者接口，监听 GF 插件的状态变化。以下方法验证自 `LyraGameFeaturePolicy.cpp` 中两个 Observer 的实现：

| 方法 | 触发时机 |
|---|---|
| `OnGameFeatureRegistering(GFData, PluginName, PluginURL)` | 插件注册时 |
| `OnGameFeatureUnregistering(GFData, PluginName, PluginURL)` | 插件注销时 |
| `OnGameFeatureLoading(GFData, PluginURL)` | 插件内容加载时 |
| `OnGameFeatureActivating(GFData, PluginURL)` | 插件激活时 |
| `OnGameFeatureDeactivating(GFData, PluginURL)` | 插件反激活时 |

---

## 3. 项目配置层：LyraGameFeaturePolicy

### 3.1 类定义

**位置**：`Source/LyraGame/GameFeatures/LyraGameFeaturePolicy.h`

```cpp
// 主策略类
UCLASS(MinimalAPI, Config = Game)
class ULyraGameFeaturePolicy : public UDefaultGameFeaturesProjectPolicies
{
    static ULyraGameFeaturePolicy& Get();                    // 快捷获取单例
    virtual void InitGameFeatureManager() override;          // 初始化
    virtual void ShutdownGameFeatureManager() override;      // 关闭
    virtual void GetGameFeatureLoadingMode(...) override;    // 客户端/服务器加载策略
    virtual bool IsPluginAllowed(...) const override;        // 白名单
    virtual TArray<FPrimaryAssetId> GetPreloadAssetListForGameFeature(...) const override;
private:
    TArray<TObjectPtr<UObject>> Observers;                   // 状态观察者列表
};
```

### 3.2 两个内建 Observer

LyraGameFeaturePolicy 在 `InitGameFeatureManager()` 中注册了两个 Observer：

#### Observer 1: ULyraGameFeature_HotfixManager

**触发**：`OnGameFeatureLoading()` — GF 插件内容加载时

**行为**：调用 `ULyraHotfixManager::RequestPatchAssetsFromIniFiles()` 检查是否需要热修复。

#### Observer 2: ULyraGameFeature_AddGameplayCuePaths

**触发**：
- `OnGameFeatureRegistering()` — 自动注册 GF 插件中的 GameplayCue 路径
- `OnGameFeatureUnregistering()` — 自动移除路径

**行为**：遍历 GF 插件中所有 `UGameFeatureAction_AddGameplayCuePath`，将其路径注册到 `UGameplayCueManager`，然后重建运行时对象库。

### 3.3 客户端/服务器加载策略

`GetGameFeatureLoadingMode()` 实现：
```cpp
void ULyraGameFeaturePolicy::GetGameFeatureLoadingMode(bool& bLoadClientData, bool& bLoadServerData) const
{
    bLoadClientData = !IsRunningDedicatedServer();  // DS 不加载客户端数据
    bLoadServerData = !IsRunningClientOnly();       // 纯客户端不加载服务器数据
}
```

---

## 4. 项目执行层：GameFeatureAction 体系

### 4.1 UGameFeatureAction_WorldActionBase（所有 Action 的公共基类）

**文件**：
- `Source/LyraGame/GameFeatures/GameFeatureAction_WorldActionBase.h`
- `Source/LyraGame/GameFeatures/GameFeatureAction_WorldActionBase.cpp`

**职责**：监听 World 的创建。

```cpp
void OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
    // 1. 注册 FWorldDelegates::OnStartGameInstance 回调
    GameInstanceStartHandles.FindOrAdd(Context) = FWorldDelegates::OnStartGameInstance.AddUObject(
        this, &HandleGameInstanceStart, Context);

    // 2. 对已存在的 World 立即执行
    for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
    {
        if (Context.ShouldApplyToWorldContext(WorldContext))
        {
            AddToWorld(WorldContext, Context);  // → 由子类实现
        }
    }
}
```

**核心机制**：`UGameFrameworkComponentManager::AddExtensionHandler()` — 这是 Action 系统最关键的模式。每个 Action 通过 ExtensionHandler 监听特定 Actor 类的添加/移除事件，从而在正确的时机注入内容。

### 4.2 GameFeatureAction_AddAbilities

**作用**：给指定类型的 Actor 批量授予 GAS 技能、AttributeSet、AbilitySet。

**配置结构**：
```cpp
USTRUCT()
struct FGameFeatureAbilitiesEntry
{
    TSoftClassPtr<AActor> ActorClass;                              // 目标 Actor 类型
    TArray<FLyraAbilityGrant> GrantedAbilities;                    // 技能列表
    TArray<FLyraAttributeSetGrant> GrantedAttributes;              // 属性集列表
    TArray<TSoftObjectPtr<const ULyraAbilitySet>> GrantedAbilitySets; // 技能集列表
};
```

**工作流程**：

```
GameFeature 激活
  ↓
AddToWorld() → 通过 GameFrameworkComponentManager 注册 ActorClass 的扩展处理器
  ↓
当匹配的 ActorClass 实例被创建
  ↓
HandleActorExtension() 被调用 (EventName = ExtensionAdded)
  ↓
AddActorAbilities():
  1. FindOrAddComponentForActor → 确保目标 Actor 有 UAbilitySystemComponent
  2. 遍历 GrantedAbilities → AbilitySystemComponent->GiveAbility()
  3. 遍历 GrantedAttributes → NewObject<UAttributeSet> → AddAttributeSetSubobject()
  4. 遍历 GrantedAbilitySets → Set->GiveToAbilitySystem()
  ↓
记录到 ActiveExtensions (用于反激活时清理)
```

**清理流程**：`RemoveActorAbilities()` 反向执行——移除 AttributeSet、标记 Ability 移除、回收 AbilitySet。

### 4.3 GameFeatureAction_AddInputContextMapping

**作用**：往 EnhancedInput 系统注册/注销 InputMappingContext，支持优先级。

**两步机制**：

**第一步（Registering 阶段）**：注册到 `UEnhancedInputUserSettings`（全局输入设置）
```
OnGameFeatureRegistering()
  └→ 监听 GameInstance 的创建
       └→ 当 LocalPlayer 加入时 → Settings->RegisterInputMappingContext(IMC)
```

**第二步（Activating 阶段）**：注入到活跃的 PlayerController
```
OnGameFeatureActivating()
  └→ AddToWorld()
       └→ ExtensionHandler 监听 APlayerController
            └→ 当 PC 扩展时 → EnhancedInputSubsystem->AddMappingContext(IMC, Priority)
```

**配置结构**：
```cpp
USTRUCT()
struct FInputMappingContextAndPriority
{
    TSoftObjectPtr<UInputMappingContext> InputMapping;
    int32 Priority = 0;
    bool bRegisterWithSettings = true;  // 是否注册到全局设置
};
```

**清理**：`OnGameFeatureDeactivating` 时移除 MappingContext，`OnGameFeatureUnregistering` 时从 Settings 中注销。

### 4.4 GameFeatureAction_AddInputBinding

**作用**：通过 `ULyraInputConfig` 给 Pawn 绑定输入配置。

与 `AddInputContextMapping` 的区别：
- `AddInputContextMapping`：直接操作 EnhancedInput 的 IMC
- `AddInputBinding`：通过 Lyra 的 `ULyraInputConfig` 系统 + `ULyraHeroComponent` 的 `AddAdditionalInputConfig()`

**流程**：
```
AddToWorld() → ExtensionHandler 监听 APawn
  ↓
当 Pawn 扩展时 → AddInputMappingForPlayer()
  ↓
检查 HeroComponent->IsReadyToBindInputs()
  ↓
HeroComponent->AddAdditionalInputConfig(InputConfig)
```

### 4.5 GameFeatureAction_AddWidgets

**作用**：激活时往 HUD 添加 UI 布局和控件，反激活时移除。

**两个功能**：

| 功能 | 配置来源 | 实现方式 |
|---|---|---|
| Layout（布局） | `TArray<FLyraHUDLayoutRequest>` | `CommonUIExtensions::PushContentToLayer_ForPlayer()` |
| Widgets（控件） | `TArray<FLyraHUDElementEntry>` | `UUIExtensionSubsystem::RegisterExtensionAsWidgetForContext()` |

**流程**：
```
AddToWorld() → ExtensionHandler 监听 ALyraHUD
  ↓
当 HUD Actor 扩展时 → AddWidgets()
  ↓
遍历 Layout → PushContentToLayer_ForPlayer(LayerID, WidgetClass)
遍历 Widgets → ExtensionSubsystem->RegisterExtensionAsWidgetForContext(SlotID, Player, WidgetClass)
  ↓
反激活时 → DeactivateWidget() / Handle.Unregister()
```

### 4.6 GameFeatureAction_AddGameplayCuePath

**作用**：将 GF 插件的目录注册到 GameplayCueManager 的搜索路径中。

**继承关系**：直接继承 `UGameFeatureAction`（非 WorldActionBase），因为它不是 World 级操作，而是全局注册。

**默认路径**：构造函数默认添加 `/GameplayCues` 目录。

**实际执行**：由 `ULyraGameFeature_AddGameplayCuePaths` Observer 在 `OnGameFeatureRegistering` 时调用：
```cpp
GCM->AddGameplayCueNotifyPath(MutablePath, false);
GCM->InitializeRuntimeObjectLibrary();
```

### 4.7 GameFeatureAction_SplitscreenConfig

**作用**：控制是否禁用分屏。

**机制**：使用全局投票计数器 `GlobalDisableVotes`，多个 Action 可以投票禁用分屏。只有投票数归零时才重新启用。

```cpp
void AddToWorld(...)
{
    if (bDisableSplitscreen)
    {
        int32& VoteCount = GlobalDisableVotes.FindOrAdd(ViewportKey);
        VoteCount++;
        if (VoteCount == 1)
        {
            VC->SetForceDisableSplitscreen(true);
        }
    }
}
```

---

## 5. 内容层：GameFeature 插件

### 5.1 与普通插件的区别

| 属性 | 普通插件 | GameFeature 插件 |
|---|---|---|
| 注册方式 | `.uproject` → Plugins 列表 | 引擎自动扫描 `Plugins/GameFeatures/` |
| 加载时机 | 引擎启动时 | 运行时 Experience 按需激活 |
| 能否卸载 | 否 | 可以运行时注销 |
| 关键字段 | 无 | `ExplicitlyLoaded`, `BuiltInInitialFeatureState` |

### 5.2 ShooterCore（射击核心）

**路径**：`Plugins/GameFeatures/ShooterCore/`

**BuiltInInitialFeatureState**：`Registered`（启动时注册但不激活）

**内容模块**：`ShooterCoreRuntime`，包含：

| 系统 | 文件 | 描述 |
|---|---|---|
| 瞄准辅助 | `AimAssistInputModifier.h` | 增强输入修饰器 |
| 瞄准辅助 | `AimAssistTargetComponent.h` | 辅助瞄准目标组件 |
| 瞄准辅助 | `AimAssistTargetManagerComponent.h` | 辅助瞄准管理组件 |
| 消息系统 | `ElimStreakProcessor.h` / `ElimChainProcessor.h` / `AssistProcessor.h` | 击杀相关消息处理器 |
| 消息系统 | `ControlPointStatusMessage.h` | 控制点状态消息 |
| 荣誉系统 | `LyraAccoladeDefinition.h` / `LyraAccoladeHostWidget.h` | 击杀荣誉/奖励 |
| 出生系统 | `TDM_PlayerSpawningManagmentComponent.h` | 团队死斗出生管理 |
| 世界 | `LyraWorldCollectable.h` | 可收集物 |
| 设置 | `ShooterCoreRuntimeSettings.h` | 运行时配置 |

### 5.3 LyraFPSGame（你的自定义 FPS 插件）

**路径**：`Plugins/GameFeatures/LyraFPSGame/`

**BuiltInInitialFeatureState**：`Active`（启动即激活）
**EnabledByDefault**：`true`
**作者**：`wzh`

**内容模块**：`LyraFPSGameRuntime`，包含：

| 系统 | 文件 | 描述 |
|---|---|---|
| 角色 | `LyraFPSCharacter.h` | FPS 角色 |
| 角色 | `LyraHeroFPSComponent.h` | FPS 英雄组件 |
| 角色 | `LyraFPSCameraComponent.h` | FPS 摄像头组件 |
| 角色 | `LyraFPSCameraMode.h` | FPS 摄像头模式 |
| 移动 | `LyraFPSCharacterMovementComponent.h` | FPS 人物移动 |
| GAS | `LyraFPSAbilitySystemComponent.h` | FPS ASC |
| GAS | `GameplayAbility_Sprint.h` | 冲刺技能 |
| GAS | `AbilityTask_PlayMontageForCustomMeshAndWait.h` | 自定义网格播放蒙太奇 |
| GAS | `FPSMontageSelectEntry.h` | FPS 蒙太奇选择 |
| 武器 | `LyraFPSWeaponBase.h` | FPS 武器基类 |
| 武器 | `LyraRangedWeaponInstance_FPS.h` | 远程武器实例 |
| 装备 | `LyraEquipmentManagerComponent_FPS.h` | FPS 装备管理 |
| 装备 | `LyraEquipmentDefinition_FPS.h` | FPS 装备定义 |
| 装备 | `LyraQuickBarComponent_FPS.h` | 快捷栏 |
| 外观 | `SkeletalMeshComponent_FPS.h` | FPS 骨骼网格 |
| 外观 | `LyraControllerComp_CharacterParts_FPS.h` | 角色部件控制 |
| 外观 | `LyraPawnComp_CharacterParts_FPS.h` | 角色部件 Pawn |
| 外观 | `CharacterPartData_FPS.h` | 角色部件数据 |
| 游戏 | `LyraFPSGameMode.h` | FPS GameMode |
| 游戏 | `LyraFPSPlayerState.h` | FPS PlayerState |
| 出生 | `LyraFPSPlayerSpawningManagementComponent.h` | FPS 出生管理 |
| 工具 | `LyraFPSFunctionLibrary.h` | FPS 工具函数库 |
| 调试 | `FPSDebugComponent.h` | FPS 调试组件 |
| 标签 | `LyraFPSGameplayTags.h` | FPS GameplayTags |
| 标签 | `LyraTaggedActor_FPS.h` | FPS 标记 Actor |
| 武器 | `LyraFPSWeaponSpawnerDefinitionComponent.h` | 武器生成定义 |

### 5.4 其他 GF 插件

| 插件 | 依赖 | 描述 |
|---|---|---|
| **ShooterExplorer** | ShooterCore + LyraExampleContent | 在 ShooterCore 基础上添加冒险元素 |
| **ShooterMaps** | ShooterCore + LyraFPSGame + LyraExampleContent | 射击游戏关卡地图 |
| **ShooterTests** | ShooterCore + CQTest | 自动化测试（Shipping 不编译） |
| **TopDownArena** | GameplayAbilities + Niagara + LyraExampleContent | 完全独立的俯视角竞技场玩法 |

---

## 6. 运行时流程：从启动到游戏运行

```
┌─────────────────────────────────────────────────────────────────┐
│                    引擎启动阶段                                    │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  UGameFeaturesSubsystem 初始化 (引擎内部)                            │
│    │                                                              │
│    ├── 调用 LyraGameFeaturePolicy::InitGameFeatureManager()       │
│    │     ├── 创建 HotfixManager Observer                           │
│    │     ├── 创建 AddGameplayCuePaths Observer                     │
│    │     ├── Subsystem.AddObserver() → 注册两个 Observer            │
│    │     └── Super::InitGameFeatureManager() → 引擎默认逻辑        │
│    │                                                              │
│    └── 扫描 Plugins/GameFeatures/ 目录，发现 6 个 .uplugin          │
│          (基于各插件的 BuiltInInitialFeatureState 决定初始状态)      │
│                ├── ShooterCore.uplugin  → BuiltInFeatureState=Registered │
│                ├── ShooterExplorer.uplugin → Registered           │
│                ├── ShooterMaps.uplugin → Registered               │
│                ├── ShooterTests.uplugin → Registered              │
│                ├── TopDownArena.uplugin → Registered              │
│                └── LyraFPSGame.uplugin → BuiltInFeatureState=Active │
│                                                                   │
│  (LyraFPSGame 为 Active，其余 Registered 等待 Experience 激活)      │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                    PIE / 游戏开始阶段                               │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  1. 引擎从 Project Settings → GlobalDefaultGameMode 读取          │
│     → B_LyraGameMode_C (基于 ALyraGameMode 的蓝图)                │
│                                                                   │
│  2. 引擎 spawn B_LyraGameMode 实例                                 │
│                                                                   │
│  3. ALyraGameMode::InitGame() 被调用                               │
│     └── SetTimerForNextTick(HandleMatchAssignmentIfNotExpectingOne) │
│                                                                   │
│  4. HandleMatchAssignmentIfNotExpectingOne() 按优先级查找 Experience:│
│     ┌─────────────────────────────────────────────────────────┐  │
│     │ a. Matchmaking 分配 (最高优先级)                          │  │
│     │ b. URL Options 参数 (?Experience=...)                     │  │
│     │ c. Developer Settings (PIE 专用)                          │  │
│     │ d. Command Line 参数 (-Experience=...)                    │  │
│     │ e. ALyraWorldSettings::GetDefaultGameplayExperience()    │  │
│     │ f. Dedicated Server 逻辑                                  │  │
│     │ g. 最终回退 → "B_LyraDefaultExperience"                    │  │
│     └─────────────────────────────────────────────────────────┘  │
│     └── 确定后调用 OnMatchAssignmentGiven(ExperienceId)           │
│                                                                   │
│  5. OnMatchAssignmentGiven()                                      │
│     └── ExperienceManagerComponent->SetCurrentExperience(Id)     │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                   Experience 加载阶段                              │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  6. ULyraExperienceManagerComponent::StartExperienceLoad()       │
│     │                                                              │
│     ├── 加载 Experience Definition + ActionSet 的 AssetBundle      │
│     │                                                              │
│     ├── OnExperienceLoadComplete()                                │
│     │   │                                                          │
│     │   ├── 收集 Experience 中所有 GameFeaturesToEnable 插件名      │
│     │   ├── 对每个插件名通过 UGameFeaturesSubsystem::              │
│     │   │   GetPluginURLByName() 查找 URL                         │
│     │   └── 对每个 PluginURL 调用                                  │
│     │       LoadAndActivateGameFeaturePlugin(URL, OnComplete)      │
│     │                                                              │
│     └── 所有 GF 插件加载完毕 → OnExperienceFullLoadCompleted()     │
│                                                                   │
│  7. OnExperienceFullLoadCompleted()                               │
│     │                                                              │
│     ├── LoadState = ExecutingActions                              │
│     ├── 创建 FGameFeatureActivatingContext                         │
│     └── 依次调用每个 Action:                                       │
│           ├── Action->OnGameFeatureRegistering()                  │
│           ├── Action->OnGameFeatureLoading()                      │
│           └── Action->OnGameFeatureActivating(Context)            │
│                   │                                                │
│                   └──→ WorldActionBase 的子类会调用 AddToWorld()    │
│                         └──→ 注册 ComponentManager ExtensionHandler│
│                               └──→ 等待目标 Actor 出现时注入内容    │
│                                                                   │
│  8. LoadState = Loaded                                            │
│     └── 广播 OnExperienceLoaded 委托                              │
│           └── ALyraGameMode::OnExperienceLoaded() 被调用           │
│                 └── 重启等待中的玩家                                │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                关闭 / 切换关卡                                     │
├─────────────────────────────────────────────────────────────────┤
│                                                                   │
│  EndPlay → 遍历加载的 GF 插件 URL                                  │
│    └── DeactivateGameFeaturePlugin(URL)                           │
│          └── Action->OnGameFeatureDeactivating(Context)           │
│                ├── AddAbilities: RemoveActorAbilities()           │
│                ├── AddInputMapping: RemoveMappingContext()         │
│                ├── AddInputBinding: RemoveAdditionalInputConfig()  │
│                ├── AddWidgets: DeactivateWidget() / Unregister()  │
│                └── SplitscreenConfig: SetForceDisableSplitscreen(false) │
│                                                                   │
└─────────────────────────────────────────────────────────────────┘
```

---

## 7. Experience 系统：GameFeature 的调度者

### 7.1 核心数据结构

**ULyraExperienceDefinition** (`Source/LyraGame/GameModes/LyraExperienceDefinition.h`)：

```cpp
class ULyraExperienceDefinition : public UPrimaryDataAsset
{
    TArray<FString> GameFeaturesToEnable;             // 要激活的 GF 插件名列表
    TObjectPtr<const ULyraPawnData> DefaultPawnData;  // 默认 Pawn 数据
    TArray<TObjectPtr<UGameFeatureAction>> Actions;   // 直接包含的 Action 列表
    TArray<TObjectPtr<ULyraExperienceActionSet>> ActionSets;  // 外部 ActionSet 组合
};
```

### 7.2 层级关系

```
Level (关卡)
  └── ALyraWorldSettings
        └── DefaultGameplayExperience → 指向某个 ULyraExperienceDefinition 资产
              ├── GameFeaturesToEnable: ["ShooterCore", "ShooterMaps"]
              ├── Actions: [AddAbilities, AddWidgets, ...]
              └── ActionSets: [B_ShooterCoreActionSet, ...]
                    └── 每个 ActionSet 也是一个 PrimaryAsset
                          ├── GameFeaturesToEnable: ["..." ]
                          └── Actions: [...]
```

### 7.3 Experience 与 GameFeature 的关系

Experience **驱动** GameFeature 插件的激活：

1. Experience 定义需要哪些 GF 插件（`GameFeaturesToEnable`）
2. `ExperienceManagerComponent` 负责加载这些插件的资产 Bundle
3. 加载完成后调用 `UGameFeaturesSubsystem::LoadAndActivateGameFeaturePlugin()` 激活
4. 激活过程触发 GF 插件中 `GameFeatureData` 里的所有 Action
5. Action 执行具体的游戏逻辑注入

**关键区别**：
- **Experience**：定义了"这个游戏模式需要什么"——哪些 GF 插件、哪些 Action
- **GameFeature 插件**：包含了实际的 C++ 代码和内容资产（武器类、技能类、输入配置等）
- **Action**：桥接两者——在 GF 插件代码和游戏运行时之间建立连接

---

## 8. 完整时序图

```
项目启动
  │
  ├─→ UGameFeaturesSubsystem 初始化
  │     └─→ LyraGameFeaturePolicy::InitGameFeatureManager()
  │           ├─→ 注册 HotfixManager Observer
  │           └─→ 注册 AddGameplayCuePaths Observer
  │
  ├─→ 引擎扫描 Plugins/GameFeatures/，发现 6 个 GF 插件
  │     └─→ LyraFPSGame 因 BuiltInFeatureState=Active 被激活
  │           └─→ 触发该插件的 GameFeatureData 中的 Actions
  │     └─→ 其余 5 个 Registered 状态，等待 Experience 激活
  │
  ├─→ 进入前端关卡 L_LyraFrontEnd
  │
PIE 开始 / 进入游戏关卡
  │
  ├─→ WorldSettings(GameMode Override=空) → 回退到 Project Settings 的 GlobalDefaultGameMode
  │     └─→ spawn B_LyraGameMode_C
  │
  ├─→ ALyraGameMode::InitGame()
  │     └─→ 下一帧: HandleMatchAssignmentIfNotExpectingOne()
  │           └─→ 优先级链查找 Experience
  │                 └─→ 找到 ExperienceId
  │                       └─→ OnMatchAssignmentGiven()
  │
  ├─→ ExperienceManagerComponent::SetCurrentExperience()
  │     └─→ StartExperienceLoad()
  │           ├─→ 加载 Experience + ActionSet 的 AssetBundle
  │           └─→ OnExperienceLoadComplete()
  │                 ├─→ 收集 GameFeaturesToEnable
  │                 ├─→ 对每个: GetPluginURLByName()
  │                 └─→ 对每个: LoadAndActivateGameFeaturePlugin()
  │
  ├─→ [ShooterCore 开始加载]
  │     ├─→ 加载 ShooterCoreRuntime.dll
  │     ├─→ 加载 GameFeatureData 资产
  │     └─→ 遍历 Actions:
  │           ├─→ AddAbilities::OnGameFeatureRegistering()
  │           ├─→ AddAbilities::OnGameFeatureLoading()
  │           └─→ AddAbilities::OnGameFeatureActivating(Context)
  │                 └─→ AddToWorld()
  │                       └─→ ComponentManager->AddExtensionHandler(ALyraCharacter::StaticClass(), ...)
  │
  ├─→ [ShooterMaps 开始加载]
  │     └─→ 同上流程, 注册 Widget、Input 等
  │
  ├─→ 所有 GF 插件加载完毕
  │     └─→ OnExperienceFullLoadCompleted()
  │           ├─→ 执行 Experience 和 ActionSet 级别的 Actions
  │           ├─→ LoadState = Loaded
  │           └─→ 广播 OnExperienceLoaded
  │                 └─→ ALyraGameMode::OnExperienceLoaded()
  │                       └─→ 遍历 PlayerController 列表重启玩家
  │
  ├─→ [游戏运行中]
  │     ├─→ 玩家加入 → APlayerController 被创建
  │     │     └─→ ComponentManager 通知所有 ExtensionHandler
  │     │           ├─→ AddInputMapping: AddMappingContext(IMC, Priority)
  │     │           └─→ ...
  │     │
  │     └─→ Pawn 被 spawn → ExtensionHandler
  │           ├─→ AddAbilities: GiveAbility() / AddAttributeSetSubobject()
  │           └─→ AddInputBinding: HeroComponent->AddAdditionalInputConfig()
  │
  ├─→ [PIE 结束 / 关卡切换]
  │     └─→ EndPlay → DeactivateGameFeaturePlugin()
  │           └─→ 所有 Action 反激活 → 清理技能、输入、UI
  │
  └─→ ShutdownGameFeatureManager()
        └─→ 移除所有 Observer
```

---

## 9. 如何添加新的 GameFeature 插件

### 9.1 插件目录结构

```
Plugins/GameFeatures/MyNewFeature/
├── MyNewFeature.uplugin
├── Source/
│   └── MyNewFeatureRuntime/
│       ├── MyNewFeatureRuntime.Build.cs
│       ├── Public/
│       │   └── MyNewFeatureRuntimeModule.h
│       └── Private/
│           └── MyNewFeatureRuntimeModule.cpp
└── Content/
    └── MyNewFeatureGameFeatureData.uasset  (类型: UGameFeatureData)
```

### 9.2 .uplugin 编写要点

```json
{
    "FileVersion": 3,
    "FriendlyName": "MyNewFeature",
    "Description": "我的新功能",
    "Category": "Game Features",
    "CanContainContent": true,
    "ExplicitlyLoaded": true,
    "EnabledByDefault": false,
    "BuiltInInitialFeatureState": "Registered",
    "Modules": [{
        "Name": "MyNewFeatureRuntime",
        "Type": "Runtime",
        "LoadingPhase": "Default"
    }]
}
```

### 9.3 GameFeatureData 资产配置

在编辑器中：
1. 右键 → Miscellaneous → Data Asset → GameFeatureData
2. 命名为 `MyNewFeatureGameFeatureData`
3. 在 `Actions` 数组中添加需要的 Action：
   - Add Abilities → 配置要授权的技能
   - Add Input Mapping → 配置输入映射
   - Add Widgets → 配置 UI
   - Add Gameplay Cue Path → 配置 Cue 路径
   - Splitscreen Config → 禁用分屏

### 9.4 Experience 引用与激活机制

在 Experience Definition 资产中，将 `"MyNewFeature"` 添加到 `GameFeaturesToEnable` 数组。

激活时机根据需求选择：
- `BuiltInInitialFeatureState: "Active"` — 启动即激活（如 LyraFPSGame）
- `BuiltInInitialFeatureState: "Registered"` + 在 Experience 中引用 — 由关卡按需激活（如 ShooterCore）

---

## 附录

### 关键类继承关系

```
UGameFeatureAction                          (引擎层 — 基类)
  ├── UGameFeatureAction_WorldActionBase    (项目层 — 监听 World)
  │     ├── UGameFeatureAction_AddAbilities
  │     ├── UGameFeatureAction_AddInputContextMapping
  │     ├── UGameFeatureAction_AddInputBinding
  │     ├── UGameFeatureAction_AddWidgets
  │     └── UGameFeatureAction_SplitscreenConfig
  └── UGameFeatureAction_AddGameplayCuePath (直接继承 — 全局注册)

UGameFeaturesProjectPolicies                (引擎层 — 策略基类)
  └── UDefaultGameFeaturesProjectPolicies   (引擎层 — 默认实现)
        └── ULyraGameFeaturePolicy          (项目层 — 自定义策略)

IGameFeatureStateChangeObserver             (引擎层 — 观察者接口)
  ├── ULyraGameFeature_HotfixManager        (项目层)
  └── ULyraGameFeature_AddGameplayCuePaths  (项目层)

ALyraWorldSettings : AWorldSettings          (关卡层 — 定义默认 Experience)
```

### 相关文件索引

| 用途 | 路径 |
|---|---|
| GF 策略 | `Source/LyraGame/GameFeatures/LyraGameFeaturePolicy.h/.cpp` |
| Action 基类 | `Source/LyraGame/GameFeatures/GameFeatureAction_WorldActionBase.h/.cpp` |
| 授技能 | `Source/LyraGame/GameFeatures/GameFeatureAction_AddAbilities.h/.cpp` |
| 输映射 | `Source/LyraGame/GameFeatures/GameFeatureAction_AddInputContextMapping.h/.cpp` |
| 输绑定 | `Source/LyraGame/GameFeatures/GameFeatureAction_AddInputBinding.h/.cpp` |
| UI 控件 | `Source/LyraGame/GameFeatures/GameFeatureAction_AddWidget.h/.cpp` |
| Cue 路径 | `Source/LyraGame/GameFeatures/GameFeatureAction_AddGameplayCuePath.h/.cpp` |
| 分屏配置 | `Source/LyraGame/GameFeatures/GameFeatureAction_SplitscreenConfig.h/.cpp` |
| Experience 定义 | `Source/LyraGame/GameModes/LyraExperienceDefinition.h` |
| Experience 管理器 | `Source/LyraGame/GameModes/LyraExperienceManagerComponent.h/.cpp` |
| World 设置 | `Source/LyraGame/GameModes/LyraWorldSettings.h` |
| GF 插件: 射击核心 | `Plugins/GameFeatures/ShooterCore/` |
| GF 插件: FPS 自定义 | `Plugins/GameFeatures/LyraFPSGame/` |
| GF 插件: 关卡 | `Plugins/GameFeatures/ShooterMaps/` |
| GF 插件: 测试 | `Plugins/GameFeatures/ShooterTests/` |
| GF 插件: 俯视角 | `Plugins/GameFeatures/TopDownArena/` |
| GF 插件: 冒险扩展 | `Plugins/GameFeatures/ShooterExplorer/` |
| 项目配置 | `Config/DefaultEngine.ini` (GlobalDefaultGameMode) |
