// P

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_FindNearestPlayer.generated.h"

/**
 * 获取最近的玩家目标的服务
 */
UCLASS()
class AURA_API UBTService_FindNearestPlayer : public UBTService_BlueprintBase
{
	GENERATED_BODY()

protected:
	/*
	 * 按照设置的频率持续运行
	 * OwnerComp：拥有这个行为树服务的行为树组件
	 * NodeMemory：一个指向节点内存的指针，用于存储和访问节点特定的数据
	 * DeltaSeconds：上次执行 TickNode 到现在的时间间隔。
	 * 执行时间间隔可以在蓝图的 Interval 中设置， Random Deviation 表示随机偏差，作用在Interval设置的时间上
	 *	 eg：Interval = 0.5， Random Deviation = 0.1，则结果是0.5正负0.1
	 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};
