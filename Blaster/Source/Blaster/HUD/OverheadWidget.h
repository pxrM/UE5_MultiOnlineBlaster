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
	UPROPERTY(meta = (BindWidget))	//��c++��������ͼ�еĹ�����������c++�����ĸ��Ķ���Ӱ�쵽��ͼ���
		class UTextBlock* DisplayText;

public:
	void SetDisplayText(FString TextToDisplay);

	UFUNCTION(BlueprintCallable)
		void ShowPlayerNetRole(APawn* InPawn);

protected:
	virtual void NativeDestruct() override;

};
