// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	friend class ABlasterCharacter;	//角色类可以访问buff组件类

public:	
	// Sets default values for this component's properties
	UBuffComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;


public:	
	/// <summary>
	/// 治疗
	/// </summary>
	/// <param name="HealAmmp">治疗量</param>
	/// <param name="HealingTime">治疗所需时间2</param>
	void Heal(float HealAmmo, float HealingTime);


protected:
	void HealRampUp(float DeltaTime); //每一帧的血量修复程度

		
private:
	UPROPERTY()
		class ABlasterCharacter* Character;

	bool bHealing = false; //是否正在治疗
	float HealingRate = 0.f; //治愈率，每秒治疗多少
	float AmountToHeal = 0.f; //治疗buff总数量

};
