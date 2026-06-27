// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/LyraFPSWeaponBase.h"


// Sets default values
ALyraFPSWeaponBase::ALyraFPSWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SkeletalMesh=CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh");
	SetRootComponent(SkeletalMesh);
	SkeletalMesh->FirstPersonPrimitiveType=EFirstPersonPrimitiveType::WorldSpaceRepresentation;
}

// Called when the game starts or when spawned
void ALyraFPSWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void ALyraFPSWeaponBase::Fire_Implementation(const TArray<FVector>& ImpactPositions,
	const TArray<FVector>& ImpactNormals, const TArray<TEnumAsByte<EPhysicalSurface>>& ImpactSurfaceTypes)
{
	
}


// Called every frame
void ALyraFPSWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALyraFPSWeaponBase::TriggerFireAudio_Implementation(USoundBase* Sound, AActor* Actor)
{
	
}

void ALyraFPSWeaponBase::OnRep_Owner()
{
	Super::OnRep_Owner();
	for (auto tmpPrim: K2_GetComponentsByClass(UPrimitiveComponent::StaticClass()))
	{
		tmpPrim->MarkRenderStateDirty();
	}
}

