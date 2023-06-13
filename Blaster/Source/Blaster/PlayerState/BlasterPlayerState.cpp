// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetWork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}

#define CHECK_SETHUDTEXT(Func, Num) \
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
	CHECK_SETHUDTEXT(SetHUDScore, Score); //������ִ��
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
	CHECK_SETHUDTEXT(SetHUDScore, Score);  //�ͻ���ִ��
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
	CHECK_SETHUDTEXT(SetHUDDefeats, Defeats);
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
	CHECK_SETHUDTEXT(SetHUDDefeats, Defeats);
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