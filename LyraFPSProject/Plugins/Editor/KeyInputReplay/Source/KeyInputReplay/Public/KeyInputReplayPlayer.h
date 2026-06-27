// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "KeyInputReplayDataTypes.h"
#include "KeyInputReplayPlayer.generated.h"

class UKeyInputReplayRecordDataAsset;

UCLASS()
class KEYINPUTREPLAY_API AKeyInputReplayPlayer : public AActor
{
	GENERATED_BODY()
	
public:
	AKeyInputReplayPlayer();

	//~ Begin AActor Interface.
	virtual void  Tick(float DeltaTime) override;
	//~ End AActor Interface.

public:
	UPROPERTY(Category = "Component", BlueprintReadOnly)
	TObjectPtr<UTextRenderComponent> TextRenderComponent;

	UPROPERTY(Category = "Settings", BlueprintReadOnly, EditInstanceOnly)
	TArray<TObjectPtr<UKeyInputReplayRecordDataAsset>> RecordDataList;

	UPROPERTY(Category = "Settings", BlueprintReadOnly, EditInstanceOnly)
	bool bAutoPlay = true;

	UPROPERTY(Category = "Settings", BlueprintReadOnly, EditInstanceOnly, meta = (UIMin = 0, ClampMin = 0))
	int32 AutoPlayDelayFrame = 30;

	UPROPERTY(Category = "Settings", BlueprintReadOnly, EditInstanceOnly)
	bool PrintToScreen = true;

	UPROPERTY(Category = "AutoPlay", BlueprintReadOnly, VisibleInstanceOnly)
	float CurrentAutoPlayDelayFrame = 0;

	UPROPERTY(Category = "Playing", BlueprintReadOnly, VisibleInstanceOnly)
	bool bPlaying = false;

	UPROPERTY(Category = "Playing", BlueprintReadOnly, VisibleInstanceOnly)
	float Time = 0.f;

	UPROPERTY(Category = "Playing", BlueprintReadOnly, VisibleInstanceOnly)
	int32 RecordDataIndex = 0;

	UPROPERTY(Category = "Playing", BlueprintReadOnly, VisibleInstanceOnly)
	int32 KeyDataIndex = 0;

private:
	bool bNeverPlay = true;

public:
	UFUNCTION(Category = "KeyInputReplay", BlueprintCallable)
	void Play();

	UFUNCTION(Category = "KeyInputReplay", BlueprintCallable)
	void Stop();

private:
	void Reset();
};