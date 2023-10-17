// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/GameState/BlasterGameState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");	//比赛时间已结束，显示获胜者并开始冷却倒计时
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true; //游戏模式将保持在等待开始状态，GameMode会为每个玩家生成默认的Pawn（只能飞来飞去），直到手动调用StartMatch()
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = WarmupTime + MatchTime + CooldownTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			BlasterPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (AttackPlayerState && AttackPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> PlayerCurrentlyInTheLead;
		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayerCurrentlyInTheLead.Add(LeadPlayer);
		}

		AttackPlayerState->AddToScore(1.0f);
		BlasterGameState->UpdateTopScore(AttackPlayerState);

		if (BlasterGameState->TopScoringPlayers.Contains(AttackPlayerState))
		{
			ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayerCurrentlyInTheLead.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(PlayerCurrentlyInTheLead[i]))
			{
				ABlasterCharacter* Loser = Cast<ABlasterCharacter>(PlayerCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim(false);
	}

	if (AttackPlayerState && VictimPlayerState)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
			if (BlasterPlayer)
			{
				BlasterPlayer->BroadcastElim(AttackPlayerState, VictimPlayerState);
			}
		}
	}
}

void ABlasterGameMode::ResquestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		//用于获取场景中指定类型的所有Actors对象，并将结果存储到PlayerStarts数组中。
		//这里指定获取的类型为APlayerStart::StaticClass()，即获取类型为APlayerStart的所有Actor对象，APlayerStart通常被用作玩家重生的出生点。
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		//用于将游戏玩家的控制器(ElimmedController)重新放置到指定的玩家起始点(PlayerStarts[Selection])上。
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	// 移除排行榜
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	// 走销毁逻辑
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}

float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}
