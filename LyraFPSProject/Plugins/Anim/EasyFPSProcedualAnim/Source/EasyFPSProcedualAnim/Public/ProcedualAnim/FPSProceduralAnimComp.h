// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/ActorComponent.h"
#include "FPSProceduralAnimComp.generated.h"

class UDataAsset_ProceduralAnim;
class UProceduralAnimFragment;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAnimEventSignal,FGameplayTag, EventTag,FGameplayEventData, Payload);

// DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJumpedSignature);

USTRUCT(BlueprintType)
struct FFragmentDataMap
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FGameplayTag,TObjectPtr< UDataAsset_ProceduralAnim>> Data;	
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent),Blueprintable)
class EASYFPSPROCEDUALANIM_API UFPSProceduralAnimComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UFPSProceduralAnimComp();

protected:
	virtual void InitializeComponent() override;
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
public:
	void AddFragments(UProceduralAnimFragment* NewFragments);
	
	UFUNCTION(BlueprintPure)
	FVector GetTargetOffset(
		UPARAM(meta=(Categories="ProceduralAnim.Fragment.Type")) FGameplayTag FragmentTag=FGameplayTag(),
		UPARAM(meta=(Categories="ProceduralAnim.Fragment.Target")) FGameplayTag TargetType=FGameplayTag());
	
	UFUNCTION(BlueprintPure)
	FRotator GetTargetRot(
		UPARAM(meta=(Categories="ProceduralAnim.Fragment.Type")) FGameplayTag FragmentTag=FGameplayTag(),
		UPARAM(meta=(Categories="ProceduralAnim.Fragment.Target")) FGameplayTag TargetType=FGameplayTag());
	
	UFUNCTION(BlueprintPure)
	UAnimMontage* GetTargetMontage(
		UPARAM(meta=(Categories="ProceduralAnim.Fragment.Type")) FGameplayTag FragmentTag=FGameplayTag());
	
	
	UFUNCTION(BlueprintPure)
	float GetTargetAlpha(
		UPARAM(meta=(Categories="ProceduralAnim.Fragment.Type")) FGameplayTag FragmentTag=FGameplayTag());
	
	UFUNCTION(BlueprintCallable)
	void UpdateData(TMap<FGameplayTag,UDataAsset_ProceduralAnim*>& Data);
	
	UFUNCTION(BlueprintPure)
	UProceduralAnimFragment* FindFragment(UPARAM(meta=(Categories="ProceduralAnim.Fragment.Type")) FGameplayTag FragmentTag=FGameplayTag());
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Instanced)
	TArray<TObjectPtr<UProceduralAnimFragment>> Fragments;
	
public:
	UFUNCTION(BlueprintCallable)
	void SendSignal(UPARAM(meta=(Categories="ProceduralAnim.Signal")) const FGameplayTag Signal);
	
protected:
	void InitInhanceInput();
	
	void SendEvent(FGameplayTag EventTag,FGameplayEventData Payload=FGameplayEventData()) const;
public:
	UPROPERTY(BlueprintAssignable, BlueprintReadOnly)
	FAnimEventSignal AnimEvent;
	
	
	friend class UProceduralAnimFragment;
};
