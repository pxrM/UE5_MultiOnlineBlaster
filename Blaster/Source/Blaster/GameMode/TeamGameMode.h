// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ATeamGameMode : public ABlasterGameMode
{
	GENERATED_BODY()
	
public:
	ATeamGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override; // Íæ¼ÒÍË³ö

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;


protected:
	virtual void HandleMatchHasStarted() override;
};
