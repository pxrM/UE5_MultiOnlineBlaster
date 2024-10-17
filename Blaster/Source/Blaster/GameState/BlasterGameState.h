// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

class ABlasterPlayerState;

/**
 * AGameState���ڱ�����Ϸ���ݣ���������ȡ���Ϸ���
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(ABlasterPlayerState* ScoringPlayer);

	void RedTeamScores();
	void BlueTeamScores();
	UFUNCTION()
		void OnRep_RedTeamScore();
	UFUNCTION()
		void OnRep_BlueTeamScore();

public:
	/// <summary>
	/// ��߷���ң���������Ҳ��и߷֣�������TArray
	/// </summary>
	UPROPERTY(Replicated)
		TArray<ABlasterPlayerState*> TopScoringPlayers;

	TArray<ABlasterPlayerState*> RedTeam;
	TArray<ABlasterPlayerState*> BlueTeam;
	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
		float RedTeamScore = 0.f;
	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
		float BlueTeamScore = 0.f;

private:
	/// <summary>
	/// ��ǰ��߷�
	/// </summary>
	float CurTopScore = 0.f;

};
