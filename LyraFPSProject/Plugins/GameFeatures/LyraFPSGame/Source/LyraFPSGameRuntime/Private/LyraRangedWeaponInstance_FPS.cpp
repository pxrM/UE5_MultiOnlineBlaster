// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraRangedWeaponInstance_FPS.h"

#include "LyraFPSFunctionLibrary.h"
#include "Equipment/LyraEquipmentDefinition_FPS.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "Equipment/LyraEquipmentManagerComponent_FPS.h"
#include "Gameplay/LyraFPSCameraComponent.h"
#include "Gameplay/Cosmetic/SkeletalMeshComponent_FPS.h"
#include "ProcedualAnim/FPSProceduralAnimComp.h"


ULyraRangedWeaponInstance_FPS::ULyraRangedWeaponInstance_FPS(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InitOwnerCameraComp();
}

void ULyraRangedWeaponInstance_FPS::PickBestAnimLayer_FPS(bool bEquipped, const FGameplayTagContainer& CosmeticTags,
                                                          TSubclassOf<UAnimInstance>& OutAnimLayer, TSubclassOf<UAnimInstance>& OutFPSAnimLayer) const
{
	const FLyraAnimLayerSelectionSet& SetToQuery_FullBody = (bEquipped ? EquippedAnimSet : UneuippedAnimSet);
	const FLyraAnimLayerSelectionSet& SetToQuery_FPS = (bEquipped ? EquippedAnimSet_FPS : UnequippedAnimSet_FPS);
	OutAnimLayer=SetToQuery_FullBody.SelectBestLayer(CosmeticTags);
	OutFPSAnimLayer=SetToQuery_FPS.SelectBestLayer(CosmeticTags);
}





void ULyraRangedWeaponInstance_FPS::InitOwnerCameraComp()
{
	if (auto ownerPawn= Cast<APawn>(GetOuter()) )
	{
		if (ownerPawn->HasAuthority())
		{
			OwnerCameraComp= ownerPawn->GetComponentByClass<ULyraFPSCameraComponent>();
		}
	}
}

USceneComponent* ULyraRangedWeaponInstance_FPS::GetAttachTarget_FPS(AActor* NewActor,bool bAllowNull) const
{
	//绑定到FPS模型
	if (auto foundTarget = ULyraFPSFunctionLibrary::GetFPSMesh(GetPawn()))
	{
		return Cast<USceneComponent>(foundTarget);
	}
	return bAllowNull? nullptr: Super::GetAttachTarget(NewActor);
}


void ULyraRangedWeaponInstance_FPS::SpawnEquipmentActors_FPS(const TArray<FLyraEquipmentActorToSpawn>& ActorsToSpawn)
{
	if (APawn* OwningPawn = GetPawn())
	{
		if (OwningPawn->IsPlayerControlled())
		{
			for (const FLyraEquipmentActorToSpawn& SpawnInfo : ActorsToSpawn)
			{
				if (auto attachTarget= GetAttachTarget_FPS(nullptr, true))
				{
					AActor* NewActor = GetWorld()->SpawnActorDeferred<AActor>(SpawnInfo.ActorToSpawn, FTransform::Identity, OwningPawn);
					NewActor->FinishSpawning(FTransform::Identity, /*bIsDefaultTransform=*/ true);
					NewActor->SetActorRelativeTransform(SpawnInfo.AttachTransform);
					NewActor->AttachToComponent(attachTarget, FAttachmentTransformRules::KeepRelativeTransform, SpawnInfo.AttachSocket);
					SpawnedActors_Local.Add(NewActor);
				}
			}
		}
		
	}
}

void ULyraRangedWeaponInstance_FPS::DestroyEquipmentActors_FPS()
{
	for (auto tmpActor:SpawnedActors_Local)
	{
		if (tmpActor)
		{
			tmpActor->Destroy();
		}
	}
}


void ULyraRangedWeaponInstance_FPS::OnEquipped()
{
	Super::OnEquipped();
	if (OwnerCameraComp)
	{
		OwnerCameraComp->AddCameraShakeRatio("WeaponInstance", headCameraShadeRatio);
	}
	if (auto ownerPawn= Cast<APawn>(GetOuter()) )
	{
		if (ownerPawn->IsLocallyControlled())
		{
			//应用FPS程序化动画
			if (UFPSProceduralAnimComp* FPSAnimComp= ownerPawn->GetComponentByClass<UFPSProceduralAnimComp>())
			{
				TMap<FGameplayTag, UDataAsset_ProceduralAnim*> animAssets;
				for (auto tmpAsset:FPSProceduralAnimDatas)
				{
					animAssets.Add(tmpAsset);
				}
				FPSAnimComp->UpdateData(animAssets);
			}
			//生成本地武器
			if (auto equipmentManager= ownerPawn->FindComponentByClass<ULyraEquipmentManagerComponent_FPS>())
			{
				if (auto equipDefinition= equipmentManager->GetDefinitionByInstance(this))
				{
					SpawnEquipmentActors_FPS(equipDefinition->ActorsToSpawn_Local);
				}
			}
		}
		
	}
}
void ULyraRangedWeaponInstance_FPS::OnUnequipped()
{
	Super::OnUnequipped();
	if (OwnerCameraComp)
	{
		OwnerCameraComp->RemoveCameraShakeRatio("WeaponInstance");
	}
	
	DestroyEquipmentActors_FPS();
}
