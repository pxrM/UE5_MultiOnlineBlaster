// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))	//将c++变量与蓝图中的关联，这样对c++变量的更改都会影响到蓝图里的
		class UTextBlock* DisplayText;

public:
	void SetDisplayText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn* InPawn);

protected:
	virtual void NativeDestruct() override;

};
