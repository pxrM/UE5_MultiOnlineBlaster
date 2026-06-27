// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraCharacter.h"
#include "LyraFPSCharacter.generated.h"

class ULyraHeroFPSComponent;

UCLASS()
class LYRAFPSGAMERUNTIME_API ALyraFPSCharacter : public ALyraCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALyraFPSCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	UFUNCTION(BlueprintPure)
	ULyraHeroFPSComponent* GetHeroComp() const;
	
protected:
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
};
