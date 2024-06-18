// P


#include "Actor/AuraProjectileActor.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AAuraProjectileActor::AAuraProjectileActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	SphereCmp = CreateDefaultSubobject<USphereComponent>("SphereCmp");
	SetRootComponent(SphereCmp);
	SphereCmp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCmp->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereCmp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	SphereCmp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	SphereCmp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovementCmp = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovementCmp");
	ProjectileMovementCmp->InitialSpeed = 550.f;
	ProjectileMovementCmp->MaxSpeed = 550.f;
	ProjectileMovementCmp->ProjectileGravityScale = 0.f;
}

void AAuraProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	SphereCmp->OnComponentBeginOverlap.AddDynamic(this, &AAuraProjectileActor::OnSphereOverlap);
}

void AAuraProjectileActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
}


