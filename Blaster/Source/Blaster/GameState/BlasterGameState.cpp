// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		CurTopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == CurTopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > CurTopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		CurTopScore = ScoringPlayer->GetScore();
	}
}

void ABlasterGameState::RedTeamScores()
{
	++RedTeamScore;
	if (ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::BlueTeamScores()
{
	++BlueTeamScore;
	if (ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	if (ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	if (ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
