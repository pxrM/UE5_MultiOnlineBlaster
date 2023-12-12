// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Images/SThrobber.h"

class FDeferredCleanupSlateBrush;
struct FLoadingWidgetSettings;
struct FThrobberSettings;

/**
 * Loading Widget base class
 */
class MASYNCLOADINGSCREEN_API SLoadingWidget : public SCompoundWidget
{
public:
	/// <summary>
	/// OnPaint��Slate��������ڻ���С������۵��麯���������̳���SWidget������������أ�����ʵ���Զ���Ļ����߼���
	/// </summary>
	/// <param name="Args"> ��������ʱ����ĸ��ֲ���������Canvas��ʱ����� </param>
	/// <param name="AllottedGeometry"> ��ʾ����Ļ�Ϸ����С�����Ŀռ���Ϣ������λ�á���С�����ű����� </param>
	/// <param name="MyCullingRect"> ��ʾС�����ľ�����������ȷ����ЩԪ����Ҫ������ </param>
	/// <param name="OutDrawElements"> ָ��Ԫ���б�����ã����ڴ洢��Ҫ���Ƶ�Ԫ�� </param>
	/// <param name="LayerId"> ���ڱ�ʶ���Ʋ�νṹ�еĲ��ID </param>
	/// <param name="InWidgetStyle"> ��ǰС��������ʽ </param>
	/// <param name="bParentEnabled"> ��ʾС�����Ƿ����� </param>
	/// <returns></returns>
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;


public:
	/// <summary>
	/// ��ȡһ��throbber������SThrobber::EAnimation ��һ��ö�����ͣ����ܰ����˲�ͬ�Ķ������ͣ�������ת����˸�ȡ�
	/// </summary>
	/// <param name="ThrobberSettings"></param>
	/// <returns></returns>
	SThrobber::EAnimation GetThrobberAnimation(const FThrobberSettings& ThrobberSettings) const;

	/// <summary>
	/// ���� loading icon
	/// </summary>
	/// <param name="Settings"></param>
	void ConstructLoadingIcon(const FLoadingWidgetSettings& Settings);


protected:
	// ��ȡLoadingWidget�Ŀɼ���
	EVisibility GetLoadingWidgetVisibility() const;


protected:
	/// <summary>
	/// Loading Iconռλ
	/// </summary>
	TSharedRef<SWidget> LoadingIcon = SNullWidget::NullWidget;

	/// <summary>
	/// Image slate brush list
	/// FDeferredCleanupSlateBrush ��һ���ṹ�壬���ڴ洢��Ҫ�ӳ������ Slate ͼ����Դ��
	/// </summary>
	TArray<TSharedPtr<FDeferredCleanupSlateBrush>> CleanupBrushList;

	/// <summary>
	/// ���򲥷�ͼ������
	/// </summary>
	bool bPlayReverse = false;

	/// <summary>
	/// ��ǰͼƬ���е�index
	/// </summary>
	mutable int32 ImageIndex = 0;

	/// <summary>
	/// �洢��ǰ�ܵ�����ʱ��
	/// </summary>
	mutable float TotalDeltaTime = 0.0f;

	/// <summary>
	/// ����ָ������ͼ���ʱ����������Ϊ��λ����ֵԽС����Խ�졣0ֵ��ÿ֡����ͼ��
	/// </summary>
	float Interval = 0.05f;
};
