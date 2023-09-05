// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;
	UFUNCTION()
		void OnDestroySession(bool bWasSuccessful);

public:
	void MenuSetup();
	void MenuTearDown();

private:
	UFUNCTION()
		void ReturnBtnClicked();

private:
	UPROPERTY(meta = (BindWidget))
		class UButton* ReturnBtn;
	UPROPERTY()
		class UMultiPlayerSessionSubsystem* MultiPlayerSessionSubsystem;
	UPROPERTY()
		class APlayerController* PlayerCtl;

};
