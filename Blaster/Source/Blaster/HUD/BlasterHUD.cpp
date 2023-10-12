// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlayWidget.h"
#include "AnnouncementWidget.h"
#include "ElimAnnouncement.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	//AddCharacterOverlay(); ��Ϊ��Ϸ�������ٿ�ʼ���
	//AddElimAnnouncement("Player1", "Player2");
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

void ABlasterHUD::AddStateAnnouncement()
{
	OwningPlayerCtr = OwningPlayerCtr ? OwningPlayerCtr : GetOwningPlayerController();
	if (OwningPlayerCtr && AnnouncementClass)
	{
		AnnouncementWidget = CreateWidget<UAnnouncementWidget>(OwningPlayerCtr, AnnouncementClass);
		if (AnnouncementWidget)
		{
			AnnouncementWidget->AddToViewport();
		}
	}
}

void ABlasterHUD::AddElimAnnouncement(FString AttackerName, FString VictimName)
{
	OwningPlayerCtr = OwningPlayerCtr ? OwningPlayerCtr : GetOwningPlayerController();
	if (OwningPlayerCtr && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayerCtr, ElimAnnouncementClass);
		if (ElimAnnouncementWidget)
		{
			ElimAnnouncementWidget->SetElimAnnouncementText(AttackerName, VictimName);
			ElimAnnouncementWidget->AddToViewport();

			for (UElimAnnouncement* Msg : ElimMessages)
			{
				if (Msg && Msg->AnnouncementBox)
				{
					//Canvas Panel Slot ������UE4�е�һ�ֲ��ֲ����ͣ����ڿ����� Canvas Panel �з��õ� Widget �� Canvas �еĲ��ֺ�λ�á�
					//UCanvasPanelSlot ���� UPanelSlot ������࣬���������� Canvas ������ص����ԣ�
					//����λ�á���С������ȡ�ͨ���޸� UCanvasPanelSlot �����ԣ�����ʵ�ֶ� Canvas Panel �и��� Widget �Ĳ��ֺ�λ�ý��о�ȷ���ơ�
					//ʹ�� UCanvasPanelSlot ���͵ı��������ǿ��Ի�ȡ�������� Canvas Panel ���ض� Widget �Ĳ�����Ϣ��
					//		���磬���ǿ���ʹ�� UCanvasPanelSlot ������������ Widget ��λ�á���С�����뷽ʽ�Լ������벼����ص����ԡ�
					//ͨ���� Widget �� Slot ת��Ϊ Canvas Panel Slot ���ͣ����ǿ��Է��ʺ��޸� Canvas Panel Slot ���е����ԣ�
					//��ͨ����Щ���������� Widget �� Canvas Panel �еĲ��֡�
					// 
					//��Canvas Panel�У�Widget ��ͨ�� Canvas Panel Slot ������λ�úʹ�С�ġ����ֱ��ʹ�� Widget��Slot�޸�λ�úʹ�С����������Ч��
					//��ˣ���Canvas Panel�п���Widget��λ�úʹ�С��Ҫ�Ȼ�ȡ��Widget��Ӧ��Canvas Panel Slot����ʹ��Canvas Panel Slot���������޸�λ�úʹ�С�Ȳ�����Ϣ��
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						// ����Ļ�Ϸ��ƶ�
						FVector2D NewPosition(CanvasSlot->GetPosition().X, Position.Y - CanvasSlot->GetPosition().Y);
						CanvasSlot->SetPosition(NewPosition);
					}
				}
			}
			ElimMessages.Add(ElimAnnouncementWidget);

			FTimerHandle ElimmMsgTimer;
			FTimerDelegate ElimDelegate;
			ElimDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinish"), ElimAnnouncementWidget);
			GetWorldTimerManager().SetTimer(ElimmMsgTimer, ElimDelegate, ElimAnnouncementTime, false);
		}
	}
}

void ABlasterHUD::ElimAnnouncementTimerFinish(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}
