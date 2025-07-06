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
	FVector2D LocalPos = ImageMaskWidget->GetCachedGeometry().AbsoluteToLocal(ScreenPos);

	if (bIsSelected &&
		LocalPos.X >= SelectionStart.X && LocalPos.X <= SelectionEnd.X &&
		LocalPos.Y >= SelectionStart.Y && LocalPos.Y <= SelectionEnd.Y)
	{
		// 如果点击位置在当前选择区域内，则开始拖动选择框
		bIsDragging = true;
		DStartPos = LocalPos;
		DDragOffset = LocalPos - SelectionStart;
	}
	else if(!bIsSelected)
	{
		SelectionStart = LocalPos;
		SelectionEnd = SelectionStart;
		bIsSelecting = true;
	}
	else
	{
		bIsMoveImg = true;
		TouchStartImagePosition = ImageWidget->GetCachedGeometry().GetAbsolutePosition();
		TouchStartMousePosition = InTouchEvent.GetScreenSpacePosition() - TouchStartImagePosition;
	}

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

	if (bIsSelecting)
	{
		SelectionEnd = ImageMaskWidget->GetCachedGeometry().AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
		if ((SelectionEnd - LastSelectionEnd).SizeSquared() < 4.0f)
		{
			return  UWidgetBlueprintLibrary::Unhandled();
		}
		LastSelectionEnd = SelectionEnd;

		FVector2D WidgetSize = ImageMaskWidget->GetCachedGeometry().GetLocalSize();
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
		SelectionStart = BoxLeftTop;

		SelectAreaCallBack.Broadcast(NormalizedCenter, NormalizedSize, false);

		UpdateCanvasPanelSlot(BoxLeftTop, BoxSize);
	}

	if (bIsDragging)
	{
		FVector2D LocalPos = ImageMaskWidget->GetCachedGeometry().AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
		FVector2D Delta = LocalPos - DStartPos;
		DStartPos = LocalPos;

		// 根据鼠标的偏移量更新选中框的位置
		FVector2D NewSelectionStart = LocalPos - DDragOffset;
		FVector2D BoxSize = SelectionEnd - SelectionStart;

		// 限制选中框的位置，确保不会超出小部件边界
		FVector2D WidgetSize = ImageMaskWidget->GetCachedGeometry().GetLocalSize();
		NewSelectionStart.X = FMath::Clamp(NewSelectionStart.X, 0.f, WidgetSize.X - BoxSize.X);
		NewSelectionStart.Y = FMath::Clamp(NewSelectionStart.Y, 0.f, WidgetSize.Y - BoxSize.Y);

		SelectionStart = NewSelectionStart;
		SelectionEnd = NewSelectionStart + BoxSize;

		FVector2D NormalizedCenter(
			FMath::Clamp((SelectionStart.X + BoxSize.X * 0.5f) / WidgetSize.X, 0.0f, 1.0f),
			FMath::Clamp((SelectionStart.Y + BoxSize.Y * 0.5f) / WidgetSize.Y, 0.0f, 1.0f)
		);
		FVector2D NormalizedSize(
			FMath::Clamp(BoxSize.X / WidgetSize.X, 0.0f, 1.0f),
			FMath::Clamp(BoxSize.Y / WidgetSize.Y, 0.0f, 1.0f)
		);
		SelectAreaCallBack.Broadcast(NormalizedCenter, NormalizedSize, false);

		FVector2D BoxLeftTop(FMath::Min(NewSelectionStart.X, SelectionEnd.X), FMath::Min(NewSelectionStart.Y, SelectionEnd.Y));
		UpdateCanvasPanelSlot(BoxLeftTop, BoxSize);
	}
	
	if (bIsMoveImg)
	{
		const FVector2D MouseDelta = InTouchEvent.GetScreenSpacePosition() - TouchStartMousePosition;
		FVector2D TargetScreenPos =  MouseDelta;
		float CurrentScale = static_cast<float>(ImageWidget->GetRenderTransform().Scale.X);
		if (UCanvasPanelSlot* ImageParentSlot = Cast<UCanvasPanelSlot>(ImageWidget->GetParent()->Slot))
		{
			FVector2D LocalPos = ImageWidget->GetParent()->GetCachedGeometry().AbsoluteToLocal(UpdateImgPos(TargetScreenPos/= CurrentScale));

			if (UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(ImageWidget->Slot))
			{
				ImageSlot->SetPosition(LocalPos);
			}
		}
		//float CurrentScale = static_cast<float>(ImageWidget->GetRenderTransform().Scale.X);
		//FVector2D ScaledLocalPos = ImageWidget->GetParent()->GetCachedGeometry().AbsoluteToLocal(
		//	TargetScreenPos / CurrentScale
		//);

		//if (UCanvasPanelSlot* ImageSlot = Cast<UCanvasPanelSlot>(ImageWidget->Slot))
		//{
		//	ImageSlot->SetPosition(ScaledLocalPos);
		//}
		
	}

	return UWidgetBlueprintLibrary::Handled();
}

FEventReply UPhotoTouchWidget::TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	if (!CheckPointEffectiveIndex(UKismetInputLibrary::PointerEvent_GetPointerIndex(InTouchEvent)))
	{
		return UWidgetBlueprintLibrary::Unhandled();
	}

	if (bIsSelecting)
	{
		// 选择结束，更新选择框UI
		FIntRect CropArea(
			FMath::Min(SelectionStart.X, SelectionEnd.X),
			FMath::Min(SelectionStart.Y, SelectionEnd.Y),
			FMath::Max(SelectionStart.X, SelectionEnd.X),
			FMath::Max(SelectionStart.Y, SelectionEnd.Y)
		);
		bIsSelected = true;

		SelectAreaCallBack.Broadcast(SelectionStart, SelectionEnd, true);

		TouchEndedCallBack.Broadcast(InTouchEvent);

		//SelectionEnd = FVector2D::Zero();
		LastSelectionEnd = FVector2D::Zero();
	}
	bIsSelecting = false;
	bIsDragging = false;
	bIsMoveImg = false;

	FEventReply Reply = UWidgetBlueprintLibrary::Handled();
	return UWidgetBlueprintLibrary::ReleaseMouseCapture(Reply);
}

FReply UPhotoTouchWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseWheel(InGeometry, InMouseEvent);

	// 1. 获取当前缩放参数
	float WheelDelta = InMouseEvent.GetWheelDelta(); // 滚轮方向（+1放大，-1缩小）
	float CurrentScale = static_cast<float>(ImageWidget->GetRenderTransform().Scale.X);
	FVector2D OriginalSize = ImageWidget->GetDesiredSize(); // 原始尺寸

	// 2. 计算新缩放值并限制范围 (0.5x ~ 5x)
	float ScaleStep = 0.1f;
	float NewScale = FMath::Clamp(CurrentScale + WheelDelta * ScaleStep, 1.0f, 5.0f);

	// 3. 获取鼠标相对于图片的位置（缩放中心）
	FVector2D ImageAbsolutePos = ImageWidget->GetCachedGeometry().AbsoluteToLocal(ImageWidget->GetCachedGeometry().AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition()));
	float RelativeX = ImageAbsolutePos.X / OriginalSize.X;
	float RelativeY = ImageAbsolutePos.Y / OriginalSize.Y;

	// 4. 计算缩放前后的位置偏移（保持鼠标位置不变）
	FVector2D SizeChange = OriginalSize * (NewScale - CurrentScale);
	FVector2D Offset = FVector2D(
		SizeChange.X * (0.5f - RelativeX),
		SizeChange.Y * (0.5f - RelativeY)
	);

	// 5. 应用新的缩放和位置
	FVector2D OriginalPosition = ImageWidget->RenderTransform.Translation;
	ImageWidget->SetRenderScale(FVector2D(NewScale, NewScale));
	//ImageWidget->SetRenderTranslation(OriginalPosition + Offset);

	return FReply::Handled();
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
		FVector2D AbsolutePos = ImageMaskWidget->GetCachedGeometry().LocalToAbsolute(BoxLeftTop);
		FVector2D ParentLocalPos = ParentGeometry.AbsoluteToLocal(AbsolutePos);
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
		CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
		CanvasSlot->SetPosition(ParentLocalPos);
		CanvasSlot->SetSize(BoxSize/* * ParentGeometry.Scale*/);
	}
}

void UPhotoTouchWidget::UpdateSizePos()
{
	FGeometry ImageWidgetGeo = ImageWidget->GetCachedGeometry();
	InitialImagePosition = ImageWidgetGeo.GetAbsolutePosition();
	InitialImageSize = ImageWidgetGeo.GetAbsoluteSize();
}

FVector2D UPhotoTouchWidget::UpdateImgPos(FVector2D TargetScreenPos)
{
	const FGeometry& ParentGeometry = ImageMaskWidget->GetParent()->GetCachedGeometry();
	FVector2D CropScreenStartPos = ParentGeometry.LocalToAbsolute(SelectionStart);
	FVector2D CropScreenEndPos = ParentGeometry.LocalToAbsolute(SelectionEnd);

	FGeometry ImageWidgetGeo = ImageWidget->GetCachedGeometry();
	FVector2D ImageSize = ImageWidgetGeo.GetAbsoluteSize();
	FVector2D ImageTargetEndPos = TargetScreenPos + ImageSize;
	UE_LOG(LogTemp, Log, TEXT("ImageWidget:%s"), *ImageWidget->GetCachedGeometry().GetAbsolutePosition().ToString());
	UE_LOG(LogTemp, Log, TEXT("CropScreenStartPos:%s"), *CropScreenStartPos.ToString());

	float CurrentScale = static_cast<float>(ImageWidget->GetRenderTransform().Scale.X);
	FVector2D NewPos = TargetScreenPos;

	// X轴约束：确保绿色始终在红色左侧或匹配左边界
	if (NewPos.X > CropScreenStartPos.X) {
		// 绿色左上角进入红色区域→向右推到红色左边界
		NewPos.X = CropScreenStartPos.X;
		UE_LOG(LogTemp, Log, TEXT("1"));
	}
	else if (ImageTargetEndPos.X < CropScreenEndPos.X) {
		// 绿色右边界未覆盖红色→向右推到刚好覆盖
		NewPos.X = CropScreenEndPos.X - ImageSize.X;
		UE_LOG(LogTemp, Log, TEXT("2"));
	}

	// Y轴约束：确保绿色始终在红色上方或匹配上边界
	if (NewPos.Y > CropScreenStartPos.Y) {
		// 绿色左上角进入红色区域→向下推到红色上边界
		NewPos.Y = CropScreenStartPos.Y;
		UE_LOG(LogTemp, Log, TEXT("3"));
	}
	else if (ImageTargetEndPos.Y < CropScreenEndPos.Y) {
		// 绿色下边界未覆盖红色→向下推到刚好覆盖
		NewPos.Y = CropScreenEndPos.Y - ImageSize.Y;
		UE_LOG(LogTemp, Log, TEXT("4"));
	}

	const bool bIsNotValidPosition =
		// 绿色左边界 >= 红色左边界
		(TargetScreenPos.X <= CropScreenStartPos.X) &&
		// 绿色上边界 >= 红色上边界
		(TargetScreenPos.Y <= CropScreenStartPos.Y) &&
		// 绿色右边界 <= 红色右边界
		(ImageTargetEndPos.X  >= CropScreenEndPos.X) &&
		// 绿色下边界 <= 红色下边界
		(ImageTargetEndPos.Y >= CropScreenEndPos.Y);

	return NewPos;
}


