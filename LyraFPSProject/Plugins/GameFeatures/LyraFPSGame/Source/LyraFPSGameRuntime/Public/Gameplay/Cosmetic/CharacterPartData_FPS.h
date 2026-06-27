// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Cosmetics/LyraCharacterPartTypes.h"
#include "Cosmetics/LyraControllerComponent_CharacterParts.h"
#include "CharacterPartData_FPS.generated.h"

USTRUCT(BlueprintType)
struct FLyraCharacterPart_FPS:public FLyraCharacterPart
{
	GENERATED_BODY()

	/** 是否仅在本地生成  */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsLocally=false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<USceneComponent> CustomAttachComp;
	// The part to spawn
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// TSubclassOf<AActor> PartClass;
	//
	// // The socket to attach the part to (if any)
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// FName SocketName;
	//
	// // How to handle collision for the primitive components in the part
	// UPROPERTY(EditAnywhere, BlueprintReadWrite)
	// ECharacterCustomizationCollisionMode CollisionMode = ECharacterCustomizationCollisionMode::NoCollision;
	//
	// // Compares against another part, ignoring the collision mode
	// static bool AreEquivalentParts(const FLyraCharacterPart& A, const FLyraCharacterPart& B)
	// {
	// 	return (A.PartClass == B.PartClass) && (A.SocketName == B.SocketName);
	// }
	
	FLyraCharacterPart ToPart() const
	{
		FLyraCharacterPart Part;
		Part.PartClass = PartClass;
		Part.SocketName = SocketName;
		Part.CollisionMode = CollisionMode;
		return Part;
	}
};

// A character part requested on a controller component
USTRUCT()
struct FLyraControllerCharacterPartEntry_FPS
{
	GENERATED_BODY()

	FLyraControllerCharacterPartEntry_FPS()
	{}
public:
	// The character part being represented
	UPROPERTY(EditAnywhere, meta=(ShowOnlyInnerProperties))
	FLyraCharacterPart_FPS Part;

	// The handle if already applied to a pawn
	int32 Handle=INDEX_NONE;

	// The source of this part
	ECharacterPartSource Source = ECharacterPartSource::Natural;
};