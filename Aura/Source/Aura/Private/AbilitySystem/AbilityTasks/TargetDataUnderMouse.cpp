// P


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"

#include "AbilitySystemComponent.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);

	return MyObj;
}

void UTargetDataUnderMouse::Activate()
{
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if(bIsLocallyControlled)
	{
		// Client：发送数据
		SendMouseCursorData();
	}
	else
	{
		// Server：监听数据
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(
			GetAbilitySpecHandle(),
			GetActivationPredictionKey()
		).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
	}
}

void UTargetDataUnderMouse::SendMouseCursorData()
{
	// FScopedPredictionWindow作用域锁的对象（在构造时打开，在析构时关闭），用于管理预测窗口
	FScopedPredictionWindow ScopedPredictionWindow(AbilitySystemComponent.Get());

	// 通过玩家控制器获取鼠标下的命中结果
	const APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);

	// 建一个包含单个命中目标数据的句柄和数据对象
	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);

	// 将创建的目标数据句柄 DataHandle 发送到服务器
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey
	);

	// 检查这个AT是否有效然后广播数据
	if(ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle,
	FGameplayTag ActivationTag)
{
	
}
