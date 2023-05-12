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

		const float SpreadScaled = MaxCrosshairSpread * HUDPackage.CrosshairSpread;
		DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, FVector2D(0.f, 0.f));
		DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, FVector2D(-SpreadScaled, 0.f));
		DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, FVector2D(SpreadScaled, 0.f));
		DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, FVector2D(0.f, -SpreadScaled));
		DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, FVector2D(0.f, SpreadScaled));
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
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
		FLinearColor::White //Tint ��ɫ������������ɫ����
	);
}
