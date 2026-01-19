// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "PImage.generated.h"

/**
 * 
 */
UCLASS()
class MENUSYSTEMPLUGINP_API UPImage : public UImage
{
	GENERATED_BODY()

public:
	UPImage(const FObjectInitializer& ObjectInitializer);
	virtual void SetBrush(const FSlateBrush& InBrush) override;
	
public:
	UFUNCTION(BlueprintCallable, Category = "PImage")
	void SetBrushes(const TArray<FSlateBrush>& NewBrushes);

	UFUNCTION(BlueprintCallable, Category = "PImage")
	const TArray<FSlateBrush>& GetBrushes() const { return Brushes; }

	UFUNCTION(BlueprintCallable, Category = "PImage")
	void SetCurrentIndex(const int32 NewIndex);

	UFUNCTION(BlueprintCallable, Category = "PImage")
	int32 GetCurrentIndex() const { return CurrentIndex; }

private:
	void UpdateImageAppearance();

protected:
	virtual void SynchronizeProperties() override;
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
#endif

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	int32 CurrentIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TArray<FSlateBrush> Brushes;
};
