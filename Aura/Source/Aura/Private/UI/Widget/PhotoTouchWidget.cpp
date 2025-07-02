// P


#include "UI/Widget/PhotoTouchWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/KismetInputLibrary.h"
#include "GameFramework/HUD.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/ScaleBox.h"


int32 UPhotoTouchWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	// 调用父类的绘制逻辑，获取基础图层ID
	int32 RetLayer = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (bIsSelecting)
	{
		FVector2D BoxLeftTop(
			FMath::Min(SelectionStart.X, SelectionEnd.X),
			FMath::Min(SelectionStart.Y, SelectionEnd.Y)
		);
		FVector2D BoxSize(
			FMath::Abs(SelectionEnd.X - SelectionStart.X),
			FMath::Abs(SelectionEnd.Y - SelectionStart.Y)
		);
		FLinearColor BoxColor(0.f, 0.5f, 1.f, 0.5f);
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Brush.TintColor = FSlateColor(BoxColor);

		//FSlateDrawElement::MakeBox(
		//	OutDrawElements,
		//	RetLayer + 1,
		//	AllottedGeometry.ToPaintGeometry(BoxLeftTop, BoxSize),
		//	&Brush
		//);
	}
	return RetLayer + 1;
}

FEventReply UPhotoTouchWidget::TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	if (!CheckPointEffectiveIndex(UKismetInputLibrary::PointerEvent_GetPointerIndex(InTouchEvent)) || ImageWidget == nullptr)
	{
		return UWidgetBlueprintLibrary::Unhandled();
	}

	FVector2D ScreenPos = InTouchEvent.GetScreenSpacePosition();
	FVector2D LocalPos = ImageWidget->GetCachedGeometry().AbsoluteToLocal(ScreenPos);

	if (IsFixedRatio)
	{
		LastSelectionEnd = LocalPos;
		return UWidgetBlueprintLibrary::Handled();
	}

	if (bIsDragging &&
		LocalPos.X >= DSelectionStart.X && LocalPos.X <= DSelectionEnd.X &&
		LocalPos.Y >= DSelectionStart.Y && LocalPos.Y <= DSelectionEnd.Y)
	{
		// 如果点击位置在当前选择区域内，则开始拖动选择框
		DStartPos = LocalPos;
		DDragOffset = LocalPos - DSelectionStart;
		return UWidgetBlueprintLibrary::Handled();
	}

	SelectionStart = LocalPos;
	SelectionEnd = SelectionStart;
	bIsSelecting = true;

	TouchStartedCallBack.Broadcast(InTouchEvent);

	// Reply 事件回复的引用，用于传递和修改事件处理状态
	FEventReply Reply = UWidgetBlueprintLibrary::Handled();
	// 用CaptureMouse捕获鼠标后整个游戏将只能和锁定的Widget进行响应，该Widget的父级和子级也是不能响应的，直到调用ReleaseMouseCapture。
	return UWidgetBlueprintLibrary::CaptureMouse(Reply, this);
}

FEventReply UPhotoTouchWidget::TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	if (!CheckPointEffectiveIndex(UKismetInputLibrary::PointerEvent_GetPointerIndex(InTouchEvent)))
	{
		return UWidgetBlueprintLibrary::Unhandled();
	}

	if (IsFixedRatio)
	{
		
	}

	if (bIsSelecting)
	{
		SelectionEnd = ImageWidget->GetCachedGeometry().AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
		if ((SelectionEnd - LastSelectionEnd).SizeSquared() < 4.0f)
		{
			return  UWidgetBlueprintLibrary::Unhandled();
		}
		LastSelectionEnd = SelectionEnd;

		FVector2D WidgetSize = ImageWidget->GetCachedGeometry().GetLocalSize();
		FVector2D BoxLeftTop(FMath::Min(SelectionStart.X, SelectionEnd.X), FMath::Min(SelectionStart.Y, SelectionEnd.Y));
		FVector2D BoxSize(FMath::Abs(SelectionEnd.X - SelectionStart.X), FMath::Abs(SelectionEnd.Y - SelectionStart.Y));
		FVector2D CenterPoint = BoxLeftTop + BoxSize * 0.5f;
		FVector2D NormalizedSize(
			FMath::Clamp(BoxSize.X / WidgetSize.X, 0.0f, 1.0f),
			FMath::Clamp(BoxSize.Y / WidgetSize.Y, 0.0f, 1.0f)
		);
		FVector2D NormalizedCenter(
			FMath::Clamp(CenterPoint.X / WidgetSize.X, 0.0f, 1.0f),
			FMath::Clamp(CenterPoint.Y / WidgetSize.Y, 0.0f, 1.0f)
		);
		UE_LOG(LogTemp, Log, TEXT("TouchMoved:%s"), *BoxSize.ToString());
		SelectAreaCallBack.Broadcast(NormalizedCenter, NormalizedSize, false);

		UpdateCanvasPanelSlot(BoxLeftTop, BoxSize);
	}

	if (bIsDragging)
	{
		FVector2D LocalPos = ImageWidget->GetCachedGeometry().AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
		FVector2D Delta = LocalPos - DStartPos;
		DStartPos = LocalPos;

		// 根据鼠标的偏移量更新选中框的位置
		FVector2D NewSelectionStart = LocalPos - DDragOffset;
		FVector2D BoxSize = DSelectionEnd - DSelectionStart;

		// 限制选中框的位置，确保不会超出小部件边界
		FVector2D WidgetSize = ImageWidget->GetCachedGeometry().GetLocalSize();
		NewSelectionStart.X = FMath::Clamp(NewSelectionStart.X, 0.f, WidgetSize.X - BoxSize.X);
		NewSelectionStart.Y = FMath::Clamp(NewSelectionStart.Y, 0.f, WidgetSize.Y - BoxSize.Y);

		DSelectionStart = NewSelectionStart;
		DSelectionEnd = NewSelectionStart + BoxSize;

		FVector2D NormalizedCenter(
			FMath::Clamp((DSelectionStart.X + BoxSize.X * 0.5f) / WidgetSize.X, 0.0f, 1.0f),
			FMath::Clamp((DSelectionStart.Y + BoxSize.Y * 0.5f) / WidgetSize.Y, 0.0f, 1.0f)
		);
		FVector2D NormalizedSize(
			FMath::Clamp(BoxSize.X / WidgetSize.X, 0.0f, 1.0f),
			FMath::Clamp(BoxSize.Y / WidgetSize.Y, 0.0f, 1.0f)
		);
		SelectAreaCallBack.Broadcast(NormalizedCenter, NormalizedSize, false);

		FVector2D BoxLeftTop(FMath::Min(NewSelectionStart.X, DSelectionEnd.X), FMath::Min(NewSelectionStart.Y, DSelectionEnd.Y));
		//FVector2D ImageDrawSize = ImageWidget->Brush.GetImageSize();
		//FVector2D ScaleFactor = WidgetSize / ImageDrawSize;
		UpdateCanvasPanelSlot(BoxLeftTop, BoxSize);
	}

	return UWidgetBlueprintLibrary::Handled();
}

FEventReply UPhotoTouchWidget::TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	if (!CheckPointEffectiveIndex(UKismetInputLibrary::PointerEvent_GetPointerIndex(InTouchEvent)))
	{
		return UWidgetBlueprintLibrary::Unhandled();
	}
	if (bIsDragging)
	{
		return UWidgetBlueprintLibrary::Handled();
	}
	if (bIsSelecting)
	{
		bIsSelecting = false;

		// 选择结束，更新选择框UI
		FIntRect CropArea(
			FMath::Min(SelectionStart.X, SelectionEnd.X),
			FMath::Min(SelectionStart.Y, SelectionEnd.Y),
			FMath::Max(SelectionStart.X, SelectionEnd.X),
			FMath::Max(SelectionStart.Y, SelectionEnd.Y)
		);

		bIsDragging = true;
		DSelectionStart = SelectionStart;
		DSelectionEnd = SelectionEnd;

		SelectAreaCallBack.Broadcast(SelectionStart, SelectionEnd, true);

		TouchEndedCallBack.Broadcast(InTouchEvent);

		SelectionEnd = FVector2D::Zero();
		LastSelectionEnd = FVector2D::Zero();
	}

	FEventReply Reply = UWidgetBlueprintLibrary::Handled();
	return UWidgetBlueprintLibrary::ReleaseMouseCapture(Reply);
}

bool UPhotoTouchWidget::CheckPointEffectiveIndex(const int32 PointIndex)
{
	if (PointIndex == -99)
	{
		return false;
	}
	return true;
}

void UPhotoTouchWidget::UpdateCanvasPanelSlot(const FVector2D& BoxLeftTop, const FVector2D& BoxSize)
{
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(DecorateCanvasPanel->Slot))
	{
		const FGeometry& ParentGeometry = DecorateCanvasPanel->GetParent()->GetCachedGeometry();
		FVector2D AbsolutePos = ImageWidget->GetCachedGeometry().LocalToAbsolute(BoxLeftTop);
		FVector2D ParentLocalPos = ParentGeometry.AbsoluteToLocal(AbsolutePos);
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
		CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
		CanvasSlot->SetPosition(ParentLocalPos);
		CanvasSlot->SetSize(BoxSize/* * ParentGeometry.Scale*/);
	}
}