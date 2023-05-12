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
		Texture, //Ҫ���Ƶ�����
		TextureDrawPoint.X, TextureDrawPoint.Y, //����Ļ�ϻ�����������Ͻǵ� x �� y ����
		TextureWidth, TextureHeight, //����Ļ�ϻ��Ƶ�����Ŀ�Ⱥ͸߶�
		0.f, 0.f, //ָ���������л��Ƶ���������Ͻǣ��� UV ����Ϊ��λ�������Ͻǣ�0,0����ʾ��������Ͻǣ����½ǣ�1,1����ʾ��������½�
		1.f, 1.f, //ָ���������л��Ƶ�����Ŀ�Ⱥ͸߶ȣ��� UV ����Ϊ��λ����Ĭ��ֵ�� 1������Щ��������ΪС�� 1 �����ֿ���ֻ���Ʋ�������
		FLinearColor::White //Tint ��ɫ������������ɫ����
	);
}
