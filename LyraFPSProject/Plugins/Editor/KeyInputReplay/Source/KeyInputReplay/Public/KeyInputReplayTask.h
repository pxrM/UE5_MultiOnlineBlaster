// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "Tickable.h"
#include "KeyInputReplayTask.generated.h"

class UKeyInputReplayRecordDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKeyInputReplayTask_Completed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKeyInputReplayTask_Failed);

UCLASS()
class KEYINPUTREPLAY_API UKeyInputReplay_ReplayKeyInput : public UBlueprintAsyncActionBase, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UKeyInputReplay_ReplayKeyInput();

	//~ Begin UBlueprintAsyncActionBase Interface.
	virtual void Activate() override;
	//~ End UBlueprintAsyncActionBase Interface.

	//~ Begin FTickableObject Interface.
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UKeyInputReplay_ReplayKeyInput, STATGROUP_Tickables); }
	//~ End FTickableObject Interface.
	
public:
	UPROPERTY(BlueprintAssignable)
	FKeyInputReplayTask_Completed Completed;

	UPROPERTY(BlueprintAssignable)
	FKeyInputReplayTask_Failed Failed;

private:
	bool bPlaying = false;
	float Time = 0.f;
	int32 KeyDataIndex = 0;
	bool PrintScreen = true;

	UPROPERTY()
	TObjectPtr<UKeyInputReplayRecordDataAsset> RecordData;

public:
	UFUNCTION(Category = "KeyInputReplay", BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", AdvancedDisplay = "2"))
	static UKeyInputReplay_ReplayKeyInput* ReplayKeyInput(UObject* WorldContextObject, UKeyInputReplayRecordDataAsset* RecordDataAsset, bool PrintToScreen = true);

private:
	void Deactivate(bool bCompleted);
};