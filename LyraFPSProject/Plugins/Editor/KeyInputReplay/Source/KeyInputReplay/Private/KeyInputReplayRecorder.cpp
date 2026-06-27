// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#include "KeyInputReplayRecorder.h"
#include "KeyInputReplayRecordDataAsset.h"
#include "KeyInputReplaySettings.h"

#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#if WITH_EDITOR
#include "UObject/Package.h"
#include "FileHelpers.h"
#endif

#if UE_EDITOR
static FAutoConsoleCommandWithWorld KeyInputReplay_RecordStart
(
	TEXT("KeyInputReplay.StartRecord"),
	TEXT(""),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, AKeyInputReplayRecorder::StaticClass(), Actors);

		if (Actors.Num() == 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed start input recording. Should place KeyInputReplay Recorder in level."));
		}
		else
		{
			AKeyInputReplayRecorder* recorder = Cast<AKeyInputReplayRecorder>(Actors[0]);
			recorder->Start();
		}
	})
);

static FAutoConsoleCommandWithWorld KeyInputReplay_RecordStop
(
	TEXT("KeyInputReplay.StopRecord"),
	TEXT(""),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, AKeyInputReplayRecorder::StaticClass(), Actors);

		if (Actors.Num() > 0)
		{
			AKeyInputReplayRecorder* recorder = Cast<AKeyInputReplayRecorder>(Actors[0]);
			recorder->Stop();
		}
	})
);
#endif

AKeyInputReplayRecorder::AKeyInputReplayRecorder()
{
	PrimaryActorTick.bCanEverTick = true;
	// RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	// TextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>("TextRender");
	// TextRenderComponent->SetupAttachment(RootComponent);
	// TextRenderComponent->HorizontalAlignment = EHTA_Center;
	// TextRenderComponent->VerticalAlignment = EVRTA_TextCenter;
	// TextRenderComponent->bHiddenInGame = true;
	// TextRenderComponent->WorldSize = 50.f;
	// TextRenderComponent->SetText(FText::FromString("KeyInputReplayRecorder"));
	// TextRenderComponent->SetTextRenderColor(FColor::Red);

	// Resister InputProcessor
	InputProcessor = MakeShared<FKeyInputReplayRecordInputProcessor>();
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}
}

AKeyInputReplayRecorder::~AKeyInputReplayRecorder()
{
	// Unresister InputProcessor
	if (FSlateApplication::IsInitialized() && InputProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
}

void AKeyInputReplayRecorder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// AutoPlay
	if (bRecording == false && DoAutoRecord == false)
	{
		if (bAutoRecord)
		{
			CurrentAutoRecordDelayFrame++;
			if (CurrentAutoRecordDelayFrame >= AutoRecordDelayFrame)
			{
				Start();
				DoAutoRecord = true;
			}
		}
	}

	// Recording
	if (bRecording)
	{
		Time += DeltaTime;
		if (PrintToScreen)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("KeyInputReplay Recording: %.2fsec"), Time), true, false, FColor::Red, 0.f);
		}
	}
}

void AKeyInputReplayRecorder::StartRecording()
{
	Start();
}

void AKeyInputReplayRecorder::StopRecording()
{
	Stop();
}

void AKeyInputReplayRecorder::Start()
{
	if (IsValid(RecordDataAsset) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed start input recording. Should setup RecordDataAsset."));
		return;
	}

	// Settings InputProcessor
	const UKeyInputReplaySettings* Settings = GetDefault<UKeyInputReplaySettings>();
	InputProcessor->Recorder = this;
	InputProcessor->IgnoreKeys = Settings->IgnoreRecordKeys;

	// Start recording.
	Reset();
	bRecording = true;
}

void AKeyInputReplayRecorder::Stop()
{
	if (bRecording)
	{
		// Save data asset.
		if (IsValid(RecordDataAsset))
		{
			RecordDataAsset->Reset();

			RecordDataAsset->bRecorded = true;
			RecordDataAsset->KeyData = KeyData;

#if UE_EDITOR
			// Checkout and Save
			RecordDataAsset->MarkPackageDirty();

			TArray<UPackage*> PackagesToSave;
			PackagesToSave.Add(RecordDataAsset->GetOutermost());
			FEditorFileUtils::PromptForCheckoutAndSave(PackagesToSave, true, true);
#endif
		}
		
		// Finalize recording.
		Reset();
		bRecording = false;
	}
}

void AKeyInputReplayRecorder::OnKeyDown(const FKeyEvent& KeyEvent)
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

void AKeyInputReplayRecorder::OnKeyUp(const FKeyEvent& KeyEvent)
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

void AKeyInputReplayRecorder::OnAnalogInput(const FAnalogInputEvent& AnalogInputEvent)
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

void AKeyInputReplayRecorder::OnMouseMove(const FPointerEvent& PointerEvent)
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

void AKeyInputReplayRecorder::OnMouseButtonDown(const FPointerEvent& PointerEvent)
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

void AKeyInputReplayRecorder::OnMouseButtonUp(const FPointerEvent& PointerEvent)
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

void AKeyInputReplayRecorder::OnMouseButtonDoubleClick(const FPointerEvent& PointerEvent)
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

void AKeyInputReplayRecorder::OnMouseWheelOrGesture(const FPointerEvent& WheelEvent)
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

void AKeyInputReplayRecorder::OnMotionDetected(const FMotionEvent& MotionEvent)
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

void AKeyInputReplayRecorder::Reset()
{
	Time = 0.f;
	KeyData.Empty();
}

bool AKeyInputReplayRecorder::IsRecordKey(const FKey& Key)
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
