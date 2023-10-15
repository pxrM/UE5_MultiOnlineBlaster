// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * AGameState���ڱ�����Ϸ���ݣ���������ȡ���Ϸ���
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);

	UFUNCTION()
		void OnRep_OnRepRedTeamScore();
	UFUNCTION()
		void OnRep_OnRepBlueTeamScore();

public:
	UPROPERTY(Replicated)
		TArray<class ABlasterPlayerState*> TopScoringPlayers; //��߷���ң���������Ҳ��и߷֣�������TArray

	TArray<ABlasterPlayerState*> RedTeam;
	TArray<ABlasterPlayerState*> BlueTeam;
	UPROPERTY(ReplicatedUsing = OnRepRedTeamScore)
		float RedTeamScore = 0.f;
	UPROPERTY(ReplicatedUsing = OnRepBlueTeamScore)
		float BlueTeamScore = 0.f;

private:
	float CurTopScore = 0.f; //��ǰ��߷�

};
