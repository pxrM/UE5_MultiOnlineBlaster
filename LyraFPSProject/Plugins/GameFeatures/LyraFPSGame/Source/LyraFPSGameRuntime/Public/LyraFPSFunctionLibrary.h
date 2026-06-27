// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "LyraFPSFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class LYRAFPSGAMERUNTIME_API ULyraFPSFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** 获取FPS模型：含FPS Tag的SkeletalMesh */
	UFUNCTION(Blueprintpure, Category = "LyraFPS",meta=(BlueprintThreadSafe))
	static class USkeletalMeshComponent_FPS* GetFPSMesh(AActor* InActor) ;

	/** 寻找武器Mesh
	 * InActor: AvatarActor，一般为Pawn
	 * Tag: 特殊标记，如FPS
	 */
	UFUNCTION(Blueprintpure, Category = "LyraFPS",meta=(BlueprintThreadSafe))
	static TArray<class ALyraFPSWeaponBase*> GetWeaponActors(AActor* InActor,bool bIsLocal=false);

	UFUNCTION(Blueprintpure, Category = "LyraFPS",meta=(BlueprintThreadSafe))
	static TArray<class USkeletalMeshComponent*> GetWeaponMeshes(AActor* InActor, bool bIsLocal=false);
};
