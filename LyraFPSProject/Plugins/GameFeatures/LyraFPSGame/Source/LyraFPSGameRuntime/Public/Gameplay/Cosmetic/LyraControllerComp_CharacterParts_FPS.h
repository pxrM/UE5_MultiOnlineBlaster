// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CharacterPartData_FPS.h"
#include "Cosmetics/LyraControllerComponent_CharacterParts.h"
#include "LyraControllerComp_CharacterParts_FPS.generated.h"


UCLASS(ClassGroup=(FPS), meta=(BlueprintSpawnableComponent))
class LYRAFPSGAMERUNTIME_API ULyraControllerComp_CharacterParts_FPS : public UControllerComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	ULyraControllerComp_CharacterParts_FPS(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


public:
	
	//~UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	//~End of UActorComponent interface

	// Adds a character part to the actor that owns this customization component, should be called on the authority only
	UFUNCTION(BlueprintCallable, Category=Cosmetics)
	void AddCharacterPart(const FLyraCharacterPart_FPS& NewPart);

	// Removes a previously added character part from the actor that owns this customization component, should be called on the authority only
	UFUNCTION(BlueprintCallable, Category=Cosmetics)
	void RemoveCharacterPart(const FLyraCharacterPart_FPS& PartToRemove);

	// Removes all added character parts, should be called on the authority only
	UFUNCTION(BlueprintCallable, Category=Cosmetics)
	void RemoveAllCharacterParts();

	// Applies relevant developer settings if in PIE
	void ApplyDeveloperSettings();

protected:
	UPROPERTY(EditAnywhere, Category=Cosmetics)
	TArray<FLyraControllerCharacterPartEntry> CharacterParts;
	
	UPROPERTY(EditAnywhere, Category=Cosmetics)
	TArray<FLyraControllerCharacterPartEntry_FPS> CharacterParts_Local;
private:
	ULyraPawnComponent_CharacterParts* GetPawnCustomizer_FPS() const;

	UFUNCTION()
	void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	UFUNCTION()
	void OnPossessedPawnChanged_Local(APawn* OldPawn, APawn* NewPawn);
	
	void AddCharacterPartInternal(const FLyraCharacterPart_FPS& NewPart, ECharacterPartSource Source);

	void AddCheatPart(const FLyraCharacterPart_FPS& NewPart, bool bSuppressNaturalParts);
	void ClearCheatParts();

	void SetSuppressionOnNaturalParts(bool bSuppressed);

	friend class ULyraCosmeticCheats;
};
