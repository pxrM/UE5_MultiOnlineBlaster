// Fill out your copyright notice in the Description page of Project Settings.


#include "SClassicLayout.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SSafeZone.h"
#include "Widgets/Layout/SDPIScaler.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "LoadingScreenSettings.h"
#include "SBackgroundWidget.h"
#include "STipWidget.h"
#include "SHorizontalLoadingWidget.h"
#include "SVerticalLoadingWidget.h"
#include "SLoadingCompleteText.h"

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SClassicLayout::Construct(const FArguments& InArgs, const FALoadingScreenSettings& Settings, const FClassicLayoutSettings& LayoutSettings)
{
	// SOverlay��Slate���е�һ���������������ڴ���һ�����Ե������������������������ͬһ�������ڷ��ö�����������������ӵ�˳����е�����ʾ��
	// ��SlateUI����У�+ ��������ڽ�������ӵ� SOverlay �����С�����ͨ��ʹ��SOverlay::AddSlot()������ʵ�ֵġ�
	// ��ʹ�� + �����ʱ�����������һ��SOverlay�����Ҳ�������Ҫ��ӵ�SOverlay�еĲ�����
	// ʹ�� + ��������Է���ؽ�����Ӳ�����ӵ� SOverlay �У���ʹ����ʽ���ý�һ�����ò��ֲ�������ʽ��
	TSharedRef<SOverlay> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SBackgroundWidget, Settings.Background)
		];

	// ʹ����SNullWidget::NullWidget��̬��������һ���յ�SWidget��������ת��ΪTSharedRef����ʽ��
	// ����д���������ڳ�������ʱ����Ҫʹ�ü�����Ļ��UI���ʱ��ռλ�����Ա�����Ҫʱ����ؽ����滻��
	// ���Ը���ʵ����Ҫ����"SNullWidget::NullWidget"�滻Ϊ������SWidget������Զ����UI�����
	TSharedRef<SWidget> LoadingWidget = SNullWidget::NullWidget;
	if (Settings.LoadingWidget.LoadingWidgetType == ELoadingWidgetType::LWT_Horizontal)
	{
		LoadingWidget = SNew(SHorizontalLoadingWidget, Settings.LoadingWidget);
	}
	else
	{
		LoadingWidget = SNew(SVerticalLoadingWidget, Settings.LoadingWidget);
	}

	TSharedRef<SHorizontalBox> HorizontalBox = SNew(SHorizontalBox);
	// ���LoadingWidget����� Loading -> Space ->  Tip
	if (LayoutSettings.bIsLoadingWidgetAtLeft)
	{
		// ������������LoadingWidget
		HorizontalBox.Get().AddSlot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			.AutoWidth()
			[
				LoadingWidget
			];

		// ���һ����ȵļ���ؼ�
		HorizontalBox.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
					.Size(FVector2D(LayoutSettings.Space, 0.0f))
			];

		// ���ұ����tip
		// ��ʾ��ӵĲ�۵Ŀ�Ƚ�������п��ÿռ䣬ʹSTipWidget �ؼ�����ˮƽ����Ӧ���������������Ӧˮƽ���ӵĴ�С�仯��
		HorizontalBox.Get().AddSlot()
			.FillWidth(1.0f)
			.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
			[
				SNew(STipWidget, Settings.TipWidget)
			];
	}
	else // tip -> space -> loading
	{
		HorizontalBox.Get().AddSlot()
			.FillWidth(1.0f)
			.HAlign(LayoutSettings.TipAlignment.HorizontalAlignment)
			.VAlign(LayoutSettings.TipAlignment.VerticalAlignment)
			[
				SNew(STipWidget, Settings.TipWidget)
			];

		HorizontalBox.Get().AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.AutoWidth()
			[
				SNew(SSpacer)
					.Size(FVector2D(LayoutSettings.Space, 0.0f))
			];

		HorizontalBox.Get().AddSlot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			.AutoWidth()
			[
				LoadingWidget
			];
	}

	// ����widgetΪ��ֱ����
	EVerticalAlignment VerticalAlignment;
	if (LayoutSettings.bIsWidgetAtBottom)
	{
		VerticalAlignment = EVerticalAlignment::VAlign_Bottom;
	}
	else
	{
		VerticalAlignment = EVerticalAlignment::VAlign_Top;
	}

	Root->AddSlot()
		.HAlign(LayoutSettings.BorderHorizontalAlignment)
		.VAlign(VerticalAlignment)
		[
			// SBorder�ؼ������ڴ��������߿�ľ����������������ڰ��������ؼ�����Ϊ�����ṩ�߿���ʽ�ͱ�����ʽ��
			// .BorderImage() ����һ��ͼƬ��Դ��������Ϊ���������ڿؼ���Χ��ʾ��ͼ����Ϊ�߿�
			// .BorderBackgroundColor��ʾSBorder�ؼ��ı߿򱳾���ɫ�ǰ�ɫ
			SNew(SBorder)
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				.BorderImage(&LayoutSettings.BorderBackground)
				.BorderBackgroundColor(FLinearColor::White)
				[
					// ����ȷ�����������Ļ�İ�ȫ�����ڽ��в��֡��ڲ�ͬ���豸�ͷֱ����£���Ļ�Ͽ��ܴ���һЩ��Ϊ"����ȫ����"������
					// ������Ļ��Ե���ܻᱻ�ڵ�����ʾ��������SSafeZone���԰���������Ա����� UI ʱ���ǵ���Щ����ȫ���򣬲������������ڰ�ȫ�����ڡ�
					// .IsTitleSafe(true) ��ʾʹ�ñ�׼�ġ����ⰲȫ���򡱣�ȷ�����ݲ��ᱻ�ü����ڵ���
					SNew(SSafeZone)
						.HAlign(HAlign_Fill)
						.VAlign(VAlign_Fill)
						.IsTitleSafe(true)
						.Padding(LayoutSettings.BorderPadding)
						[
							// ���ڸ����豸�� DPI ���ű������������������
							// DPI��ÿӢ���������ָ��Ļ��ÿӢ����ʾ��������������ͬ���豸���в�ͬ�� DPI ֵ
							// SDPIScaler �ؼ�����һ���ӿؼ����������ӿؼ��Ĵ�С���Ե�ǰ�豸�������ܶ����ӣ��Ӷ�ʵ���Զ����š�
							SNew(SDPIScaler)
								.DPIScale(this, &SClassicLayout::GetDPIScale)
								[
									HorizontalBox
								]
						]
				]
		];

	// ������ã��������������ı�
	if (Settings.bShowLoadingCompleteText)
	{
		Root->AddSlot()
			.HAlign(Settings.LoadingCompleteTextSettings.Alignment.HorizontalAlignment)
			.VAlign(Settings.LoadingCompleteTextSettings.Alignment.VerticalAlignment)
			.Padding(Settings.LoadingCompleteTextSettings.Padding)
			[
				SNew(SLoadingCompleteText, Settings.LoadingCompleteTextSettings)
			];
	}

	ChildSlot
		[
			Root
		];

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION
