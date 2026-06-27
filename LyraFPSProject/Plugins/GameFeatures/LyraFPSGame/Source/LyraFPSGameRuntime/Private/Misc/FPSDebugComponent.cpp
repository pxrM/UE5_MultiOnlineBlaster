// Fill out your copyright notice in the Description page of Project Settings.

#include "Misc/FPSDebugComponent.h"

#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetSystemLibrary.h"

UFPSDebugComponent::UFPSDebugComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UFPSDebugComponent::GetNetworkDebugValues(float& PingMilliseconds, float& InLossPercent, float& OutLossPercent, bool bUseAverageLoss) const
{
	PingMilliseconds = 0.0f;
	InLossPercent = 0.0f;
	OutLossPercent = 0.0f;

	const APlayerController* PlayerController = GetOwnerPlayerController();
	if (!PlayerController)
	{
		return false;
	}

	if (const APlayerState* PlayerState = PlayerController->PlayerState)
	{
		PingMilliseconds = PlayerState->GetPingInMilliseconds();
	}

	const UNetConnection* NetConnection = GetOwnerNetConnection(PlayerController);
	if (!NetConnection)
	{
		return false;
	}

	const UNetConnection::FNetConnectionPacketLoss& InLoss = NetConnection->GetInLossPercentage();
	const UNetConnection::FNetConnectionPacketLoss& OutLoss = NetConnection->GetOutLossPercentage();

	InLossPercent = (bUseAverageLoss ? InLoss.GetAvgLossPercentage() : InLoss.GetLossPercentage()) * 100.0f;
	OutLossPercent = (bUseAverageLoss ? OutLoss.GetAvgLossPercentage() : OutLoss.GetLossPercentage()) * 100.0f;
	return true;
}

void UFPSDebugComponent::PrintNetworkDebugString(bool bUseAverageLoss, float Duration) const
{
	float PingMilliseconds = 0.0f;
	float InLossPercent = 0.0f;
	float OutLossPercent = 0.0f;
	const bool bHasConnection = GetNetworkDebugValues(PingMilliseconds, InLossPercent, OutLossPercent, bUseAverageLoss);

	const FString Message = FString::Printf(
		TEXT("Ping: %.0f ms | Loss In: %.1f%% Out: %.1f%% | Conn: %s"),
		PingMilliseconds,
		InLossPercent,
		OutLossPercent,
		bHasConnection ? TEXT("Valid") : TEXT("None"));

	UKismetSystemLibrary::PrintString(this, Message, true, true, FLinearColor::Green, Duration);
}

APlayerController* UFPSDebugComponent::GetOwnerPlayerController() const
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	return OwnerPawn ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
}

UNetConnection* UFPSDebugComponent::GetOwnerNetConnection(const APlayerController* PlayerController) const
{
	if (PlayerController && PlayerController->Player)
	{
		if (UNetConnection* PlayerConnection = Cast<UNetConnection>(PlayerController->Player))
		{
			return PlayerConnection;
		}
	}

	UWorld* World = GetWorld();
	UNetDriver* NetDriver = World ? World->GetNetDriver() : nullptr;
	return NetDriver ? NetDriver->ServerConnection : nullptr;
}
