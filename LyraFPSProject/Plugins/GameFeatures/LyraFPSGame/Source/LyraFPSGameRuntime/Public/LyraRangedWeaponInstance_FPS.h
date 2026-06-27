// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/LyraRangedWeaponInstance.h"
#include "LyraRangedWeaponInstance_FPS.generated.h"

class UDataAsset_ProceduralAnim;
class UWeaponRotationSwayFragment;
/**
 * 每把武器有自己的默认AnimLayer
 * 由于可能适配多种角色/情况(如Lyra男女角色)，用Tag执行进一步筛选
 */
UCLASS()
class LYRAFPSGAMERUNTIME_API ULyraRangedWeaponInstance_FPS : public ULyraRangedWeaponInstance
{
	GENERATED_BODY()

public:
	ULyraRangedWeaponInstance_FPS(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	// Choose the best layer from EquippedAnimSet or UneuippedAnimSet based on the specified gameplay tags
	UFUNCTION(BlueprintCallable, Category=Animation)
	void PickBestAnimLayer_FPS(bool bEquipped, const FGameplayTagContainer& CosmeticTags,TSubclassOf<UAnimInstance>& OutAnimLayer,TSubclassOf<UAnimInstance>& OutFPSAnimLayer) const;
	
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;
protected:
	/** FPS动画集 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation|FPS")
	FLyraAnimLayerSelectionSet EquippedAnimSet_FPS;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation|FPS")
	FLyraAnimLayerSelectionSet UnequippedAnimSet_FPS;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation|FPS",Instanced)
	TArray<TObjectPtr<UWeaponRotationSwayFragment>> WeaponRotationSways;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation|FPS")
	int32 WeaponRotationSwayDefaultIndex=0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation|FPS",meta=(Categories="ProceduralAnim.Fragment.Type"))
	TMap<FGameplayTag,TObjectPtr<UDataAsset_ProceduralAnim>> FPSProceduralAnimDatas;
	
	/*** Camera 相机死否跟随骨骼摇晃 */
protected:
	void InitOwnerCameraComp();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "FPS|Camera")
	TObjectPtr<class ULyraFPSCameraComponent> OwnerCameraComp;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FPS|Camera")
	float headCameraShadeRatio=.7f;
	
	//武器生成和吸附
#pragma region Equipment
public:
	virtual void SpawnEquipmentActors_FPS(const TArray<FLyraEquipmentActorToSpawn>& ActorsToSpawn);
	virtual void DestroyEquipmentActors_FPS();

	/**
	 * 找吸附目标
	 * @param NewActor 占位，无用 
	 * @param bAllowNull 如果Component为空，允许返回null，否则返回父类的搜索结果，默认Mesh
	 * @return 
	 */
	virtual USceneComponent* GetAttachTarget_FPS(AActor* NewActor=nullptr,bool bAllowNull=false) const;
	TArray<TObjectPtr<AActor>> GetSpawnedActors_Local(){return SpawnedActors_Local;}
protected:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> SpawnedActors_Local;
#pragma endregion 
};
