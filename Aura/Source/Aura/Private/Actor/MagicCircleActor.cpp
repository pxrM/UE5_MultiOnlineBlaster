// P


#include "Actor/MagicCircleActor.h"

#include "Components/DecalComponent.h"

// Sets default values
AMagicCircleActor::AMagicCircleActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	MagicCircleDecalCmp = CreateDefaultSubobject<UDecalComponent>("MagicCircleDecal");
	MagicCircleDecalCmp->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AMagicCircleActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMagicCircleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

