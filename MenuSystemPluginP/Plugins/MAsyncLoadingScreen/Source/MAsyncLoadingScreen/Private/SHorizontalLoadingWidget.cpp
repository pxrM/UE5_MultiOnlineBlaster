// Fill out your copyright notice in the Description page of Project Settings.


#include "SHorizontalLoadingWidget.h"
#include "SlateOptMacros.h"
#include "LoadingScreenSettings.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Text/STextBlock.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SHorizontalLoadingWidget::Construct(const FArguments& InArgs, const FLoadingWidgetSettings& Settings)
{
	bPlayReverse = Settings.ImageSequenceSettings.bPlayReverse;

	TSharedRef<SHorizontalBox> Root = SNew(SHorizontalBox);

	ConstructLoadingIcon(Settings);

	EVisibility LoadingTextVisibility;

	if (Settings.LoadingText.IsEmpty())
	{
		LoadingTextVisibility = EVisibility::Collapsed;
	}
	else
	{
		LoadingTextVisibility = EVisibility::SelfHitTestInvisible;
	}

	// ��������ı����ұ�	icon -> Space - > text
	if (Settings.bLoadingTextRightPosition)
	{
		// ����Get()��ȡRoot��������ã�������AddSlot()�����һ���µĲ�ۣ�Slot������ڶ����������з����ӿؼ��ķ�ʽ����ʽ��
		// .AutoWidth() ��ʾ��۵Ŀ�Ƚ��������������Զ�������
		Root.Get().AddSlot()
			.HAlign(Settings.LoadingIconAlignment.HorizontalAlignment)
			.VAlign(Settings.LoadingIconAlignment.VerticalAlignment)
			.AutoWidth()
			[
				LoadingIcon
			];

		// ���һ����ȵļ���ؼ�
		Root.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
					.Size(FVector2D(Settings.Space, 0.0f))
			];

		// ������ұ����loading text
		Root.Get().AddSlot()
			.HAlign(Settings.TextAlignment.HorizontalAlignment)
			.VAlign(Settings.TextAlignment.VerticalAlignment)
			.AutoWidth()
			[
				SNew(STextBlock)
					.Visibility(LoadingTextVisibility)
					.ColorAndOpacity(Settings.Appearance.ColorAndOpacity)
					.Font(Settings.Appearance.Font)
					.ShadowOffset(Settings.Appearance.ShadowOffset)
					.ShadowColorAndOpacity(Settings.Appearance.ShadowColorAndOpacity)
					.Justification(Settings.Appearance.Justification)
					.Text(Settings.LoadingText)
			];
	}
	else//��������ı������	text -> Space - > icon
	{
		// �����������Ӽ����ı�
		Root.Get().AddSlot()
			.HAlign(Settings.TextAlignment.HorizontalAlignment)
			.VAlign(Settings.TextAlignment.VerticalAlignment)
			.AutoWidth()
			[
				SNew(STextBlock)
					.Visibility(LoadingTextVisibility)
					.ColorAndOpacity(Settings.Appearance.ColorAndOpacity)
					.Font(Settings.Appearance.Font)
					.ShadowOffset(Settings.Appearance.ShadowOffset)
					.ShadowColorAndOpacity(Settings.Appearance.ShadowColorAndOpacity)
					.Justification(Settings.Appearance.Justification)
					.Text(Settings.LoadingText)
			];

		// ���һ����ȵļ���ؼ�
		Root.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
					.Size(FVector2D(Settings.Space, 0.0f))
			];

		// ��������ұ����loading icon
		Root.Get().AddSlot()
			.HAlign(Settings.LoadingIconAlignment.HorizontalAlignment)
			.VAlign(Settings.LoadingIconAlignment.VerticalAlignment)
			.AutoWidth()
			[
				LoadingIcon
			];
	}

	// ChildSlot��ʾ��ǰWidget���ӿؼ���ۣ�ʹ��[ ]�������Root�ؼ���Ϊ�ӿؼ���ӵ��ò���С�
	ChildSlot
	[
		Root
	];
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION