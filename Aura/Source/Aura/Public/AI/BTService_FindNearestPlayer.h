// P

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_FindNearestPlayer.generated.h"

/**
 * 获取最近的玩家目标的服务
 * 
 * UBTService_BlueprintBase 是行为树中服务节点的蓝图基类。服务节点是一种特殊的辅助节点，它附加在复合节点（如Selector、Sequence等）上，在行为树执行过程中提供后台服务功能。
 * 主要功能
 *	定时执行：服务节点可以按照设定的时间间隔周期性执行
 *	数据更新：用于更新黑板数据、检查条件、执行后台计算
 *	事件监听：可以监听游戏事件并做出响应
 *	状态监控：持续监控AI的状态变化
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
	 *	 eg：Interval = 0.5， Random Deviation = 0.1，则结果是0.5正负0.1，0.4~0.6
	 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	// 需要跟随攻击的目标
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FBlackboardKeySelector TargetToFollowSelector;
	// 和目标的距离
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FBlackboardKeySelector DistanceToTargetSelector;
};
