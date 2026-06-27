// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "UObject/Object.h"
#include "ProceduralAnimFragment.generated.h"

class UDataAsset_ProceduralAnim;
class UCharacterMovementComponent;
class UFPSProceduralAnimComp;



/**
 * 
 */
UCLASS(BlueprintType,Blueprintable,EditInlineNew,DefaultToInstanced,Abstract, meta = (autoExpandCategories = "Default"))
class EASYFPSPROCEDUALANIM_API UProceduralAnimFragment : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(UFPSProceduralAnimComp* InOwnerComp);
	
	virtual  void Tick(float DeltaTime);

public:
	/** 
	 * 获取最终偏移
	 * @return 找不到：有Data->0 无Data->Offset
	 * @param TargetType 目标类型匹配->需要Data
	 */
	UFUNCTION(BlueprintPure)
	virtual FVector GetOffset(FGameplayTag TargetType=FGameplayTag());
	
	UFUNCTION(BlueprintPure)
	virtual FRotator GetRot(FGameplayTag TargetType=FGameplayTag());
	
	UFUNCTION(BlueprintPure)
	virtual FORCEINLINE UAnimMontage* GetMontage(){return Montage;}
	
	UFUNCTION(BlueprintPure)
	virtual float GetAlpha();
	
	FGameplayTag GetTag() const{return Tag;}
	
	UFUNCTION(BlueprintPure)
	virtual FORCEINLINE UDataAsset_ProceduralAnim* GetData(){return nullptr;}
	
	UFUNCTION(BlueprintCallable)
	virtual bool SetData(UDataAsset_ProceduralAnim* InData){return false;}
protected:
	//发送不带参数的信号，方便从外部激活函数
	virtual void ReceiveSignal(const FGameplayTag& Signal){}
	virtual void SendEvent(FGameplayTag EventTag,FGameplayEventData Payload=FGameplayEventData());

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,meta=(Categories="ProceduralAnim.Fragment.Type"),Category="Default")
	FGameplayTag Tag;
	
	UPROPERTY()
	FVector Offset=FVector::Zero();
	
	UPROPERTY()
	FRotator Rot=FRotator::ZeroRotator;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> Montage;
	
	UPROPERTY()
	float Alpha=0.f;

protected:
	
	UFPSProceduralAnimComp* OwnerComp = nullptr;
	ACharacter* OwnerChar = nullptr;
	TObjectPtr<UCharacterMovementComponent> OwnerMoveComp = nullptr;
	
	
	
	friend class UFPSProceduralAnimComp;
};
