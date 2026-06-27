// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Input/Events.h"
#include "KeyInputReplayDataTypes.generated.h"

UENUM(BlueprintType)
enum class EKeyInputReplayInputType : uint8
{
	KeyDown,
	KeyUp,
	AnalogInput,
	MouseMove,
	MouseButtonDown,
	MouseButtonUp,
	MouseButtonDoubleClick,
	MouseWheelOrGesture,
	MotionDetected,
};

USTRUCT(BlueprintType)
struct KEYINPUTREPLAY_API FKeyInputReplay_ModifierKeysState
{
	GENERATED_USTRUCT_BODY();

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bIsLeftShiftDown = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bIsRightShiftDown = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bIsLeftControlDown = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bIsRightControlDown = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bIsLeftAltDown = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bIsRightAltDown = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bIsLeftCommandDown = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bIsRightCommandDown = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bAreCapsLocked = false;

public:
	void SetModifierKeysState(const FModifierKeysState& InModifierKeys)
	{
		bIsLeftShiftDown    = InModifierKeys.IsLeftShiftDown();
		bIsRightShiftDown   = InModifierKeys.IsRightShiftDown();
		bIsLeftControlDown  = InModifierKeys.IsLeftControlDown();
		bIsRightControlDown = InModifierKeys.IsRightControlDown();
		bIsLeftAltDown      = InModifierKeys.IsLeftAltDown();
		bIsRightAltDown     = InModifierKeys.IsRightAltDown();
		bIsLeftCommandDown  = InModifierKeys.IsLeftCommandDown();
		bIsRightCommandDown = InModifierKeys.IsRightCommandDown();
		bAreCapsLocked      = InModifierKeys.AreCapsLocked();
	};

	FModifierKeysState GetModifierKeysState() const
	{
		return FModifierKeysState(bIsLeftShiftDown, bIsRightShiftDown, bIsLeftControlDown, bIsRightControlDown, bIsLeftAltDown, bIsRightAltDown, bIsLeftCommandDown, bIsRightCommandDown, bAreCapsLocked);
	};
};

USTRUCT(BlueprintType)
struct KEYINPUTREPLAY_API FKeyInputReplayInputEvents
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Input")
	FKeyInputReplay_ModifierKeysState ModifierKeys;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	bool bIsRepeat = false;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Input")
	int64 UserIndex = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Key")
	FKey Key;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Key")
	int64 CharacterCode = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Key")
	int64 KeyCode = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Analog")
	float AnalogValue = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pointer")
	FVector2D ScreenSpacePosition = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pointer")
	FVector2D LastScreenSpacePosition = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Pointer")
	FVector2D CursorDelta = FVector2D::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pointer")
	TArray<FKey> PressedButtons;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pointer")
	FKey EffectingButton;
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pointer")
	int64 PointerIndex = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Pointer")
	float WheelDelta = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Motion")
	FVector Tilt = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Motion")
	FVector RotationRate = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Motion")
	FVector Gravity = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Motion")
	FVector Acceleration = FVector::ZeroVector;

public:
	void SetKeyEvent(const FKeyEvent& InKeyEvent)
	{
		Key = InKeyEvent.GetKey();
		CharacterCode = InKeyEvent.GetCharacter();
		KeyCode = InKeyEvent.GetKeyCode();

		(&ModifierKeys)->SetModifierKeysState(InKeyEvent.GetModifierKeys());
		bIsRepeat = InKeyEvent.IsRepeat();
		UserIndex = InKeyEvent.GetUserIndex();
	};

	void SetAnalogInputEvent(const FAnalogInputEvent& InAnalogInputEvent)
	{
		AnalogValue = InAnalogInputEvent.GetAnalogValue();
		
		Key = InAnalogInputEvent.GetKey();
		CharacterCode = InAnalogInputEvent.GetCharacter();
		KeyCode = InAnalogInputEvent.GetKeyCode();

		(&ModifierKeys)->SetModifierKeysState(InAnalogInputEvent.GetModifierKeys());
		bIsRepeat = InAnalogInputEvent.IsRepeat();
		UserIndex = InAnalogInputEvent.GetUserIndex();
	};

	void SetPointerEvent(const FPointerEvent& InPointerEvent)
	{
		ScreenSpacePosition = InPointerEvent.GetScreenSpacePosition();
		LastScreenSpacePosition = InPointerEvent.GetLastScreenSpacePosition();
		CursorDelta = InPointerEvent.GetCursorDelta();
		PressedButtons = InPointerEvent.GetPressedButtons().Array();
		EffectingButton = InPointerEvent.GetEffectingButton();
		PointerIndex = InPointerEvent.GetPointerIndex();
		WheelDelta = InPointerEvent.GetWheelDelta();

		(&ModifierKeys)->SetModifierKeysState(InPointerEvent.GetModifierKeys());
		bIsRepeat = InPointerEvent.IsRepeat();
		UserIndex = InPointerEvent.GetUserIndex();
	};

	void SetMotionEvent(const FMotionEvent& InMotionEvent)
	{
		Tilt = InMotionEvent.GetTilt();
		RotationRate = InMotionEvent.GetRotationRate();
		Gravity = InMotionEvent.GetGravity();
		Acceleration = InMotionEvent.GetAcceleration();

		(&ModifierKeys)->SetModifierKeysState(InMotionEvent.GetModifierKeys());
		bIsRepeat = InMotionEvent.IsRepeat();
		UserIndex = InMotionEvent.GetUserIndex();
	};

	FKeyEvent GetKeyEvent() const
	{
		return FKeyEvent(
			Key,
			(&ModifierKeys)->GetModifierKeysState(),
			UserIndex,
			bIsRepeat,
			CharacterCode,
			KeyCode
			);
	};

	FAnalogInputEvent GetAnalogInputEvent() const
	{
		return FAnalogInputEvent(
			Key,
			(&ModifierKeys)->GetModifierKeysState(),
			UserIndex,
			bIsRepeat,
			CharacterCode,
			KeyCode,
			AnalogValue
			);
	};

	FPointerEvent GetMouseMoveEvent() const
	{
		return FPointerEvent(
			UserIndex,
			PointerIndex,
			ScreenSpacePosition,
			LastScreenSpacePosition,
			CursorDelta,
			TSet<FKey>(PressedButtons),
			ModifierKeys.GetModifierKeysState()
			);
	};

	FPointerEvent GetMouseButtonUpEvent() const
	{
		return FPointerEvent(
			UserIndex,
			PointerIndex,
			ScreenSpacePosition,
			LastScreenSpacePosition,
			TSet<FKey>(PressedButtons),
			EffectingButton,
			WheelDelta,
			(&ModifierKeys)->GetModifierKeysState()
			);
	};

	FPointerEvent GetMouseWheelOrGestureEvent() const
	{
		return FPointerEvent(
			UserIndex,
			PointerIndex,
			ScreenSpacePosition,
			LastScreenSpacePosition,
			TSet<FKey>(PressedButtons),
			EffectingButton,
			WheelDelta,
			(&ModifierKeys)->GetModifierKeysState()
			);
	};

	FPointerEvent GetMouseButtonDownEvent() const
	{
		return FPointerEvent(
			UserIndex,
			PointerIndex,
			ScreenSpacePosition,
			LastScreenSpacePosition,
			TSet<FKey>(PressedButtons),
			EffectingButton,
			WheelDelta,
			(&ModifierKeys)->GetModifierKeysState()
			);
	};

	FMotionEvent GetMotionEvent() const
	{
		return FMotionEvent(
			UserIndex,
			Tilt,
			RotationRate,
			Gravity,
			Acceleration
			);
	};
};

USTRUCT(BlueprintType)
struct KEYINPUTREPLAY_API FKeyInputReplayKeyData
{
	GENERATED_USTRUCT_BODY();

public:
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "KeyInputReplay")
	float Time = 0.f;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "KeyInputReplay")
	EKeyInputReplayInputType InputType = EKeyInputReplayInputType::KeyDown;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "KeyInputReplay")
	FKeyInputReplayInputEvents InputEvents;
};