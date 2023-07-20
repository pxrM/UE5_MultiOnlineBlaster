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
	friend class ABlasterCharacter;	//��ɫ����Է���buff�����

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
	/// ����
	/// </summary>
	/// <param name="HealAmmp">������</param>
	/// <param name="HealingTime">��������ʱ��2</param>
	void Heal(float HealAmmo, float HealingTime);
	/// <summary>
	/// �ٶ�buff
	/// </summary>
	/// <param name="BuffBaseSpeed">�����ƶ��ٶ�</param>
	/// <param name="BuffCrouchSpeed">�׷��ƶ��ٶ�</param>
	/// <param name="BuffTime">����ʱ��</param>
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	/// <summary>
	/// ��¼��ɫ�ĳ�ʼ�ٶ�
	/// </summary>
	/// <param name="BaseSpeed">�����ƶ��ٶ�</param>
	/// <param name="CrouchSpeed">�׷��ƶ��ٶ�</param>
	void SetInitialSpeeds(const float BaseSpeed, const float CrouchSpeed);


protected:
	/// <summary>
	/// ÿһ֡��Ѫ���޸��̶�
	/// </summary>
	/// <param name="DeltaTime"></param>
	void HealRampUp(float DeltaTime);


private:
	/// <summary>
	/// �ٶ�buffʱЧ�����ָ�֮ǰ���ٶ�
	/// </summary>
	void ResetSpeeds();
	/// <summary>
	/// ����㲥ͬ���ٶ�
	/// </summary>
	/// <param name="BaseSpeed"></param>
	/// <param name="CrouchSpeed"></param>
	UFUNCTION(NetMulticast, Reliable)
		void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

		
private:
	UPROPERTY()
		class ABlasterCharacter* Character;

	/* health buff */
	bool bHealing = false; //�Ƿ���������
	float HealingRate = 0.f; //�����ʣ�ÿ�����ƶ���
	float AmountToHeal = 0.f; //����buff������

	/* speed buff */
	FTimerHandle SpeedBuffTimer;
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

};
