// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()); //Owner�� AProjectileWeapon::Fire =�� UCombatComponent::EquipWeapon��ָ��Ϊ��ɫ
	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			//����ֻ��֪ͨ���ã���Ҫ�˺����պ���
			//��OtherActor������˺���DamageVal���˺�ֵ��OwnerController������˺��Ŀ�������this��������˺��Ķ���UDamageType::StaticClass()�������ʹ�õ��˺�����
			UGameplayStatics::ApplyDamage(OtherActor, DamageVal, OwnerController, this, UDamageType::StaticClass());
		}
	}

	// ��Ϊsuper����destroy��������Ҫ�ŵ�����ִ��
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
