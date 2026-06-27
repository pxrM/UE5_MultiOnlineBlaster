// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#include "KeyInputReplayTask_Record.h"
#include "KeyInputReplayRecordDataAsset.h"
#include "KeyInputReplayRecorder.h"
#include "KeyInputReplaySettings.h"

#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#if WITH_EDITOR
#include "UObject/Package.h"
#include "FileHelpers.h"
#endif



UKeyInputReplay_Record::UKeyInputReplay_Record()
{

}

UKeyInputReplay_Record* UKeyInputReplay_Record::RecordKeyInput(UObject* WorldContextObject, UKeyInputReplayRecordDataAsset* RecordDataAsset, bool PrintToScreen,bool bRecordMouseInput,bool bRecordKeyboardInput,bool bRecordGamepadInput)
{
	UKeyInputReplay_Record* Action = NewObject<UKeyInputReplay_Record>();
	Action->RegisterWithGameInstance(WorldContextObject);
	Action->RecordData = RecordDataAsset;
	Action->PrintScreen = PrintToScreen;
	Action->bRecordMouseInput=bRecordMouseInput;
	Action->bRecordKeyboardInput=bRecordKeyboardInput;
	Action->bRecordGamepadInput=bRecordGamepadInput;
	
	return Action;
}

void UKeyInputReplay_Record::Activate()
{
	bRecording = true;
	InputProcessor = MakeShared<FKeyInputReplayRecordInputProcessor>();
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}
	Start.Broadcast(this);
	StartRecord();
}

void UKeyInputReplay_Record::Tick(float DeltaTime)
{
	// Recording
	if (bRecording)
	{
		Time += DeltaTime;
		if (PrintScreen)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ReCordKeyInput Task Recording: %.2fsec"), Time), true, false, FColor::Red, 0.f);
		}
	}
}

void UKeyInputReplay_Record::StopingRecord()
{
	StopRecord();
	if (PrintScreen)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("ReCordKeyInput Task: Finished!")), true, false, FColor::Red, 1.f);
	}
	// Unresister InputProcessor
	if (FSlateApplication::IsInitialized() && InputProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
	SetReadyToDestroy();
	
	bRecording = false;
	RecordData = nullptr;

	Completed.Broadcast();
}

void UKeyInputReplay_Record::StartRecord()
{
	if (IsValid(RecordData) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed start input recording. Should setup RecordDataAsset."));
		return;
	}

	// Settings InputProcessor
	const UKeyInputReplaySettings* Settings = GetDefault<UKeyInputReplaySettings>();
	InputProcessor->Recorder = this;
	InputProcessor->IgnoreKeys = Settings->IgnoreRecordKeys;

	// Start recording.
	ResetRecord();
	bRecording = true;
}

void UKeyInputReplay_Record::StopRecord()
{
	if (bRecording)
	{
		// Save data asset.
		if (IsValid(RecordData))
		{
			RecordData->Reset();

			RecordData->bRecorded = true;
			RecordData->KeyData = KeyData;

#if UE_EDITOR
			// Checkout and Save
			RecordData->MarkPackageDirty();

			TArray<UPackage*> PackagesToSave;
			PackagesToSave.Add(RecordData->GetOutermost());
			FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, true);
#endif
		}
		
		// Finalize recording.
		ResetRecord();
		bRecording = false;
	}
}

void UKeyInputReplay_Record::ResetRecord()
{
	Time = 0.f;
	KeyData.Empty();
}




void UKeyInputReplay_Record::OnKeyDown(const FKeyEvent& KeyEvent)
{
	if (bRecording)
	{
		if (IsRecordKey(KeyEvent.GetKey()))
		{
			FKeyInputReplayKeyData data;
			data.Time = Time;
			data.InputType = EKeyInputReplayInputType::KeyDown;
			(&data.InputEvents)->SetKeyEvent(KeyEvent);
			KeyData.Add(data);
		}
	}
}

void UKeyInputReplay_Record::OnKeyUp(const FKeyEvent& KeyEvent)
{
	if (bRecording)
	{
		if (IsRecordKey(KeyEvent.GetKey()))
		{
			FKeyInputReplayKeyData data;
			data.Time = Time;
			data.InputType = EKeyInputReplayInputType::KeyUp;
			(&data.InputEvents)->SetKeyEvent(KeyEvent);
			KeyData.Add(data);
		}
	}
}

void UKeyInputReplay_Record::OnAnalogInput(const FAnalogInputEvent& AnalogInputEvent)
{
	if (bRecording)
	{
		if (IsRecordKey(AnalogInputEvent.GetKey()))
		{
			FKeyInputReplayKeyData data;
			data.Time = Time;
			data.InputType = EKeyInputReplayInputType::AnalogInput;
			(&data.InputEvents)->SetAnalogInputEvent(AnalogInputEvent);
			KeyData.Add(data);
		}
	}
}

void UKeyInputReplay_Record::OnMouseMove(const FPointerEvent& PointerEvent)
{
	if (bRecording)
	{
		if (bRecordMouseInput)
		{
			FKeyInputReplayKeyData data;
			data.Time = Time;
			data.InputType = EKeyInputReplayInputType::MouseMove;
			(&data.InputEvents)->SetPointerEvent(PointerEvent);
			KeyData.Add(data);
		}
	}
}

void UKeyInputReplay_Record::OnMouseButtonDown(const FPointerEvent& PointerEvent)
{
	if (bRecording)
	{
		if (bRecordMouseInput)
		{
			FKeyInputReplayKeyData data;
			data.Time = Time;
			data.InputType = EKeyInputReplayInputType::MouseButtonDown;
			(&data.InputEvents)->SetPointerEvent(PointerEvent);
			KeyData.Add(data);
		}
	}
}

void UKeyInputReplay_Record::OnMouseButtonUp(const FPointerEvent& PointerEvent)
{
	if (bRecording)
	{
		if (bRecordMouseInput)
		{
			FKeyInputReplayKeyData data;
			data.Time = Time;
			data.InputType = EKeyInputReplayInputType::MouseButtonUp;
			(&data.InputEvents)->SetPointerEvent(PointerEvent);
			KeyData.Add(data);
		}
	}
}

void UKeyInputReplay_Record::OnMouseButtonDoubleClick(const FPointerEvent& PointerEvent)
{
	if (bRecording)
	{
		if (bRecordMouseInput)
		{
			FKeyInputReplayKeyData data;
			data.Time = Time;
			data.InputType = EKeyInputReplayInputType::MouseButtonDoubleClick;
			(&data.InputEvents)->SetPointerEvent(PointerEvent);
			KeyData.Add(data);
		}
	}
}

void UKeyInputReplay_Record::OnMouseWheelOrGesture(const FPointerEvent& WheelEvent)
{
	if (bRecording)
	{
		if (bRecordMouseInput)
		{
			FKeyInputReplayKeyData data;
			data.Time = Time;
			data.InputType = EKeyInputReplayInputType::MouseWheelOrGesture;
			(&data.InputEvents)->SetPointerEvent(WheelEvent);
			KeyData.Add(data);
		}
	}
}

void UKeyInputReplay_Record::OnMotionDetected(const FMotionEvent& MotionEvent)
{
	if (bRecording)
	{
		if (bRecordMouseInput)
		{
			FKeyInputReplayKeyData data;
			data.Time = Time;
			data.InputType = EKeyInputReplayInputType::MotionDetected;
			(&data.InputEvents)->SetMotionEvent(MotionEvent);
			KeyData.Add(data);
		}
	}
}

bool UKeyInputReplay_Record::IsRecordKey(const FKey& Key) const
{
	if (Key.IsValid())
	{
		if (Key.IsMouseButton())
		{
			return bRecordMouseInput;
		}
		else if (Key.IsGamepadKey())
		{
			return bRecordGamepadInput;
		}
		else
		{
			return bRecordKeyboardInput;
		}
	}

	return false;
}
