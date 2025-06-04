// P


#include "UI/Widget/PhotoWidget.h"

FEventReply UPhotoWidget::TouchGesture(FGeometry MyGeometry, const FPointerEvent& GestureEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
	return Reply;
}

FEventReply UPhotoWidget::TouchStarted(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
	SelectionStart = MyGeometry.AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
	bIsSelecting = true;
	// ��ʼ��ѡ���UI�����͸�����Σ�
	return Reply;
}

FEventReply UPhotoWidget::TouchMoved(FGeometry MyGeometry, const FPointerEvent& InTouchEvent)
{
	FEventReply Reply;
	Reply.NativeReply = FReply::Handled();
	if (bIsSelecting)
	{
		// ����ѡ���Ľ���λ��
		SelectionEnd = MyGeometry.AbsoluteToLocal(InTouchEvent.GetScreenSpacePosition());
		// ����ѡ���Ĵ�С
		FVector2D BoxSize = FVector2D(FMath::Abs(SelectionEnd.X - SelectionStart.X), FMath::Abs(SelectionEnd.Y - SelectionStart.Y));
		// ����ѡ�������Ͻ�λ��
		FVector2D BoxLeftTop = FVector2D(
			FMath::Min(SelectionStart.X, SelectionEnd.X),
			FMath::Min(SelectionStart.Y, SelectionEnd.Y)
		);
		// ��ȡС�����ı��ش�С
		FVector2D WidgetSize = MyGeometry.GetLocalSize();
		// ����ѡ�������ĵ㣨���ĵ�������Ա�ʾΪ�����ĵ�x = ���Ͻ�x + ���/2�����ĵ�y = ���Ͻ�y + �߶�/2����
		FVector2D CenterPoint = BoxLeftTop + BoxSize * 0.5f;
		// �����ĵ��һ����0-1��Χ
		float NormailizedX = FMath::Clamp(CenterPoint.X / WidgetSize.X, 0.0f, 1.0f);
		float NormailizedY = FMath::Clamp(CenterPoint.Y / WidgetSize.Y, 0.0f, 1.0f);


		// ����ѡ���UI�����͸�����Σ�
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
		// ѡ�����������ѡ���UI
		FIntRect CropArea(
			FMath::Min(SelectionStart.X, SelectionEnd.X),
			FMath::Min(SelectionStart.Y, SelectionEnd.Y),
			FMath::Max(SelectionStart.X, SelectionEnd.X),
			FMath::Max(SelectionStart.Y, SelectionEnd.Y)
		);
	}
	return Reply;
}
