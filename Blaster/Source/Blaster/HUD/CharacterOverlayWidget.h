// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlayWidget.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UCharacterOverlayWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamSocre;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamSocre;

	UPROPERTY(meta = (BindWidget))
	UWidget* TeamHorizontalBox;

	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage;

	//标记为 Transient，它不会被序列化和保存到磁盘上，而是只存在于运行时内存中。
	//这意味着，每次重新启动游戏或重新加载关卡时，该属性都会被重置为默认值。
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnim;
};
