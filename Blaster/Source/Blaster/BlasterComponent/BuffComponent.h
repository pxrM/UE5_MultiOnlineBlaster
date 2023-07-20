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
	/// ���令��
	/// </summary>
	/// <param name="ShieldAmount"></param>
	/// <param name="ReplenishTime"></param>
	void ReplenishShield(float ShieldAmount, float ReplenishTime);

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

	/// <summary>
	/// ��¼��ɫ�ĳ�ʼ��Ծ�߶�
	/// </summary>
	/// <param name="JumpVelocity"></param>
	void SetInitialJumpVelocity(const float JumpVelocity);
	/// <summary>
	/// ��Ծbuff
	/// </summary>
	/// <param name="BuffJumpVelocity"></param>
	/// <param name="BuffTime"></param>
	void BuffJump(float BuffJumpVelocity, float BuffTime);


private:
	/// <summary>
	/// ÿһ֡��Ѫ���޸��̶�
	/// </summary>
	/// <param name="DeltaTime"></param>
	void HealRampUp(float DeltaTime);

	/// <summary>
	/// ÿһ֡�Ļ��ܲ���̶�
	/// </summary>
	/// <param name="DeltaTime"></param>
	void ShieldRampUp(float DeltaTime);

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
	/// <summary>
	/// �����ٶ��޸�
	/// </summary>
	/// <param name="BaseSpeed"></param>
	/// <param name="CrouchSpeed"></param>
	void HandleChangeSpeed(float BaseSpeed, float CrouchSpeed);

	/// <summary>
	/// ��ԾbuffʱЧ�����ָ�֮ǰ����Ծ�߶�
	/// </summary>
	void ResetJumpVelocity();
	/// <summary>
	/// ����㲥��Ծ�ٶ�
	/// </summary>
	/// <param name="JumpVelocity"></param>
	UFUNCTION(NetMulticast, Reliable)
		void MulticastJumpBuff(float JumpVelocity);
	/// <summary>
	/// ������Ծ�ٶ��޸�
	/// </summary>
	/// <param name="JumpVelocity"></param>
	void HandleChangeJumpVelocity(float JumpVelocity);

		
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

	/* jump buff */
	FTimerHandle JumpBuffTimer;
	float InitialJumpVelocity;

	/* shield buff */
	bool bReplenishingShield = false;
	float ShieldReplenishRate = 0.f;
	float ShieldReplenishAmount = 0.f;

};
