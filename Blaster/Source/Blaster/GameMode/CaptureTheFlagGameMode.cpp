// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "Blaster/Weapon/Flag.h"
#include "Blaster/GaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"

void ACaptureTheFlagGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

void ACaptureTheFlagGameMode::FlagCaptured(const AFlag* Flag, const AFlagZone* Zone) const
{
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	if (!bValidCapture) return;
	
	if (ABlasterGameState* BGameState = Cast<ABlasterGameState>(GameState))
	{
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
	}
}
