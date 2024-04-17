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
	/*用于绘制自定义 Slate 控件的函数*/

	// 定义了要绘制的曲线上的点的数量。
	const int32 NumPoints = 512;
	// 创建了一个二维向量数组 Points，用于存储要绘制的曲线上的点，并为其预留了足够的空间。
	TArray<FVector2D> Points;
	Points.Reserve(NumPoints);

	// 获取了用于将曲线上的点从局部坐标系转换到控件坐标系的二维变换矩阵 PointsTransform
	const FTransform2D PointsTransform = GetPointsTransform(AllottedGeometry);

	for (int32 PointIndex = 0; PointIndex < NumPoints; ++PointIndex)
	{
		// 计算当前点在曲线上的 X 坐标，范围从 0 到 1。
		const float X = PointIndex / (NumPoints - 1.0);
		// 计算标准化的 X 坐标，通过将当前点的 X 坐标减去平均值，并除以标准差。
		const float D = (X - Mean.Get()) / StandardDeviation.Get();
		// 根据高斯分布函数计算当前点在曲线上的 Y 坐标。
		const float Y = FMath::Exp(-0.5f * D * D);

		Points.Add(PointsTransform.TransformPoint(FVector2D(X, Y)));
	}

	// 函数绘制曲线，使用 Points 数组中的点，绘制在指定的层级上。
	FSlateDrawElement::MakeLines(OutDrawElements, LayerId, AllottedGeometry.ToPaintGeometry(), Points);

	// 回绘制的层级，以便控件系统知道在哪个层级上绘制此控件。
	return LayerId;
}

FVector2D SCustomNDWidget::ComputeDesiredSize(float) const
{
	// 小部件的期望大小
	return FVector2D(200.0, 200.0);
}

FReply SCustomNDWidget::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// 检查是否可以执行事务（transaction） 确保没有正在进行事务（GIsTransacting）
	if (GEditor && GEditor->CanTransact() && ensure(!GIsTransacting))
	{
		GEditor->BeginTransaction(TEXT(""), INVTEXT("Edit Normal Distribution"), nullptr);
	}
	// FReply::Handled() 表示告诉引擎事件已被处理，CaptureMouse(SharedThis(this)) 表示捕获鼠标输入，并将其发送到共享的小部件实例。
	return FReply::Handled().CaptureMouse(SharedThis(this));
}

FReply SCustomNDWidget::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (GEditor)
	{
		// 来结束当前的编辑事务
		GEditor->EndTransaction();
	}
	// ReleaseMouseCapture() 表示释放对鼠标的捕获，即不再接收鼠标输入。
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SCustomNDWidget::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	/*在鼠标移动时，将鼠标位置转换为标准化坐标系下的值，并根据这些值触发相应的事件，通常用于更新正态分布的均值和标准差等参数。*/

	// 检查是否已经捕获了鼠标。如果没有捕获，则返回 FReply::Unhandled()，表示事件未被处理。
	if (!HasMouseCapture()) return FReply::Unhandled();
	// 获取当前控件的变换信息，这里调用了 GetPointsTransform 函数来获取一个 2D 变换矩阵 PointsTransform，用于转换坐标。
	const FTransform2D PointsTransform = GetPointsTransform(MyGeometry);
	// 将鼠标当前位置从屏幕坐标系转换为控件的局部坐标系，得到 LocalPosition，即鼠标在控件内部的相对位置。
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	// 将局部坐标系下的位置 LocalPosition 通过 PointsTransform 的逆变换转换为标准化坐标系下的位置 NormalizedPosition，用于表示正态分布的均值和标准差。
	const FVector2D NormalizedPosition = PointsTransform.Inverse().TransformPoint(LocalPosition);
	if (OnMeanChanged.IsBound())
	{
		// 传入当前鼠标位置的 X 坐标（即正态分布的均值）作为参数。
		OnMeanChanged.Execute(NormalizedPosition.X);
	}
	if (OnStandardDeviationChanged.IsBound())
	{
		// 传入一个经过调整的标准差值作为参数，确保标准差在一个合理的范围内。
		OnStandardDeviationChanged.Execute(FMath::Max(0.025f, FMath::Lerp(0.025f, 0.25f, NormalizedPosition.Y)));
	}
	return FReply::Handled();
}

FTransform2D SCustomNDWidget::GetPointsTransform(const FGeometry& AllottedGeometry) const
{
	// 计算了一个边距，其大小为所分配几何体的最小尺寸的 5%。这个边距将在后续的计算中用作偏移量。
	const double Margin = 0.05 * AllottedGeometry.GetLocalSize().GetMin();
	// 计算了一个二维缩放变换矩阵。它将几何体的大小缩放到一个新的大小，这个新的大小比原始大小小了两倍边距的大小。
	// 另外，由于 Slate 中的 Y 轴向下是正方向的，所以这里使用了 -1.0 对 Y 方向进行了反向的缩放。
	const FScale2D Scale((AllottedGeometry.GetLocalSize() - 2.0 * Margin) * FVector2D(1.0, -1.0));
	// 这一行计算了一个平移向量，将原点移动到几何体的左下角，并且留出了一个边距的空间。
	const FVector2D Translation(Margin, AllottedGeometry.GetLocalSize().Y - Margin);
	// 通过上面计算得到的缩放矩阵和平移向量构造了一个二维变换矩阵 FTransform2D
	return FTransform2D(Scale, Translation);
}
