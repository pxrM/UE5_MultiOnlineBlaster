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
	*	�� Slate ����У�SLATE_ATTRIBUTE �����ڴ����ؼ����ԵĶ��󣬸ö��������С������������ʹ�ã�
	*	��ͨ�� FArguments �ṹ�崫�ݸ�С�����Ĺ��캯����������������Щ�������ⲿ����Դ��̬�󶨡�
	*   ʹ�� SLATE_ATTRIBUTE ����Ҫԭ�������
	*	��̬�󶨣�TAttribute ������������ֵ���ⲿ����Դ���а󶨣�����ζ�����Ե�ֵ����������ʱ��̬�ظ��¡����ֶ�̬��ʹ�� UI �ؼ��ܹ�ʵʱ��ӳ���ݵı仯���Ӷ����ֽ����ͬ�����¡�
	*	���Ͱ�ȫ��TAttribute �����Ͱ�ȫ�ģ�������ȷ���󶨵�����ֵ��ؼ�������������ƥ�䡣���������ڱ���ʱ�������Ͳ�ƥ��Ĵ��󣬱���������ʱ�����������Ϊ�������
	*	�����Ż���TAttribute ʹ����һЩ�ڲ��Ż��������ӳ���ֵ�ͻ�����ƣ���������ܲ����ٲ���Ҫ�ļ��㡣��Щ�Ż�ȷ�������Եĸ���Ч�ʣ������ڴ��ģ UI ��Ҳ�ܱ������õ����ܱ��֡�
	*	�� Slate ��ܼ��ɣ�SLATE_ATTRIBUTE ���� Slate ��ܽ��ܼ��ɣ��ṩ��һ�ַ���ķ�ʽ�������͹���ؼ������ԡ�
	*	ͨ��ʹ������꣬�����߿��Ը��Ӽ��ض���ؼ������ԣ��������� Slate ����ṩ�Ĺ�����ʵ�����Եİ󶨺͸����߼���
	*/

	SLATE_ATTRIBUTE(float, Mean);
	SLATE_ATTRIBUTE(float, StandardDeviation);
	SLATE_EVENT(FOnMeanChanged, OnMeanChanged);
	SLATE_EVENT(FOnStandardDeviationChanged, OnStandardDeviationChanged);

	SLATE_END_ARGS()


public:
	// ����SWidgetʱ����õĺ���
	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float) const override;

	/**
	 * FReply �� Unreal Engine �е�һ���ṹ�壬���ڱ�ʾ Slate UI �ؼ����¼��������ķ���ֵ�����ṩ��һ�ֻ��ƣ����ڸ�֪ Slate �����δ����¼����������¼��Ĵ��ݺʹ������̡�
	 * Handled���Ѵ�������ʾ�¼��Ѿ�����ȫ�������Ҳ���Ҫ�������ݸ������ؼ������¼��������з��� FReply::Handled() ������ֹ�¼��������ݡ�
	 * Unhandled��δ��������ʾ�¼�δ����ǰ�ؼ�����������Ҫ�������ݸ������ؼ����д������¼��������з��� FReply::Unhandled() ���Խ��¼����ݸ������ؼ�����������صĿؼ���
	 * CaptureMouse��������꣩����ʾ��ǰ�ؼ����������������¼������ҽ�����պ�����������롣���¼��������з��� FReply::CaptureMouse() ���Խ���������¼����񵽵�ǰ�ؼ���
	 * ReleaseMouse���ͷ���꣩����ʾ��ǰ�ؼ��ͷ��˶����������¼��Ĳ������¼��������з��� FReply::ReleaseMouse() �����ͷŵ�ǰ�ؼ������Ĳ���
	 * ConsumeCursor��������꣩����ʾ��ǰ�ؼ����������������¼������Ҳ��Ὣ�¼����ݸ������ؼ������¼��������з��� FReply::ConsumeCursor() ���Է�ֹ�����ؼ�������ͬ������¼���
	 */

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	FTransform2D GetPointsTransform(const FGeometry& AllottedGeometry) const;

private:
	/*
	* TAttribute ��һ��ģ���࣬���ڱ�ʾһ������ֵ�����������κ����͡��������ֵ�����ǹ̶��ģ�Ҳ�����Ƕ�̬�ģ�����������ʱ��ʱ�ı䡣
	* TAttribute ����ͨ�����ڶ��� Slate UI ����пؼ������ԣ����ڴ�����̬�Ϳɽ������û����档
	*/

	TAttribute<float> Mean;
	TAttribute<float> StandardDeviation;

	FOnMeanChanged OnMeanChanged;
	FOnStandardDeviationChanged OnStandardDeviationChanged;
};
