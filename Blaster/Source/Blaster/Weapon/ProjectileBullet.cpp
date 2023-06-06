// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()); //Owner在 AProjectileWeapon::Fire =》 UCombatComponent::EquipWeapon中指定为角色
	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;
		if (OwnerController)
		{
			//这里只是通知作用，需要伤害接收函数
			//对OtherActor造成了伤害，DamageVal是伤害值，OwnerController是造成伤害的控制器，this代表造成伤害的对象，UDamageType::StaticClass()则代表所使用的伤害类型
			UGameplayStatics::ApplyDamage(OtherActor, DamageVal, OwnerController, this, UDamageType::StaticClass());
		}
	}

	// 因为super里有destroy，所以需要放到后面执行
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}
