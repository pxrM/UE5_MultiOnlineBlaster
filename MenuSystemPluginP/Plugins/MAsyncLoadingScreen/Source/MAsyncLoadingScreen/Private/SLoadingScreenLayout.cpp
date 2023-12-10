// Fill out your copyright notice in the Description page of Project Settings.


#include "SLoadingScreenLayout.h"
#include "Engine/UserInterfaceSettings.h"


float SLoadingScreenLayout::PointSizeToSlateUnits(float PointSize)
{
	// Slate框架采用的水平分辨率DPI，通常情况下为96.0。
	const float SlateFreeTypeHorizontalResolutionDPI = 96.f;
	// FreeType字体渲染引擎的原生DPI，通常情况下为72.0。
	const float FreeTypeNativeDPI = 72.0f;
	// 将水平分辨率DPI（SlateFreeTypeHorizontalResolutionDPI）除以FreeType字体渲染引擎的原生DPI（FreeTypeNativeDPI），得到一个比例因子。
	// 将比例因子乘以点大小（PointSize），得到像素大小（PixelSize）。
	const float PixelSize = PointSize * (SlateFreeTypeHorizontalResolutionDPI / FreeTypeNativeDPI);
	return PixelSize;
}

float SLoadingScreenLayout::GetDPIScale() const
{
	/*
		TickSpaceGeometry 是UE中的一个几何体对象，用于表示Widget的绘制空间几何信息。
		ToPaintGeometry 方法的作用是将绘制空间几何体（TickSpaceGeometry）转换为屏幕上的绘制几何体（PaintGeometry）。
		GetLocalSize()：返回绘制几何体的本地大小，即像素单位下的宽度和高度。
	*/
	const FVector2D DrawSize = GetTickSpaceGeometry().ToPaintGeometry().GetLocalSize();
	// 本地大小转换为 FIntPoint 类型的对象 Size。
	const FIntPoint Size((int32)DrawSize.X, (int32)DrawSize.Y);
	// 调用UUserInterfaceSettings的GetDPIScaleBasedOnSize()方法，该方法根据传入的像素大小计算出对应的 DPI 缩放比例。
	return GetDefault<UUserInterfaceSettings>()->GetDPIScaleBasedOnSize(Size);
}
