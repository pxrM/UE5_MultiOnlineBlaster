// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "KeyInputRecord.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "KeyInputReplayDataTypes.h"
#include "KeyInputReplayRecordInputProcessor.h"
#include "KeyInputReplayRecorder.generated.h"

class UKeyInputReplayRecordDataAsset;

UCLASS()
class KEYINPUTREPLAY_API AKeyInputReplayRecorder : public AActor,public IKeyInputRecord
{
	GENERATED_BODY()
	
public:
	AKeyInputReplayRecorder();
	~AKeyInputReplayRecorder();

	//~ Begin AActor Interface.
	virtual void  Tick(float DeltaTime) override;
	//~ End AActor Interface.

public:
	UPROPERTY(Category = "Component", BlueprintReadOnly)
	TObjectPtr<UTextRenderComponent> TextRenderComponent;

	UPROPERTY(Category = "Settings", BlueprintReadOnly, EditInstanceOnly)
	TObjectPtr<UKeyInputReplayRecordDataAsset> RecordDataAsset;
	
	UPROPERTY(Category = "Settings", BlueprintReadOnly, EditInstanceOnly)
	bool bAutoRecord = true;

	UPROPERTY(Category = "Settings", BlueprintReadOnly, EditInstanceOnly, meta = (UIMin = 0, ClampMin = 0))
	int32 AutoRecordDelayFrame = 30;

	UPROPERTY(Category = "Settings", BlueprintReadOnly, EditInstanceOnly)
	bool PrintToScreen = true;

	UPROPERTY(Category = "Settings|Record Input", BlueprintReadOnly, EditInstanceOnly)
	bool bRecordMouseInput = true;

	UPROPERTY(Category = "Settings|Record Input", BlueprintReadOnly, EditInstanceOnly)
	bool bRecordKeyboardInput = true;
	
	UPROPERTY(Category = "Settings|Record Input", BlueprintReadOnly, EditInstanceOnly)
	bool bRecordGamepadInput = true;

	UPROPERTY(Category = "AutoRecord", BlueprintReadOnly, VisibleInstanceOnly)
	bool DoAutoRecord = false;

	UPROPERTY(Category = "AutoRecord", BlueprintReadOnly, VisibleInstanceOnly)
	float CurrentAutoRecordDelayFrame = 0;

	UPROPERTY(Category = "Record", BlueprintReadOnly, VisibleInstanceOnly)
	bool bRecording = false;

	UPROPERTY(Category = "Record", BlueprintReadOnly, VisibleInstanceOnly)
	float Time = 0.f;

	UPROPERTY(Category = "Record", BlueprintReadOnly, VisibleInstanceOnly)
	TArray<FKeyInputReplayKeyData> KeyData;

private:
	TSharedPtr<FKeyInputReplayRecordInputProcessor> InputProcessor;

public:
	UFUNCTION(Category = "KeyInputReplay", BlueprintCallable)
	void StartRecording();

	UFUNCTION(Category = "KeyInputReplay", BlueprintCallable)
	void StopRecording();

public:
	void Start();
	void Stop();

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

private:
	void Reset();
	bool IsRecordKey(const FKey& Key);
};