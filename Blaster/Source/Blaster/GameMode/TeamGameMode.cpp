// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"

ATeamGameMode::ATeamGameMode()
{
	bTeamsMatch = true;
}

void ATeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// 玩家登录・中途加入
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState == nullptr) return;
	
	ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
	if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
	{
		if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
		{
			BGameState->RedTeam.AddUnique(BPState);
			BPState->SetTeam(ETeam::ET_RedTeam);
		}
		else
		{
			BGameState->BlueTeam.AddUnique(BPState);
			BPState->SetTeam(ETeam::ET_BlueTeam);
		}
	}
}

void ATeamGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// 玩家退出
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState == nullptr) return;

	if (ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>())
	{
		if (BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}
}

void ATeamGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	// 比赛开始时
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState == nullptr) return;
	
	for (auto PState : BGameState->PlayerArray)
	{
		ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

float ATeamGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	const ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	const ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();
	if (AttackerPState == nullptr || VictimPState == nullptr)
		return BaseDamage;
	if (AttackerPState == VictimPState)
		return BaseDamage;
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
		return 0.f;
	return BaseDamage;
}

void ATeamGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	// 添加队伍得分
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	const ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BGameState->RedTeamScores();
		}
		else if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BGameState->BlueTeamScores();
		}
	}
}

