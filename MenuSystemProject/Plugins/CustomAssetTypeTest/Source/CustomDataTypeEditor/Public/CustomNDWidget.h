// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"

DECLARE_DELEGATE_OneParam(FOnMeanChanged, float /*NewMean*/);
DECLARE_DELEGATE_OneParam(FOnStandardDeviationChanged, float /*NewStandardDeviation*/)

/**
 *
 */
class SCustomNDWidget : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SCustomNDWidget)
		:_Mean(0.5f)
		, _StandardDeviation(0.2f)
	{}

	/*
	*	在 Slate 框架中，SLATE_ATTRIBUTE 宏用于创建控件属性的对象，该对象可以在小部件的声明中使用，
	*	并通过 FArguments 结构体传递给小部件的构造函数。，并且允许这些属性与外部数据源动态绑定。
	*   使用 SLATE_ATTRIBUTE 的主要原因包括：
	*	动态绑定：TAttribute 类型允许属性值与外部数据源进行绑定，这意味着属性的值可以在运行时动态地更新。这种动态绑定使得 UI 控件能够实时反映数据的变化，从而保持界面的同步更新。
	*	类型安全：TAttribute 是类型安全的，它可以确保绑定的属性值与控件期望的类型相匹配。这样可以在编译时捕获到类型不匹配的错误，避免在运行时出现意外的行为或崩溃。
	*	性能优化：TAttribute 使用了一些内部优化，例如延迟求值和缓存机制，以提高性能并减少不必要的计算。这些优化确保了属性的更新效率，并且在大规模 UI 中也能保持良好的性能表现。
	*	与 Slate 框架集成：SLATE_ATTRIBUTE 宏与 Slate 框架紧密集成，提供了一种方便的方式来创建和管理控件的属性。
	*	通过使用这个宏，开发者可以更加简洁地定义控件的属性，并且利用 Slate 框架提供的功能来实现属性的绑定和更新逻辑。
	*/

	SLATE_ATTRIBUTE(float, Mean);
	SLATE_ATTRIBUTE(float, StandardDeviation);
	SLATE_EVENT(FOnMeanChanged, OnMeanChanged);
	SLATE_EVENT(FOnStandardDeviationChanged, OnStandardDeviationChanged);

	SLATE_END_ARGS()


public:
	// 构造SWidget时会调用的函数
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;

	/**
	 * FReply 是 Unreal Engine 中的一个结构体，用于表示 Slate UI 控件的事件处理函数的返回值。它提供了一种机制，用于告知 Slate 框架如何处理事件，并控制事件的传递和处理流程。
	 * Handled（已处理）：表示事件已经被完全处理，并且不需要继续传递给其他控件。在事件处理函数中返回 FReply::Handled() 可以阻止事件继续传递。
	 * Unhandled（未处理）：表示事件未被当前控件处理，并且需要继续传递给其他控件进行处理。在事件处理函数中返回 FReply::Unhandled() 可以将事件传递给父级控件或者其他相关的控件。
	 * CaptureMouse（捕获鼠标）：表示当前控件捕获了鼠标的输入事件，并且将会接收后续的鼠标输入。在事件处理函数中返回 FReply::CaptureMouse() 可以将鼠标输入事件捕获到当前控件。
	 * ReleaseMouse（释放鼠标）：表示当前控件释放了对鼠标的输入事件的捕获。在事件处理函数中返回 FReply::ReleaseMouse() 可以释放当前控件对鼠标的捕获。
	 * ConsumeCursor（消耗鼠标）：表示当前控件消耗了鼠标的输入事件，并且不会将事件传递给其他控件。在事件处理函数中返回 FReply::ConsumeCursor() 可以防止其他控件处理相同的鼠标事件。
	 */

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	FTransform2D GetPointsTransform(const FGeometry& AllottedGeometry) const;

private:
	/*
	* TAttribute 是一个模板类，用于表示一个属性值，它可以是任何类型。这个属性值可以是固定的，也可以是动态的，可以在运行时随时改变。
	* TAttribute 类型通常用于定义 Slate UI 框架中控件的属性，用于创建动态和可交互的用户界面。
	*/

	TAttribute<float> Mean;
	TAttribute<float> StandardDeviation;

	FOnMeanChanged OnMeanChanged;
	FOnStandardDeviationChanged OnStandardDeviationChanged;
};
