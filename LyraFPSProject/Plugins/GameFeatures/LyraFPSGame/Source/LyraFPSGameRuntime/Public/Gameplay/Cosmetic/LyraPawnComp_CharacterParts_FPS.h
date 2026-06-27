// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cosmetics/LyraPawnComponent_CharacterParts.h"
#include "Gameplay/Cosmetic/CharacterPartData_FPS.h"

#include "LyraPawnComp_CharacterParts_FPS.generated.h"


/** 与FLyraAppliedCharacterPartEntry完全相同 */
USTRUCT()
struct FLyraAppliedCharacterPartEntry_FPS 
{
	GENERATED_BODY()

	FLyraAppliedCharacterPartEntry_FPS()
	{}

	FString GetDebugString() const;

	bool operator==(int32 inHandle) const{ return PartHandle == inHandle; }
	bool operator==(FLyraAppliedCharacterPartEntry_FPS& B) const{ return PartHandle == B.PartHandle; }
	// bool operator!=(FLyraAppliedCharacterPartEntry& B) const{ return PartHandle == B.PartHandle; }
public:
	// The character part being represented
	UPROPERTY()
	FLyraCharacterPart_FPS Part;

	// Handle index we returned to the user (server only)
	UPROPERTY()
	int32 PartHandle = INDEX_NONE;

	// The spawned actor instance (client only)
	UPROPERTY()
	TObjectPtr<UChildActorComponent> SpawnedComponent = nullptr;
	

};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LYRAFPSGAMERUNTIME_API ULyraPawnComp_CharacterParts_FPS : public ULyraPawnComponent_CharacterParts
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULyraPawnComp_CharacterParts_FPS();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	//防止AI等远处的模型在编辑器播放时APose
	//触发条件：没有在角色视野内生成
	//暴力处理方案：修改mesh的tick方案为
	void WarmUpInitialAnimPose();
	/** FPS模型生成 */
#pragma region FPSPart
public:
	UFUNCTION(BlueprintCallable, Category=Cosmetics)
	int32 AddCharacterPart_FPS(const FLyraCharacterPart_FPS& NewPart);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Cosmetics)
	void RemoveCharacterPart_FPS(int32 Handle);
	
	bool SpawnActorForEntry_FPS(FLyraAppliedCharacterPartEntry_FPS& Entry);

	// 更改Attach对象
	virtual USceneComponent* GetSceneComponentToAttachTo(const FLyraCharacterPart& CompTag) const override;
	virtual USceneComponent* GetSceneComponentToAttachTo_FPS(const FLyraCharacterPart_FPS& CompTag) const ;
protected:
	//仅需要本地存储
	TArray< FLyraAppliedCharacterPartEntry_FPS> LocalParts;
	
	int32 PartHandleCounter=0;
#pragma endregion FPSPart
};
