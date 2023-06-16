// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		SetHUDHealth(BlasterCharacter->GetCurHealth(), BlasterCharacter->GetMaxHealth());
	}
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;

	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlayWidget &&
		BlasterHUD->CharacterOverlayWidget->HealthBar &&
		BlasterHUD->CharacterOverlayWidget->HealthText;
	if (bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlayWidget->HealthBar->SetPercent(HealthPercent);

		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlayWidget->HealthText->SetText(FText::FromString(HealthText));
	}
}

#define SETHUDTEXT_INT(TextCom, Num) \
BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD; \
bool bHUDValid = BlasterHUD && BlasterHUD->CharacterOverlayWidget && BlasterHUD->CharacterOverlayWidget->TextCom; \
if (bHUDValid) \
{ \
	FString NumText = FString::Printf(TEXT("%d"), Num); \
	BlasterHUD->CharacterOverlayWidget->TextCom->SetText(FText::FromString(NumText)); \
} \

void ABlasterPlayerController::SetHUDScore(float Score)
{
	SETHUDTEXT_INT(ScoreAmountText, FMath::FloorToInt(Score));
	//BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	//bool bHUDValid = BlasterHUD &&
	//	BlasterHUD->CharacterOverlayWidget &&
	//	BlasterHUD->CharacterOverlayWidget->ScoreAmountText;
	//if (bHUDValid)
	//{
	//	FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
	//	BlasterHUD->CharacterOverlayWidget->ScoreAmountText->SetText(FText::FromString(ScoreText));
	//}
}

void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	SETHUDTEXT_INT(DefeatsAmountText, Defeats);
	/*BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlayWidget &&
		BlasterHUD->CharacterOverlayWidget->DefeatsAmountText;
	if (bHUDValid)
	{
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlayWidget->DefeatsAmountText->SetText(FText::FromString(DefeatsText));
	}*/
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	SETHUDTEXT_INT(WeaponAmmoAmountText, Ammo);
	/*BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlayWidget &&
		BlasterHUD->CharacterOverlayWidget->WeaponAmmoAmountText;
	if (bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlayWidget->WeaponAmmoAmountText->SetText(FText::FromString(AmmoText));
	}*/
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	SETHUDTEXT_INT(CarriedAmmoAmountText, Ammo);
}

void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlayWidget &&
		BlasterHUD->CharacterOverlayWidget->MatchCountdownText;
	if (bHUDValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;
		FString ConntdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlayWidget->MatchCountdownText->SetText(FText::FromString(ConntdownText));
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountdown(MatchTime - GetServerTime());
	}
	CountdownInt = SecondsLeft;
}

void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds(); //ds服务器获取自己的时间
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt); //然后回包给客户端
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest; //计算客户端请求的往返时间
	float CurrentServerTime = TimeServerReceivedRequest + (RoundTripTime * 0.5f); //服务器回包的时间+往返时间的一半
	ClientServerDelte = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())
		return GetWorld()->GetTimeSeconds();
	return GetWorld()->GetTimeSeconds() + ClientServerDelte;
}
