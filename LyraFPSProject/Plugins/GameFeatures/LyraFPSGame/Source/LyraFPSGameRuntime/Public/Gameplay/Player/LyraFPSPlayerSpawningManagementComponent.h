// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Player/LyraPlayerSpawningManagerComponent.h"

#include "LyraFPSPlayerSpawningManagementComponent.generated.h"

class AActor;
class AController;
class ALyraPlayerStart;
class UObject;

USTRUCT()
struct FPlayerStartCount
{
	GENERATED_BODY()
public:
	TSoftObjectPtr<ALyraPlayerStart> PlayerStart;
	int32 Count;
	
	FPlayerStartCount(ALyraPlayerStart* NewPlayerStart=nullptr)
		:Count(1)
	{
		PlayerStart=NewPlayerStart;
	}
	
	bool operator==(const FPlayerStartCount& B) const
	{
		return B.PlayerStart==PlayerStart;
	}
	bool operator==(ALyraPlayerStart* B) const
	{
		return B==PlayerStart;
	}
	void operator++()
	{
		++Count;
	}
};

/**
 * 
 */
UCLASS()
class ULyraFPSPlayerSpawningManagementComponent : public ULyraPlayerSpawningManagerComponent
{
	GENERATED_BODY()

public:

	ULyraFPSPlayerSpawningManagementComponent(const FObjectInitializer& ObjectInitializer);

	virtual AActor* OnChoosePlayerStart(AController* Player, TArray<ALyraPlayerStart*>& PlayerStarts) override;
	virtual void OnFinishRestartPlayer(AController* Player, const FRotator& StartRotation) override;

protected:
	TArray<FPlayerStartCount> PlayerStartCount;
};
