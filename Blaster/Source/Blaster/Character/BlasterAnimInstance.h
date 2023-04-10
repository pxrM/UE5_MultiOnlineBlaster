// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BlasterAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation()override;
	virtual void NativeUpdateAnimation(float DeltaSeconds)override;


private:
	/*
	UPROPERTY ��ʾ����������� Unreal Engine ����� Unreal Object System �е����ԡ��������� Unreal Engine �ڱ���ʱ�Զ�����һЩ�������룬
				�������л���Serialization���������ڱ༭������ʾ��������Ա����޸ġ�
	BlueprintReadOnly ��ʾ�������ֻ������ͼ��Blueprint���ж�ȡ��������д�롣�������ͨ�����ڽ� C++ ���͵����ݱ�¶����ͼʹ�ã�
				�����ֲ�ϣ����ͼֱ���޸�������ݣ�����ͨ������ C++ �������ı䡣
	Category = Character ��ʾ����������� Character ���������������ɫ���Եġ��� Unreal Engine �У����е����Զ���Ҫ�����䵽һ���������Category���У�
				�Ա����ڱ༭���н��з������֯������һЩ���õ�����Ѿ��� Unreal Engine Ԥ������ˣ����� Character��Engine��Input �ȵȡ�
				�� C++ ������������ UPROPERTY ���У�����ʹ�� Category ���������Է��䵽Ԥ���������У�Ҳ���Դ����Զ����������������ԡ�
				���ʹ��δ�������������ڱ༭������ʾΪһ���µ���𣬵��ǲ���Ӱ�����ı�������С���ˣ�Category = Character ������Ԥ����� Character ���
				Ҳ�����ǿ������Լ������ Character ���
	meta = (AllowPrivateAccess = "true") ��ʾ�����������˽�з��ʣ�AllowPrivateAccess����Ҳ����˵����ʹ����˽�г�Ա������
				��ͼҲ���Է��ʲ���ȡ������ԡ��������ͨ����������ͼ�з��� C++ �����е�˽�г�Ա������
	*/

	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
		class ABlasterCharacter* BlasterCharacter;	//��ɫ

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		float Speed;  //�ٶ�

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsInAir;	//�Ƿ��ڿ���

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
		bool bIsAccelerating;	//�Ƿ����ڼ����ƶ�

	UPROPERTY(BlueprintReadOnly, Category = Equip, meta = (AllowPrivateAccess = "true"))
		bool bWeaponEquipped;	//�Ƿ�װ��������

private:
	void RefreshBlasterCharacter();

};
