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
	/// <summary>
	/// 速度buff
	/// </summary>
	/// <param name="BuffBaseSpeed">基本移动速度</param>
	/// <param name="BuffCrouchSpeed">蹲伏移动速度</param>
	/// <param name="BuffTime">增益时间</param>
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	/// <summary>
	/// 记录角色的初始速度
	/// </summary>
	/// <param name="BaseSpeed">正常移动速度</param>
	/// <param name="CrouchSpeed">蹲伏移动速度</param>
	void SetInitialSpeeds(const float BaseSpeed, const float CrouchSpeed);


protected:
	/// <summary>
	/// 每一帧的血量修复程度
	/// </summary>
	/// <param name="DeltaTime"></param>
	void HealRampUp(float DeltaTime);


private:
	/// <summary>
	/// 速度buff时效结束恢复之前的速度
	/// </summary>
	void ResetSpeeds();
	/// <summary>
	/// 网络广播同步速度
	/// </summary>
	/// <param name="BaseSpeed"></param>
	/// <param name="CrouchSpeed"></param>
	UFUNCTION(NetMulticast, Reliable)
		void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

		
private:
	UPROPERTY()
		class ABlasterCharacter* Character;

	/* health buff */
	bool bHealing = false; //是否正在治疗
	float HealingRate = 0.f; //治愈率，每秒治疗多少
	float AmountToHeal = 0.f; //治疗buff总数量

	/* speed buff */
	FTimerHandle SpeedBuffTimer;
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

};
