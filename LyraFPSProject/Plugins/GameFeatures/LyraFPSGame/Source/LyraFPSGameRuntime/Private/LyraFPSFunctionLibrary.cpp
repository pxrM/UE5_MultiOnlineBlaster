// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraFPSFunctionLibrary.h"

#include "LyraRangedWeaponInstance_FPS.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "Gameplay/Cosmetic/SkeletalMeshComponent_FPS.h"
#include "Weapon/LyraFPSWeaponBase.h"
#include "Weapons/LyraWeaponInstance.h"

USkeletalMeshComponent_FPS* ULyraFPSFunctionLibrary::GetFPSMesh(AActor* InActor) 
{
	return InActor->GetComponentByClass<USkeletalMeshComponent_FPS>();
}

TArray<ALyraFPSWeaponBase*> ULyraFPSFunctionLibrary::GetWeaponActors(AActor* InActor,bool bIsLocal) 
{
	TArray<ALyraFPSWeaponBase*> outArray;
	auto EquipmentManager= InActor->FindComponentByClass<ULyraEquipmentManagerComponent>();
	if (EquipmentManager)
	{
		TArray<ULyraEquipmentInstance*> EquipmentInstances= EquipmentManager->GetEquipmentInstancesOfType(ULyraRangedWeaponInstance_FPS::StaticClass());
		if (!EquipmentInstances.IsEmpty())
		{
			if (bIsLocal)
			{
				//ViewModel
				if (auto fpsWeaponIns=Cast<ULyraRangedWeaponInstance_FPS>(EquipmentInstances[0]))
				{
					for (auto spawnedActor_Loc : fpsWeaponIns->GetSpawnedActors_Local())
					{
						if (auto spawnedWeapon= Cast<ALyraFPSWeaponBase>(spawnedActor_Loc))
						{
							outArray.Add(spawnedWeapon);
						}
					}
				}
			}
			else
			{
				for (auto SpawnedActor : EquipmentInstances[0]->GetSpawnedActors())
				{
					if (auto spawnedWeapon= Cast<ALyraFPSWeaponBase>(SpawnedActor))
					{
						outArray.Add(spawnedWeapon);
					}
				}
			}
		}
	}
	return outArray;
}


TArray< USkeletalMeshComponent*> ULyraFPSFunctionLibrary::GetWeaponMeshes(AActor* InActor, bool bIsLocal)
{
	TArray< USkeletalMeshComponent*> outArray;
	if (InActor==nullptr) return outArray;
	auto EquipmentManager= InActor->FindComponentByClass<ULyraEquipmentManagerComponent>();
	if (EquipmentManager)
	{
		TArray<ULyraEquipmentInstance*> EquipmentInstances= EquipmentManager->GetEquipmentInstancesOfType(ULyraWeaponInstance::StaticClass());
		if (!EquipmentInstances.IsEmpty())
		{
			if (bIsLocal)
			{
				//ViewModel
				if (auto fpsWeaponIns=Cast<ULyraRangedWeaponInstance_FPS>(EquipmentInstances[0]))
				{
					for (auto spawnedActor_Loc : fpsWeaponIns->GetSpawnedActors_Local())
					{
						if (auto meshComp= spawnedActor_Loc->FindComponentByClass<USkeletalMeshComponent>())
						{
							outArray.Add(meshComp);
						}
					}
				}
			}
			else
			{
				for (auto spawnedActor : EquipmentInstances[0]->GetSpawnedActors())
				{
					if (auto meshComp= spawnedActor->FindComponentByClass<USkeletalMeshComponent>())
					{
						outArray.Add(meshComp);
					}
				}
			}
		}
	}
	return outArray;
}
