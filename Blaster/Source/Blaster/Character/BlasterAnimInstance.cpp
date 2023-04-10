// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	RefreshBlasterCharacter();
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//�� NativeUpdateAnimation() �У����ڸ��¶�����Ƶ�ʱȳ�ʼ���������ߣ���� BlasterCharacter ���ܻᱻ������ C++ �����޸Ļ�������Ϊ nullptr��
	//��ˣ���ÿ��ִ�� NativeUpdateAnimation() ����ʱ������Ҫ���»�ȡ��ǰ�����������ƵĽ�ɫ����
	//��ʹ�� Cast<ABlasterCharacter>() ����ǿ������ת������ȷ�� BlasterCharacter ָ����ȷ�Ķ���
	//����������»�ȡ����������󷵻ص� Pawn ������ ABlasterCharacter ���ͣ��� Cast<ABlasterCharacter>() ���������ؿ�ָ�루nullptr����
	//��ʱ�����Ҫ�ٴ��ж� BlasterCharacter �Ƿ�Ϊ�գ��Ա���ʹ��һ����Ч��ָ������³��������
	if (BlasterCharacter == nullptr)
	{
		RefreshBlasterCharacter();
	}
	if (BlasterCharacter == nullptr)
	{
		return;
	}

	FVector Velocity = BlasterCharacter->GetVelocity(); //��ȡ��ǰ��ɫ������ٶ�
	Velocity.Z = 0.f;	//���ٶ������� Z ������Ϊ 0����������ʹ�ý�ɫ��ˮƽ���ϵ��ƶ��ٶȸ�����ʵ��Ч��
	Speed = Velocity.Size();	//�����ٶ������Ĵ�С�����ٶȱ���ֵ��

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();	//ͨ���Ƿ�����׹���жϽ�ɫ�Ƿ��ڿ���

	//bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	//ʹ��IsNearlyZero()������и������Ƚϣ���߾��Ⱥͼ����ٶȣ�
	//�� FVector::IsNearlyZero() �������ǿ��Դ���һ�� float ���͵Ĳ��� Tolerance �ģ�����ʾ�����������С��Χ��
	//����ֶ�ָ�� Tolerance ����Ϊ0.1f����ʾ�����������С��ΧΪ 0.1 ��ƽ������ 0.01��
	//���û���ֶ�ָ�� Tolerance ��������Ĭ��ʹ�� FVector::ThreshVectorLen ����Ϊ�������ֵ��
	//FVector::ThreshVectorLen ��Ĭ��ֵΪ 1.e-4f������ӽ��� 0 ��������Сƽ�����ܳ��� 0.0001��
	bIsAccelerating = !BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero(); 

	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
}

void UBlasterAnimInstance::RefreshBlasterCharacter()
{
	//ͨ������ȡ BlasterCharacter ���߼���װ�� RefreshBlasterCharacter() �����У�������Ҫʹ�� BlasterCharacter ָ��ʱ���øú�����
	//���Ա�����ÿ��ִ�� NativeUpdateAnimation() ����ʱ�����»�ȡ���󲢽�������ת���Ĳ�����
	// 
	// TryGetPawnOwner() ������ȡ��ǰ��������Controller�������ƵĽ�ɫ��Pawn������
	// Cast<ABlasterCharacter>() ��һ����UE�ṩ������ת������ģ�壬���ڽ� UObject ���͵Ķ���ת��Ϊ�����������ָ��
	// ��������� TryGetPawnOwner() �������ص� Pawn ����ǿ��ת��Ϊ ABlasterCharacter ���͵�ָ��
	// ע�⣺���� TryGetPawnOwner() ���ص� Pawn �����п��ܲ��� ABlasterCharacter ���͵ģ���� Cast<ABlasterCharacter>() �ķ���ֵ�п���Ϊ��ָ��
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}
