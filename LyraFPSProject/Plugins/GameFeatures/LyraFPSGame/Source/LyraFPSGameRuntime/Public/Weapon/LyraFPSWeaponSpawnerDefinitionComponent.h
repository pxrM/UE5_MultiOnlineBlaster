// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/GameStateComponent.h"
#include "LyraFPSWeaponSpawnerDefinitionComponent.generated.h"

class ULyraWeaponPickupDefinition;
class ALyraWeaponSpawner;

USTRUCT(BlueprintType)
struct FLyraFPSWeaponSpawnerDefinitionOverride
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="WeaponSpawner")
	TObjectPtr<ULyraWeaponPickupDefinition> FromDefinition;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="WeaponSpawner")
	TObjectPtr<ULyraWeaponPickupDefinition> ToDefinition;
};

/**
 * 将Lyra原生地图Spawner的TPS武器换成FPS的武器Definition 
 */
UCLASS(Blueprintable, meta=(BlueprintSpawnableComponent))
class LYRAFPSGAMERUNTIME_API ULyraFPSWeaponSpawnerDefinitionComponent : public UGameStateComponent
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category="WeaponSpawner")
	void ApplyWeaponDefinitionOverrides();

	void SetWeaponDefinition(ALyraWeaponSpawner* Spawner,ULyraWeaponPickupDefinition* InWeaponDefinition);
	void RefreshWeaponMesh(ALyraWeaponSpawner* Spawner);
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="WeaponSpawner")
	TArray<FLyraFPSWeaponSpawnerDefinitionOverride> DefinitionOverrides;
};
