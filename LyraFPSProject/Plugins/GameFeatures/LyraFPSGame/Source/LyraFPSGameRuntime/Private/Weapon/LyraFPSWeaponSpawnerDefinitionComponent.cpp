// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapon/LyraFPSWeaponSpawnerDefinitionComponent.h"
#include "EngineUtils.h"
#include "Equipment/LyraPickupDefinition.h"
#include "GameFramework/GameStateBase.h"
#include "Weapons/LyraWeaponSpawner.h"

void ULyraFPSWeaponSpawnerDefinitionComponent::BeginPlay()
{
	Super::BeginPlay();

	ApplyWeaponDefinitionOverrides();
}

void ULyraFPSWeaponSpawnerDefinitionComponent::ApplyWeaponDefinitionOverrides()
{
	const AGameStateBase* GameState = GetGameState<AGameStateBase>();
	if (!GameState || !GameState->HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ALyraWeaponSpawner> It(World); It; ++It)
	{
		ALyraWeaponSpawner* Spawner = *It;
		if (!Spawner)
		{
			continue;
		}

		ULyraWeaponPickupDefinition* CurrentDefinition = Spawner->WeaponDefinition;
		for (const FLyraFPSWeaponSpawnerDefinitionOverride& Override : DefinitionOverrides)
		{
			if (Override.FromDefinition && Override.ToDefinition && CurrentDefinition == Override.FromDefinition)
			{
				SetWeaponDefinition(Spawner,Override.ToDefinition);
				break;
			}
		}
	}
}

void ULyraFPSWeaponSpawnerDefinitionComponent::SetWeaponDefinition(ALyraWeaponSpawner* Spawner, ULyraWeaponPickupDefinition* InWeaponDefinition)
{
	Spawner->WeaponDefinition = InWeaponDefinition;
	if (Spawner->WeaponDefinition)
	{
		Spawner->CoolDownTime = Spawner->WeaponDefinition->SpawnCoolDownSeconds;
	}

	RefreshWeaponMesh(Spawner);
}

void ULyraFPSWeaponSpawnerDefinitionComponent::RefreshWeaponMesh(ALyraWeaponSpawner* Spawner)
{
	if (Spawner->WeaponDefinition != nullptr && Spawner->WeaponDefinition->DisplayMesh != nullptr)
	{
		Spawner->WeaponMesh->SetStaticMesh(Spawner->WeaponDefinition->DisplayMesh);
		Spawner->WeaponMesh->SetRelativeLocation(Spawner->WeaponDefinition->WeaponMeshOffset);
		Spawner->WeaponMesh->SetRelativeScale3D(Spawner->WeaponDefinition->WeaponMeshScale);
	}
}
