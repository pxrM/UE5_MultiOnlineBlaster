// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Chaos/ChaosEngineInterface.h"
#include "LyraFPSWeaponBase.generated.h"

// class USkeletalMeshComponent;

UCLASS()
class LYRAFPSGAMERUNTIME_API ALyraFPSWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALyraFPSWeaponBase();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void Fire(const TArray<FVector>& ImpactPositions,const TArray<FVector>& ImpactNormals,const TArray<TEnumAsByte<EPhysicalSurface>>& ImpactSurfaceTypes);
	
	UFUNCTION(BlueprintNativeEvent,BlueprintCallable)
	void TriggerFireAudio(USoundBase* Sound,AActor* Actor);

protected:
	virtual void OnRep_Owner() override;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<USkeletalMeshComponent> SkeletalMesh;
};
