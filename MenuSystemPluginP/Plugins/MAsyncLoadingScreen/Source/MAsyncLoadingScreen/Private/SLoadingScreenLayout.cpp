// Fill out your copyright notice in the Description page of Project Settings.


#include "SLoadingScreenLayout.h"
#include "Engine/UserInterfaceSettings.h"


float SLoadingScreenLayout::PointSizeToSlateUnits(float PointSize)
{
	// Slate��ܲ��õ�ˮƽ�ֱ���DPI��ͨ�������Ϊ96.0��
	const float SlateFreeTypeHorizontalResolutionDPI = 96.f;
	// FreeType������Ⱦ�����ԭ��DPI��ͨ�������Ϊ72.0��
	const float FreeTypeNativeDPI = 72.0f;
	// ��ˮƽ�ֱ���DPI��SlateFreeTypeHorizontalResolutionDPI������FreeType������Ⱦ�����ԭ��DPI��FreeTypeNativeDPI�����õ�һ���������ӡ�
	// ���������ӳ��Ե��С��PointSize�����õ����ش�С��PixelSize����
	const float PixelSize = PointSize * (SlateFreeTypeHorizontalResolutionDPI / FreeTypeNativeDPI);
	return PixelSize;
}

float SLoadingScreenLayout::GetDPIScale() const
{
	/*
		TickSpaceGeometry ��UE�е�һ��������������ڱ�ʾWidget�Ļ��ƿռ伸����Ϣ��
		ToPaintGeometry �����������ǽ����ƿռ伸���壨TickSpaceGeometry��ת��Ϊ��Ļ�ϵĻ��Ƽ����壨PaintGeometry����
		GetLocalSize()�����ػ��Ƽ�����ı��ش�С�������ص�λ�µĿ�Ⱥ͸߶ȡ�
	*/
	const FVector2D DrawSize = GetTickSpaceGeometry().ToPaintGeometry().GetLocalSize();
	// ���ش�Сת��Ϊ FIntPoint ���͵Ķ��� Size��
	const FIntPoint Size((int32)DrawSize.X, (int32)DrawSize.Y);
	// ����UUserInterfaceSettings��GetDPIScaleBasedOnSize()�������÷������ݴ�������ش�С�������Ӧ�� DPI ���ű�����
	return GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(Size);
}
