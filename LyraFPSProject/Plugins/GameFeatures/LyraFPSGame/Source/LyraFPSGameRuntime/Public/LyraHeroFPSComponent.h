// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/LyraHeroComponent.h"
#include "LyraHeroFPSComponent.generated.h"

/**
 * 取出必要的输入变量，如MoveInput，主要供给动画蓝图使用
 */
UCLASS(Blueprintable, Meta=(BlueprintSpawnableComponent))
class LYRAFPSGAMERUNTIME_API ULyraHeroFPSComponent : public ULyraHeroComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	 ULyraHeroFPSComponent(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintPure, Category="Lyra|FPS")
	FVector2D GetMoveInput() const { return MoveInput; }

	UFUNCTION(BlueprintPure, Category="Lyra|FPS")
	FVector2D GetMouseInput() const { return MouseInput; }
protected:
	virtual void Input_Move(const FInputActionValue& InputActionValue) override;
	virtual void Input_Move_Cancel(const FInputActionValue& InputActionValue) override;
	virtual void Input_LookMouse(const FInputActionValue& InputActionValue) override;
	virtual void Input_LookMouse_Cancel(const FInputActionValue& InputActionValue) override;
public:
	//~ Begin IGameFrameworkInitStateInterface interface
	virtual FName GetFeatureName() const override { return NAME_ActorFeatureName; }
	virtual bool CanChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) const override;
	virtual void HandleChangeInitState(UGameFrameworkComponentManager* Manager, FGameplayTag CurrentState, FGameplayTag DesiredState) override;
	virtual void OnActorInitStateChanged(const FActorInitStateChangedParams& Params) override;
	virtual void CheckDefaultInitialization() override;
	//~ End IGameFrameworkInitStateInterface interface

	// virtual void InitializePlayerInput(UInputComponent* PlayerInputComponent);
protected:
	/** 存放wsad移动输入值 */
	UPROPERTY(BlueprintReadOnly, Category="Lyra|FPS")
	FVector2D MoveInput;
	
	UPROPERTY(BlueprintReadOnly, Category="Lyra|FPS")
	FVector2D MouseInput;
};
