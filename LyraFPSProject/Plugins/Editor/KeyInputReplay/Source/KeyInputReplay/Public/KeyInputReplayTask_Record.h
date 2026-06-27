// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "KeyInputRecord.h"
#include "KeyInputReplayDataTypes.h"
#include "KeyInputReplayRecordInputProcessor.h"
#include "KeyInputReplayTask.h"
#include "Tickable.h"
#include "KeyInputReplayTask_Record.generated.h"


class UKeyInputReplayRecordDataAsset;
class UKeyInputReplay_Record;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKeyInputReplayTask_Record_Start,UKeyInputReplay_Record*,Task);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKeyInputReplayTask_Record_Completed);


UCLASS(BlueprintType,Blueprintable)
class KEYINPUTREPLAY_API UKeyInputReplay_Record : public UBlueprintAsyncActionBase, public FTickableGameObject,public IKeyInputRecord
{
	GENERATED_BODY()

public:
	UKeyInputReplay_Record();

	UFUNCTION(Category = "KeyInputReplay", BlueprintCallable, meta = (WorldContext = "WorldContextObject", BlueprintInternalUseOnly = "true", AdvancedDisplay = "2"))
	static UKeyInputReplay_Record* RecordKeyInput(UObject* WorldContextObject, UKeyInputReplayRecordDataAsset* RecordDataAsset, bool PrintToScreen = true,bool bRecordMouseInput=true,bool bRecordKeyboardInput=true,bool bRecordGamepadInput=true);

	
	//~ Begin UBlueprintAsyncActionBase Interface.
	virtual void Activate() override;

	//~ Begin FTickableObject Interface.
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UKeyInputReplay_Record, STATGROUP_Tickables); }
	//~ End FTickableObject Interface.
	
	UFUNCTION(meta=(DisplayName ="StopRecord"),Category = "KeyInputReplay", BlueprintCallable)
	void StopingRecord();
	
	void StartRecord();
	void StopRecord();
	void ResetRecord();
	bool IsRecordKey(const FKey& Key) const;
	// Input Events
	virtual void OnKeyDown(const FKeyEvent& KeyEvent) override;
	virtual void OnKeyUp(const FKeyEvent& KeyEvent) override;
	virtual void OnAnalogInput(const FAnalogInputEvent& AnalogInputEvent) override;
	virtual void OnMouseMove(const FPointerEvent& PointerEvent) override;
	virtual void OnMouseButtonDown(const FPointerEvent& PointerEvent) override;
	virtual void OnMouseButtonUp(const FPointerEvent& PointerEvent) override;
	virtual void OnMouseButtonDoubleClick(const FPointerEvent& PointerEvent) override;
	virtual void OnMouseWheelOrGesture(const FPointerEvent& WheelEvent) override;
	virtual void OnMotionDetected(const FMotionEvent& MotionEvent) override;
	
	
public:
	UPROPERTY(BlueprintAssignable)
	FKeyInputReplayTask_Record_Start Start;
	
	UPROPERTY(BlueprintAssignable)
	FKeyInputReplayTask_Record_Completed Completed;


private:
	bool bRecording = false;
	float Time = 0.f;
	TArray<FKeyInputReplayKeyData> KeyData;
	TSharedPtr<FKeyInputReplayRecordInputProcessor> InputProcessor;
	int32 KeyDataIndex = 0;
	bool PrintScreen = true;

	UPROPERTY()
	TObjectPtr<UKeyInputReplayRecordDataAsset> RecordData;

public:
	protected:
	UPROPERTY(Category = "Settings|Record Input", BlueprintReadOnly, EditInstanceOnly)
	bool bRecordMouseInput = true;

	UPROPERTY(Category = "Settings|Record Input", BlueprintReadOnly, EditInstanceOnly)
	bool bRecordKeyboardInput = true;
	
	UPROPERTY(Category = "Settings|Record Input", BlueprintReadOnly, EditInstanceOnly)
	bool bRecordGamepadInput = true;

};