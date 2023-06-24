// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlayWidget.h"
#include "AnnouncementWidget.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	//AddCharacterOverlay(); 改为游戏进行中再开始添加
}

void ABlasterHUD::AddCharacterOverlay()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && CharacterOverlayClass)
	{
		CharacterOverlayWidget = CreateWidget<UCharacterOverlayWidget>(PlayerController, CharacterOverlayClass);
		if (CharacterOverlayWidget)
		{
			CharacterOverlayWidget->AddToViewport();
		}
	}
}

void ABlasterHUD::AddAnnouncement()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (PlayerController && AnnouncementClass)
	{
		AnnouncementWidget = CreateWidget<UAnnouncementWidget>(PlayerController, AnnouncementClass);
		if (AnnouncementWidget)
		{
			AnnouncementWidget->AddToViewport();
		}
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	if (GEngine)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		const float SpreadScaled = MaxCrosshairSpread * HUDPackage.CrosshairSpread;
		DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2D(0.f, 0.f), HUDPackage.CrosshairColor);
		DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, FVector2D(-SpreadScaled, 0.f), HUDPackage.CrosshairColor);
		DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, FVector2D(SpreadScaled, 0.f), HUDPackage.CrosshairColor);
		DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, FVector2D(0.f, -SpreadScaled), HUDPackage.CrosshairColor);
		DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, FVector2D(0.f, SpreadScaled), HUDPackage.CrosshairColor);
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	if (Texture == nullptr)return;

	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture, //要绘制的纹理
		TextureDrawPoint.X, TextureDrawPoint.Y, //在屏幕上绘制纹理的左上角的 x 和 y 坐标
		TextureWidth, TextureHeight, //在屏幕上绘制的纹理的宽度和高度
		0.f, 0.f, //指定从纹理中绘制的区域的左上角（以 UV 坐标为单位）。左上角（0,0）表示纹理的左上角，右下角（1,1）表示纹理的右下角
		1.f, 1.f, //指定从纹理中绘制的区域的宽度和高度（以 UV 坐标为单位）。默认值是 1。将这些参数设置为小于 1 的数字可以只绘制部分纹理
		CrosshairColor //Tint 颜色，可以用来着色纹理
	);
}
