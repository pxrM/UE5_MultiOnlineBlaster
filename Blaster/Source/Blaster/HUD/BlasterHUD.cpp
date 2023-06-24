// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlayWidget.h"
#include "AnnouncementWidget.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	//AddCharacterOverlay(); ��Ϊ��Ϸ�������ٿ�ʼ���
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
		Texture, //Ҫ���Ƶ�����
		TextureDrawPoint.X, TextureDrawPoint.Y, //����Ļ�ϻ�����������Ͻǵ� x �� y ����
		TextureWidth, TextureHeight, //����Ļ�ϻ��Ƶ�����Ŀ�Ⱥ͸߶�
		0.f, 0.f, //ָ���������л��Ƶ���������Ͻǣ��� UV ����Ϊ��λ�������Ͻǣ�0,0����ʾ��������Ͻǣ����½ǣ�1,1����ʾ��������½�
		1.f, 1.f, //ָ���������л��Ƶ�����Ŀ�Ⱥ͸߶ȣ��� UV ����Ϊ��λ����Ĭ��ֵ�� 1������Щ��������ΪС�� 1 �����ֿ���ֻ���Ʋ�������
		CrosshairColor //Tint ��ɫ������������ɫ����
	);
}
