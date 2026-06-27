// Copyright 2022-2025 Leon Gameworks. All Rights Reserved.

#include "KeyInputReplayRecordInputProcessor.h"
#include "KeyInputReplayRecorder.h"
#include "KeyInputRecord.h"
void FKeyInputReplayRecordInputProcessor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
}

bool FKeyInputReplayRecordInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (Recorder.IsValid())
	{
		if (!IgnoreKeys.Contains(InKeyEvent.GetKey()))
		{
			Recorder->OnKeyDown(InKeyEvent);
		}
	}
	return  false;
}

bool FKeyInputReplayRecordInputProcessor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (Recorder.IsValid())
	{
		if (!IgnoreKeys.Contains(InKeyEvent.GetKey()))
		{
			Recorder->OnKeyUp(InKeyEvent);
		}
	}
	return  false;
}

bool FKeyInputReplayRecordInputProcessor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	if (Recorder.IsValid())
	{
		if (!IgnoreKeys.Contains(InAnalogInputEvent.GetKey()))
		{
			Recorder->OnAnalogInput(InAnalogInputEvent);
		}
	}
	return  false;
}

bool FKeyInputReplayRecordInputProcessor::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (Recorder.IsValid())
	{
		Recorder->OnMouseMove(MouseEvent);
	}
	return  false;
}

bool FKeyInputReplayRecordInputProcessor::HandleMouseButtonDownEvent( FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (Recorder.IsValid())
	{
		Recorder->OnMouseButtonDown(MouseEvent);
	}
	return  false;
}

bool FKeyInputReplayRecordInputProcessor::HandleMouseButtonUpEvent( FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (Recorder.IsValid())
	{
		Recorder->OnMouseButtonUp(MouseEvent);
	}
	return  false;
}

bool FKeyInputReplayRecordInputProcessor::HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (Recorder.IsValid())
	{
		Recorder->OnMouseButtonDoubleClick(MouseEvent);
	}
	return  false;
}

bool FKeyInputReplayRecordInputProcessor::HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent)
{
	if (Recorder.IsValid())
	{
		Recorder->OnMouseWheelOrGesture(InWheelEvent);
	}
	return  false;
}

bool FKeyInputReplayRecordInputProcessor::HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent)
{
	if (Recorder.IsValid())
	{
		Recorder->OnMotionDetected(MotionEvent);
	}
	return  false;
}