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

	//AddCharacterOverlay(); 改为游戏进行中再开始添加
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
		Texture, //要绘制的纹理
		TextureDrawPoint.X, TextureDrawPoint.Y, //在屏幕上绘制纹理的左上角的 x 和 y 坐标
		TextureWidth, TextureHeight, //在屏幕上绘制的纹理的宽度和高度
		0.f, 0.f, //指定从纹理中绘制的区域的左上角（以 UV 坐标为单位）。左上角（0,0）表示纹理的左上角，右下角（1,1）表示纹理的右下角
		1.f, 1.f, //指定从纹理中绘制的区域的宽度和高度（以 UV 坐标为单位）。默认值是 1。将这些参数设置为小于 1 的数字可以只绘制部分纹理
		CrosshairColor //Tint 颜色，可以用来着色纹理
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
					//Canvas Panel Slot 类型是UE4中的一种布局槽类型，用于控制在 Canvas Panel 中放置的 Widget 在 Canvas 中的布局和位置。
					//UCanvasPanelSlot 类是 UPanelSlot 类的子类，它包含了与 Canvas 布局相关的属性，
					//例如位置、大小、对齐等。通过修改 UCanvasPanelSlot 的属性，可以实现对 Canvas Panel 中各个 Widget 的布局和位置进行精确控制。
					//使用 UCanvasPanelSlot 类型的变量，我们可以获取和设置在 Canvas Panel 中特定 Widget 的布局信息。
					//		例如，我们可以使用 UCanvasPanelSlot 的属性来设置 Widget 的位置、大小、对齐方式以及其他与布局相关的属性。
					//通过将 Widget 的 Slot 转换为 Canvas Panel Slot 类型，我们可以访问和修改 Canvas Panel Slot 特有的属性，
					//并通过这些属性来控制 Widget 在 Canvas Panel 中的布局。
					// 
					//在Canvas Panel中，Widget 是通过 Canvas Panel Slot 来控制位置和大小的。如果直接使用 Widget的Slot修改位置和大小，将不会生效。
					//因此，在Canvas Panel中控制Widget的位置和大小需要先获取该Widget对应的Canvas Panel Slot，并使用Canvas Panel Slot的属性来修改位置和大小等布局信息。
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
					if (CanvasSlot)
					{
						FVector2D Position = CanvasSlot->GetPosition();
						// 向屏幕上方移动
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
