// P


#include "UI/Widget/PhotoWidget.h"

FEventReply UPhotoWidget::TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
	SelectionStart = MyGeometry.AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
	bIsSelecting = true;
	// 初始化选择框UI（如半透明矩形）
	return Reply;
}

FEventReply UPhotoWidget::TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
	if (bIsSelecting)
	{
		// 更新选择框的结束位置
		SelectionEnd = MyGeometry.AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
		// 计算选择框的大小
		FVector2D BoxSize = FVector2D(FMath::Abs(SelectionEnd.X - SelectionStart.X), FMath::Abs(SelectionEnd.Y - SelectionStart.Y));
		// 计算选择框的左上角位置
		FVector2D BoxLeftTop = FVector2D(
			FMath::Min(SelectionStart.X, SelectionEnd.X),
			FMath::Min(SelectionStart.Y, SelectionEnd.Y)
		);
		// 获取小部件的本地大小
		FVector2D WidgetSize = MyGeometry.GetLocalSize();
		// 计算选择框的中心点（中心点坐标可以表示为：中心点x = 左上角x + 宽度/2，中心点y = 左上角y + 高度/2。）
		FVector2D CenterPoint = BoxLeftTop + BoxSize * 0.5f;
		// 将中心点归一化到0-1范围
		float NormailizedX = FMath::Clamp(CenterPoint.X / WidgetSize.X, 0.0f, 1.0f);
		float NormailizedY = FMath::Clamp(CenterPoint.Y / WidgetSize.Y, 0.0f, 1.0f);


		// 更新选择框UI（如半透明矩形）
		UpdateSelectionBox(BoxSize);
	}
	return Reply;
}

FEventReply UPhotoWidget::TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
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
	}
	return Reply;
}
