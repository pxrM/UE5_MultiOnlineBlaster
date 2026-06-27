// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#include "KeyInputReplayPlayer.h"
#include "KeyInputReplayRecordDataAsset.h"

#include "Framework/Application/SlateApplication.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AKeyInputReplayPlayer::AKeyInputReplayPlayer()
{
	PrimaryActorTick.bCanEverTick = true;
	//
	// RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
	// TextRenderComponent = CreateDefaultSubobject<UTextRenderComponent>("TextRender");
	// TextRenderComponent->SetupAttachment(RootComponent);
	// TextRenderComponent->HorizontalAlignment = EHTA_Center;
	// TextRenderComponent->VerticalAlignment = EVRTA_TextCenter;
	// TextRenderComponent->bHiddenInGame = true;
	// TextRenderComponent->WorldSize = 50.f;
	// TextRenderComponent->SetText(FText::FromString("KeyInputReplayPlayer"));
	// TextRenderComponent->SetTextRenderColor(FColor::Green);
}

void AKeyInputReplayPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RecordDataList.Num() == 0)
	{
		return;
	}

	// AutoPlay
	if (bNeverPlay && bPlaying == false)
	{
		if (bAutoPlay)
		{
			CurrentAutoPlayDelayFrame++;
			if (CurrentAutoPlayDelayFrame >= AutoPlayDelayFrame)
			{
				Play();
			}
		}
	}

	// Playing
	if (bPlaying)
	{
		if (RecordDataIndex >= RecordDataList.Num())
		{
			Stop();
		}

		Time += DeltaTime;
		if (PrintToScreen)
		{
			UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("KeyInputReplay Playing: %.2fsec, DataID[%d]"), Time, RecordDataIndex), true, false, FColor::Green, 0.f);
		}

		// Key pressed or released
		while (true)
		{
			if (KeyDataIndex >= RecordDataList[RecordDataIndex]->KeyData.Num())
			{
				RecordDataIndex++;
				break;
			}

			FKeyInputReplayKeyData* keyData = &RecordDataList[RecordDataIndex]->KeyData[KeyDataIndex];
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
			case EKeyInputReplayInputType::MotionDetected:		   FSlateApplication::Get().ProcessMotionDetectedEvent(keyData->InputEvents.GetMotionEvent()); break;
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

void AKeyInputReplayPlayer::Play()
{
	if (RecordDataList.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed start replay. Should setup RecordDataList."));
		return;
	}

	if (bPlaying == false)
	{
		Reset();
		bPlaying = true;
		bNeverPlay = false;
	}
}

void AKeyInputReplayPlayer::Stop()
{
	if (PrintToScreen)
	{
		UKismetSystemLibrary::PrintString(this, FString::Printf(TEXT("KeyInputReplay Playing: Finished!")), true, false, FColor::Green, 5.f);
	}

	Reset();
	bPlaying = false;
}



//-------------------------------------------------------------------------------------------------------
// Private Functions

void AKeyInputReplayPlayer::Reset()
{
	Time = 0.f;
	RecordDataIndex = 0;
	KeyDataIndex = 0;
}