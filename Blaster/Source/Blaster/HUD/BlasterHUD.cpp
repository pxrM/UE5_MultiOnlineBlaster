// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	if (GEngine)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter);
		DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter);
		DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter);
		DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter);
		DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter);
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter)
{
	if (Texture == nullptr)return;

	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(ViewportCenter.X - (TextureWidth / 2.f), ViewportCenter.Y - (TextureHeight / 2.f));

	DrawTexture(
		Texture, //要绘制的纹理
		TextureDrawPoint.X, TextureDrawPoint.Y, //在屏幕上绘制纹理的左上角的 x 和 y 坐标
		TextureWidth, TextureHeight, //在屏幕上绘制的纹理的宽度和高度
		0.f, 0.f, //指定从纹理中绘制的区域的左上角（以 UV 坐标为单位）。左上角（0,0）表示纹理的左上角，右下角（1,1）表示纹理的右下角
		1.f, 1.f, //指定从纹理中绘制的区域的宽度和高度（以 UV 坐标为单位）。默认值是 1。将这些参数设置为小于 1 的数字可以只绘制部分纹理
		FLinearColor::White //Tint 颜色，可以用来着色纹理
	);
}
