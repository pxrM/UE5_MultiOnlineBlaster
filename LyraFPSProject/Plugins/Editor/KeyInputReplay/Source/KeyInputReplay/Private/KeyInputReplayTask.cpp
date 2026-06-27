// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#include "KeyInputReplayTask.h"
#include "KeyInputReplayRecordDataAsset.h"

#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

UKeyInputReplay_ReplayKeyInput::UKeyInputReplay_ReplayKeyInput()
{
}

UKeyInputReplay_ReplayKeyInput* UKeyInputReplay_ReplayKeyInput::ReplayKeyInput(UObject* WorldContextObject, UKeyInputReplayRecordDataAsset* RecordDataAsset, bool PrintToScreen)
{
	UKeyInputReplay_ReplayKeyInput* Action = NewObject<UKeyInputReplay_ReplayKeyInput>();
	Action->RegisterWithGameInstance(WorldContextObject);
	Action->RecordData = RecordDataAsset;
	Action->PrintScreen = PrintToScreen;

	return Action;
}

void UKeyInputReplay_ReplayKeyInput::Activate()
{
	bPlaying = true;
}

void UKeyInputReplay_ReplayKeyInput::Tick(float DeltaTime)
{
	if (bPlaying)
	{
		if (IsValid(RecordData) == false)
		{
			Deactivate(false);
			return;
		}

		Time += DeltaTime;
		if (PrintScreen)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ReplayKeyInput: %.2fsec"), Time), true, false, FColor::Green, 0.f);
		}

		// Key pressed or released
		while (true)
		{
			if (KeyDataIndex >= RecordData->KeyData.Num())
			{
				Deactivate(true);
				break;
			}

			FKeyInputReplayKeyData* keyData = &RecordData->KeyData[KeyDataIndex];
			if (Time < keyData->Time)
			{
				break;
			}

			switch (keyData->InputType)
            {
            case EKeyInputReplayInputType::KeyDown:                FSlateApplication::Get().ProcessKeyDownEvent(keyData->InputEvents.GetKeyEvent()); break;
            case EKeyInputReplayInputType::KeyUp:                  FSlateApplication::Get().ProcessKeyUpEvent(keyData->InputEvents.GetKeyEvent()); break;
            case EKeyInputReplayInputType::AnalogInput:            FSlateApplication::Get().ProcessAnalogInputEvent(keyData->InputEvents.GetAnalogInputEvent()); break;
            case EKeyInputReplayInputType::MouseMove:              FSlateApplication::Get().ProcessMouseMoveEvent(keyData->InputEvents.GetMouseMoveEvent()); break;
            case EKeyInputReplayInputType::MouseButtonUp:          FSlateApplication::Get().ProcessMouseButtonUpEvent(keyData->InputEvents.GetMouseButtonUpEvent()); break;
            case EKeyInputReplayInputType::MouseWheelOrGesture:    FSlateApplication::Get().ProcessMouseWheelOrGestureEvent(keyData->InputEvents.GetMouseWheelOrGestureEvent(), nullptr); break;
            case EKeyInputReplayInputType::MotionDetected:         FSlateApplication::Get().ProcessMotionDetectedEvent(keyData->InputEvents.GetMotionEvent()); break;
            case EKeyInputReplayInputType::MouseButtonDoubleClick:
            	{
            		TSharedPtr<FGenericWindow> GenWindow;
            		FSlateApplication::Get().ProcessMouseButtonDownEvent(GenWindow, keyData->InputEvents.GetMouseButtonDownEvent());
            	} break;
            case EKeyInputReplayInputType::MouseButtonDown:
            	{
            		TSharedPtr<FGenericWindow> GenWindow;
            		FSlateApplication::Get().ProcessMouseButtonDownEvent(GenWindow, keyData->InputEvents.GetMouseButtonDownEvent());
            	} break;
            }
            			
			KeyDataIndex++;
		}
	}
}

void UKeyInputReplay_ReplayKeyInput::Deactivate(bool bCompleted)
{
	if (PrintScreen)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ReplayKeyInput: Finished!")), true, false, FColor::Green, 1.f);
	}

	SetReadyToDestroy();
	bPlaying = false;
	RecordData = nullptr;

	if (bCompleted)
	{
		Completed.Broadcast();
	}
	else
	{
		Failed.Broadcast();
	}
}