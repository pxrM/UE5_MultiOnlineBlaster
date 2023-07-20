// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	APickupSpawnPoint();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


protected:
	void SpawnPickup();
	void SpawnPickupTimerFinished();
	UFUNCTION()
		void StartSpawnPickupTimer(AActor* DestroyedActor);

protected:
	UPROPERTY(EditAnywhere)
		TArray<TSubclassOf<class APickup>> PickupClasses;
	UPROPERTY()
		APickup* SpawnedPickup;

private:
	FTimerHandle SpawnPickupTimer;
	UPROPERTY(EditAnywhere)
		float SpawnPickupTimeMin;
	UPROPERTY(EditAnywhere)
		float SpawnPickupTimeMax;

};
