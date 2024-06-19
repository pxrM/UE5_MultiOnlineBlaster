// P

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"


/*
 * 获取释放能力时鼠标点击的目标位置数据后的多播委托
 * FGameplayAbilityTargetDataHandle：持有FGameplayAbilityTargetData的句柄
 *	  FGameplayAbilityTargetData：
 *		 用于通过网络传输定位数据的通用结构体,一般包括AAcotr/UObject的引用、FHitResult、和其他通用的Location/Direciton/OriginData。
 *		 本质上可以继承它以增添想要的任何数据, 其可以简单理解为在客户端和服务端的GameplayAbility中传递数据
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignture, const FGameplayAbilityTargetDataHandle&, DataHandle);


/**
 * UAbilityTask是用于实现自定义异步任务的基类，通常用于游戏能力系统（Gameplay Ability System，GAS）中。
 * 它允许开发者创建能够在游戏能力执行期间执行异步操作的任务，例如等待事件、执行定时操作等。
 *
 * UTargetDataUnderMouse：获取释放能力时鼠标点击的目标位置
 */
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()

public:
	/*
	 * OwningAbility：拥有此任务的能力。
	 * 
	 *	 BlueprintCallable: 这个标记表示该函数可以从蓝图中调用。
	 *	 Category="Ability|Tasks": 这个标记将函数分类到蓝图编辑器的特定菜单中。
	 *	 meta=(...): 这是一个元数据标记，包含额外的信息，帮助 Unreal Engine 编辑器和运行时系统处理函数。具体地：
	 *		DisplayName="TargetDataUnderMouse": 在蓝图编辑器中显示的函数名称。
	 *		HidePin="OwningAbility": 这个标记将隐藏在蓝图节点中的 OwningAbility 参数针脚，使其仅在内部使用。
	 *		DefaultToSelf="OwningAbility": 这个标记指示在调用时，默认使用调用者自身作为 OwningAbility 参数。
	 *		BlueprintInternalUseOnly：指示该函数仅限在UE蓝图编辑器中的蓝图内部使用。
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(DisplayName="TargetDataUnderMouse", HidePin="OwningAbility", DefaultToSelf="OwningAbility", BlueprintInternalUseOnly))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);

	/*
	 * 释放能力时鼠标点击的目标位置数据委托事件
	 *		BlueprintAssignable：用于声明可以在蓝图中绑定事件的属性。
	 */
	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignture ValidData;


private:
	// 激活调用这个task
	virtual void Activate() override;
	// 在客户端时将数据同步给Server
	void SendMouseCursorData();
	// Server收到TargetData后的监听委托
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
