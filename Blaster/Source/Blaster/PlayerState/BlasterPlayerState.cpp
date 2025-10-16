// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetWork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, Team);
}

#define CHECK_SET_HUD_TEXT(Func, Num) \
Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character; \
if (Character) \
{ \
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller; \
	if (Controller) \
	{ \
		Controller->Func(Num); \
	} \
} \

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(Score + ScoreAmount);
	CHECK_SET_HUD_TEXT(SetHUDScore, Score); //服务器执行
	/*Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}*/
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();
	CHECK_SET_HUD_TEXT(SetHUDScore, Score);  //客户端执行
	/*Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(Score);
		}
	}*/
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	CHECK_SET_HUD_TEXT(SetHUDDefeats, Defeats);
	/*Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}*/
}

void ABlasterPlayerState::OnRep_Defeats()
{
	CHECK_SET_HUD_TEXT(SetHUDDefeats, Defeats);
	/*Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}*/
}

void ABlasterPlayerState::OnRep_Team()
{
	if (ABlasterCharacter* BCharacter = Cast<ABlasterCharacter>(GetPawn()))
	{
		BCharacter->SetTeamColor(Team);
	}
}

void ABlasterPlayerState::SetTeam(ETeam TeamToSet)
{
	Team = TeamToSet;

	if (ABlasterCharacter* BCharacter = Cast<ABlasterCharacter>(GetPawn()))
	{
		BCharacter->SetTeamColor(Team);
	}
}
