// P


#include "AI/BTService_FindNearestPlayer.h"

#include "AIController.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, *AIOwner->GetName());
	// GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Green, *ActorOwner->GetName());

	const APawn* OwningPawn = AIOwner->GetPawn();
	
	// 根据角色设置设置的标签来获取角色敌人标签
	const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) ? FName("Enemy") : FName("Player");
	// 目标标签在场景的所有实例
	TArray<AActor*> ActorsWithTag;
	UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, ActorsWithTag);
	// 遍历Actors，比对它们和OwningPawn之间的距离，来获取最短距离的目标
	float ClosestDistance = TNumericLimits<float>::Max();	// 返回float类型能表示的最大可能值
	AActor* ClosestActor = nullptr;
	for(AActor* Actor : ActorsWithTag)
	{
		const float Distance = OwningPawn->GetDistanceTo(Actor);
		if(Distance < ClosestDistance)
		{
			ClosestDistance = Distance;	// 将当前的最短距离记录更新为这个新发现的、更短的距离。
			ClosestActor = Actor;	//  将指向最近Actor的指针更新为当前这个Actor。
		}
	}
	// 设置黑板数据
	UBTFunctionLibrary::SetBlackboardValueAsObject(this, TargetToFollowSelector, ClosestActor);
	UBTFunctionLibrary::SetBlackboardValueAsFloat(this, DistanceToTargetSelector, ClosestDistance);
}
