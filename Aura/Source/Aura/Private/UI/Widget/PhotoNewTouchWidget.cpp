// P


#include "UI/Widget/PhotoNewTouchWidget.h"
#include "Kismet/KismetInputLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/SlateBlueprintLibrary.h"

FReply UPhotoNewTouchWidget::NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	//const int32 TouchIdx = InGestureEvent.GetPointerIndex();
	//const FVector2D ScreenPosition = InGestureEvent.GetScreenSpacePosition();
	const int32 TouchIdx = UKismetInputLibrary::PointerEvent_GetPointerIndex(InGestureEvent);
	const FVector2D ScreenPosition = UKismetInputLibrary::PointerEvent_GetScreenSpacePosition(InGestureEvent);

	FPhotoTouchInfo* TouchInfo = GetTouchInfoByIndex(TouchIdx);
	if (TouchInfo == nullptr) return FReply::Unhandled();
	
	bIsTouching = true;

	TouchInfo->bIsTouching = true;
	TouchInfo->TouchScreenPos = ScreenPosition;

	if (TouchStartedCallBack.IsBound())
	{
		TouchStartedCallBack.Broadcast(ScreenPosition);
	}

	FPhotoTouchInfo* AnotherTouchInfo = GetAnotherTouchInfo(TouchIdx);
	if (AnotherTouchInfo && AnotherTouchInfo->bIsTouching)
	{
		float Scale = UWidgetLayoutLibrary::GetViewportScale(GetWorld()->GetGameViewport());
		ZoomDistance = UKismetMathLibrary::Distance2D(ScreenPosition, AnotherTouchInfo->TouchScreenPos) / Scale;
	}

	return FReply::Handled().CaptureMouse(TakeWidget()->AsShared());
}

FReply UPhotoNewTouchWidget::NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	const int32 TouchIdx = UKismetInputLibrary::PointerEvent_GetPointerIndex(InGestureEvent);
	const FVector2D ScreenPosition = UKismetInputLibrary::PointerEvent_GetScreenSpacePosition(InGestureEvent);

	FPhotoTouchInfo* TouchInfo = GetTouchInfoByIndex(TouchIdx);
	if (TouchInfo == nullptr) return FReply::Unhandled();

	int32 TouchCount = GetTouchCount();
	if (TouchCount <= 0) return FReply::Unhandled();

	if (TouchCount == 1)
	{
		const FVector2D LastLocalPosition = USlateBlueprintLibrary::AbsoluteToLocal(InGeometry, TouchInfo->TouchScreenPos);
		const FVector2D LocalPosition = USlateBlueprintLibrary::AbsoluteToLocal(InGeometry, ScreenPosition);
		const FVector2D Delta = LocalPosition - LastLocalPosition;
		if(FMath::Abs(Delta.X) > 1 || FMath::Abs(Delta.Y) > 1)
		{
			if (TouchMovedCallBack.IsBound())
			{
				TouchMovedCallBack.Broadcast(ScreenPosition, Delta);
			}
		}
	}
	else if (TouchCount == 2)
	{
		FPhotoTouchInfo* AnotherTouchInfo = GetAnotherTouchInfo(TouchIdx);
		if (AnotherTouchInfo && AnotherTouchInfo->bIsTouching)
		{
			float Scale = UWidgetLayoutLibrary::GetViewportScale(GetWorld()->GetGameViewport());
			float NewZoomDistance = UKismetMathLibrary::Distance2D(ScreenPosition, AnotherTouchInfo->TouchScreenPos) / Scale;
			float ZoomFactor = NewZoomDistance / ZoomDistance;
			FVector2D MidPoint = (ScreenPosition + AnotherTouchInfo->TouchScreenPos) * 0.5f;
			OnZoomed(MidPoint, ZoomFactor);
			ZoomDistance = NewZoomDistance;
		}
	}

	TouchInfo->TouchScreenPos = ScreenPosition;

	return FReply::Handled();
}

FReply UPhotoNewTouchWidget::NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent)
{
	if (TouchEndedCallBack.IsBound())
	{
		TouchEndedCallBack.Broadcast(UKismetInputLibrary::PointerEvent_GetScreenSpacePosition(InGestureEvent));
	}
	ResetTouchData();
	return FReply::Handled().ReleaseMouseCapture();
}

FReply UPhotoNewTouchWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	float ZoomFactor = 1.1f; // Ëõ·ÅÒò×Ó
	const float WheelDelta = InMouseEvent.GetWheelDelta();
	const FVector2D ScreenPosition = UKismetInputLibrary::PointerEvent_GetScreenSpacePosition(InMouseEvent);
	if (WheelDelta > 0.f)
	{
		OnZoomed(ScreenPosition, ZoomFactor);
	}
	else
	{
		OnZoomed(ScreenPosition, 0.9f); /* 1.0f / ZoomFactor */
	}
	return FReply::Handled();
}

void UPhotoNewTouchWidget::OnZoomed(const FVector2D MidPoint, const float Scale)
{
	if (MouseWheelCallBack.IsBound())
	{
		MouseWheelCallBack.Broadcast(MidPoint, Scale);
	}
}

FPhotoTouchInfo* UPhotoNewTouchWidget::GetTouchInfoByIndex(const int32 PointerIndex)
{
	for (FPhotoTouchInfo& TouchInfo : TouchInfos)
	{
		if (TouchInfo.PointerIndex == PointerIndex)
		{
			return &TouchInfo;
		}
	}
	return nullptr;
}

FPhotoTouchInfo* UPhotoNewTouchWidget::GetAnotherTouchInfo(const int32 PointerIndex)
{
	for (FPhotoTouchInfo& TouchInfo : TouchInfos)
	{
		if (TouchInfo.PointerIndex != PointerIndex)
		{
			return &TouchInfo;
		}
	}
	return nullptr;
}

int32 UPhotoNewTouchWidget::GetTouchCount() const
{
	int32 Count = 0;
	for (const FPhotoTouchInfo& TouchInfo : TouchInfos)
	{
		if (TouchInfo.bIsTouching)
		{
			Count++;
		}
	}
	return Count;
}

void UPhotoNewTouchWidget::ResetTouchData()
{
	bIsTouching = false;
	for (FPhotoTouchInfo& TouchInfo : TouchInfos)
	{
		TouchInfo.Reset();
	}
}
