// P

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_Attack.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UBTTask_Attack : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

private:
	/*
	 * 行为树任务的核心执行函数，用于定义具体的任务逻辑。
	 * OwnerComp: 拥有这个行为树任务的行为树组件
	 * NodeMemory: 用于存储和访问节点特定的数据
	 * return: EBTNodeResult::Type 表示任务执行后的结果类型
	 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
};
