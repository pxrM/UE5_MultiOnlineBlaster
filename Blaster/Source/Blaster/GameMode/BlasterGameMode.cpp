// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");	//����ʱ���ѽ�������ʾ��ʤ�߲���ʼ��ȴ����ʱ
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true; //��Ϸģʽ�������ڵȴ���ʼ״̬��GameMode��Ϊÿ���������Ĭ�ϵ�Pawn��ֻ�ܷ�����ȥ����ֱ���ֶ�����StartMatch()
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
	if (AttackPlayerState && AttackPlayerState != VictimPlayerState)
	{
		AttackPlayerState->AddToScore(1.0f);
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	if (ElimmedCharacter)
	{
		ElimmedCharacter->Elim();
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
		//���ڻ�ȡ������ָ�����͵�����Actors���󣬲�������洢��PlayerStarts�����С�
		//����ָ����ȡ������ΪAPlayerStart::StaticClass()������ȡ����ΪAPlayerStart������Actor����APlayerStartͨ����������������ĳ����㡣
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		//���ڽ���Ϸ��ҵĿ�����(ElimmedController)���·��õ�ָ���������ʼ��(PlayerStarts[Selection])�ϡ�
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}
