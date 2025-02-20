// P


#include "Actor/AuraEnemySpawnVolume.h"

#include "Actor/AuraEnemySpawnPoint.h"
#include "Components/BoxComponent.h"
#include "Interaction/PlayerInterface.h"

AAuraEnemySpawnVolume::AAuraEnemySpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	SetRootComponent(Box);
	// 仅用于空间查询（射线投射、扫描和重叠）。不能用于模拟（刚体、约束）。对于角色移动和不需要物理模拟的事物很有用。
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// 设置为静态对象不会参与物理模拟（如重力、碰撞响应等），有助于减少计算量，提高性能。
	Box->SetCollisionObjectType(ECC_WorldStatic);
	// 先设置对所有碰撞通道进行忽略
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	// 再设置对 Pawn 通道进行重叠检测
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AAuraEnemySpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	Box->OnComponentBeginOverlap.AddDynamic(this, &AAuraEnemySpawnVolume::OnBoxOverlap);
}

void AAuraEnemySpawnVolume::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult& SweepResult)
{
	if (!OtherActor->Implements<UPlayerInterface>()) return;

	bReached = true;

	// 在设置的所有点位生成敌人
	for (AAuraEnemySpawnPoint* Point : SpawnPoints)
	{
		if (IsValid(Point))
		{
			Point->SpawnEnemy();
		}
	}

	// 设置关闭物理查询，直接销毁无法保存到存档
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AAuraEnemySpawnVolume::LoadActor_Implementation()
{
	if (bReached)
	{
		Destroy();
	}
}
