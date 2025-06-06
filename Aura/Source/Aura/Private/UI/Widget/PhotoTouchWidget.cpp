// P


#include "UI/Widget/PhotoTouchWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Kismet/KismetInputLibrary.h"

FEventReply UPhotoTouchWidget::TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	
	if (CheckPointEffectiveIndex(UKismetInputLibrary::PointerEvent_GetPointerIndex(InTouchEvent)))
	{
		return UWidgetBlueprintLibrary::Unhandled();
	}
	SelectionStart = MyGeometry.AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
	bIsSelecting = true;
	// 初始化选择框UI（如半透明矩形）

	TouchStartedCallBack.Broadcast(InTouchEvent);

	// Reply 事件回复的引用，用于传递和修改事件处理状态
	FEventReply Reply = UWidgetBlueprintLibrary::Handled();
	// 用CaptureMouse捕获鼠标后整个游戏将只能和锁定的Widget进行响应，该Widget的父级和子级也是不能响应的，直到调用ReleaseMouseCapture。
	return UWidgetBlueprintLibrary::CaptureMouse(Reply, this);
}

FEventReply UPhotoTouchWidget::TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	if (CheckPointEffectiveIndex(UKismetInputLibrary::PointerEvent_GetPointerIndex(InTouchEvent)))
	{
		UWidgetBlueprintLibrary::Handled();
	}
	
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
		//UpdateSelectionBox(BoxSize);

		TouchMovedCallBack.Broadcast(InTouchEvent);
	}
	
	return UWidgetBlueprintLibrary::Handled();
}

FEventReply UPhotoTouchWidget::TouchEnded(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	if (CheckPointEffectiveIndex(UKismetInputLibrary::PointerEvent_GetPointerIndex(InTouchEvent)))
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

		TouchEndedCallBack.Broadcast(InTouchEvent);
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
