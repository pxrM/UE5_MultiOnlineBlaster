// P


#include "AI/BTTask_Attack.h"

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 这里不实现具体功能，因为使用内置的蓝图函数在c++中无法有很高的性能提升，如果有一些复杂的计算，可以在这实现。
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
