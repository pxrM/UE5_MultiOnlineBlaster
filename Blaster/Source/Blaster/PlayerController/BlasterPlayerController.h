// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeats(int32 Defeats);
	//当一个控制器获取一个 Pawn 后，引擎会调用该控制器的 OnPossess 函数，并将获取的 Pawn 作为参数传入其中。
	virtual void OnPossess(APawn* InPawn)override;


protected:
	virtual void BeginPlay() override;


private:
	class ABlasterHUD* BlasterHUD;


};
