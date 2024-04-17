// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNDWidget.h"


void SCustomNDWidget::Construct(const FArguments& InArgs)
{
	Mean = InArgs._Mean;
	StandardDeviation = InArgs._StandardDeviation;
	OnMeanChanged = InArgs._OnMeanChanged;
	OnStandardDeviationChanged = InArgs._OnStandardDeviationChanged;
}

int32 SCustomNDWidget::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	/*���ڻ����Զ��� Slate �ؼ��ĺ���*/

	// ������Ҫ���Ƶ������ϵĵ��������
	const int32 NumPoints = 512;
	// ������һ����ά�������� Points�����ڴ洢Ҫ���Ƶ������ϵĵ㣬��Ϊ��Ԥ�����㹻�Ŀռ䡣
	TArray<FVector2D> Points;
	Points.Reserve(NumPoints);

	// ��ȡ�����ڽ������ϵĵ�Ӿֲ�����ϵת�����ؼ�����ϵ�Ķ�ά�任���� PointsTransform
	const FTransform2D PointsTransform = GetPointsTransform(AllottedGeometry);

	for (int32 PointIndex = 0; PointIndex < NumPoints; ++PointIndex)
	{
		// ���㵱ǰ���������ϵ� X ���꣬��Χ�� 0 �� 1��
		const float X = PointIndex / (NumPoints - 1.0);
		// �����׼���� X ���꣬ͨ������ǰ��� X �����ȥƽ��ֵ�������Ա�׼�
		const float D = (X - Mean.Get()) / StandardDeviation.Get();
		// ���ݸ�˹�ֲ��������㵱ǰ���������ϵ� Y ���ꡣ
		const float Y = FMath::Exp(-0.5f * D * D);

		Points.Add(PointsTransform.TransformPoint(FVector2D(X, Y)));
	}

	// �����������ߣ�ʹ�� Points �����еĵ㣬������ָ���Ĳ㼶�ϡ�
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points);

	// �ػ��ƵĲ㼶���Ա�ؼ�ϵͳ֪�����ĸ��㼶�ϻ��ƴ˿ؼ���
	return LayerId;
}

FVector2D SCustomNDWidget::ComputeDesiredSize(float) const
{
	// С������������С
	return FVector2D(200.0, 200.0);
}

FReply SCustomNDWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// ����Ƿ����ִ������transaction�� ȷ��û�����ڽ�������GIsTransacting��
	if (GEditor && GEditor->CanTransact() && ensure(!GIsTransacting))
	{
		GEditor->BeginTransaction(TEXT(""), INVTEXT("Edit Normal Distribution"), nullptr);
	}
	// FReply::Handled() ��ʾ���������¼��ѱ�����CaptureMouse(SharedThis(this)) ��ʾ����������룬�����䷢�͵������С����ʵ����
	return FReply::Handled().CaptureMouse(SharedThis(this));
}

FReply SCustomNDWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (GEditor)
	{
		// ��������ǰ�ı༭����
		GEditor->EndTransaction();
	}
	// ReleaseMouseCapture() ��ʾ�ͷŶ����Ĳ��񣬼����ٽ���������롣
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SCustomNDWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	/*������ƶ�ʱ�������λ��ת��Ϊ��׼������ϵ�µ�ֵ����������Щֵ������Ӧ���¼���ͨ�����ڸ�����̬�ֲ��ľ�ֵ�ͱ�׼��Ȳ�����*/

	// ����Ƿ��Ѿ���������ꡣ���û�в����򷵻� FReply::Unhandled()����ʾ�¼�δ������
	if (!HasMouseCapture()) return FReply::Unhandled();
	// ��ȡ��ǰ�ؼ��ı任��Ϣ����������� GetPointsTransform ��������ȡһ�� 2D �任���� PointsTransform������ת�����ꡣ
	const FTransform2D PointsTransform = GetPointsTransform(MyGeometry);
	// ����굱ǰλ�ô���Ļ����ϵת��Ϊ�ؼ��ľֲ�����ϵ���õ� LocalPosition��������ڿؼ��ڲ������λ�á�
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	// ���ֲ�����ϵ�µ�λ�� LocalPosition ͨ�� PointsTransform ����任ת��Ϊ��׼������ϵ�µ�λ�� NormalizedPosition�����ڱ�ʾ��̬�ֲ��ľ�ֵ�ͱ�׼�
	const FVector2D NormalizedPosition = PointsTransform.Inverse().TransformPoint(LocalPosition);
	if (OnMeanChanged.IsBound())
	{
		// ���뵱ǰ���λ�õ� X ���꣨����̬�ֲ��ľ�ֵ����Ϊ������
		OnMeanChanged.Execute(NormalizedPosition.X);
	}
	if (OnStandardDeviationChanged.IsBound())
	{
		// ����һ�����������ı�׼��ֵ��Ϊ������ȷ����׼����һ������ķ�Χ�ڡ�
		OnStandardDeviationChanged.Execute(FMath::Max(0.025f, FMath::Lerp(0.025f, 0.25f, NormalizedPosition.Y)));
	}
	return FReply::Handled();
}

FTransform2D SCustomNDWidget::GetPointsTransform(const FGeometry& AllottedGeometry) const
{
	// ������һ���߾࣬���СΪ�����伸�������С�ߴ�� 5%������߾ཫ�ں����ļ���������ƫ������
	const double Margin = 0.05 * AllottedGeometry.GetLocalSize().GetMin();
	// ������һ����ά���ű任��������������Ĵ�С���ŵ�һ���µĴ�С������µĴ�С��ԭʼ��СС�������߾�Ĵ�С��
	// ���⣬���� Slate �е� Y ��������������ģ���������ʹ���� -1.0 �� Y ��������˷�������š�
	const FScale2D Scale((AllottedGeometry.GetLocalSize() - 2.0 * Margin) * FVector2D(1.0, -1.0));
	// ��һ�м�����һ��ƽ����������ԭ���ƶ�������������½ǣ�����������һ���߾�Ŀռ䡣
	const FVector2D Translation(Margin, AllottedGeometry.GetLocalSize().Y - Margin);
	// ͨ���������õ������ž����ƽ������������һ����ά�任���� FTransform2D
	return FTransform2D(Scale, Translation);
}
