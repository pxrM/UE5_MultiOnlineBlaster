// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	UWorld* World = GetWorld();
	// 攻击发起者
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	/*武器的枪口位置生成投射物对象*/
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (World && InstigatorPawn && MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();
		SpawnParams.Instigator = InstigatorPawn;

		AProjectile* SpawnProjectile = nullptr;
		if (bUseServerSideRewind)
		{
			if (InstigatorPawn->HasAuthority()) //server
			{
				if (InstigatorPawn->IsLocallyControlled()) //server控制角色，使用复制弹
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = false;
					SpawnProjectile->DamageVal = Damage;
				}
				else //server其它客户端控制角色，使用非复制弹，验证倒带
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = true;
				}
			}
			else //client, using SSR
			{
				if (InstigatorPawn->IsLocallyControlled()) //client本地控制角色，使用非复制弹验证倒带
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = true;
					SpawnProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnProjectile->InitialVelocity = SpawnProjectile->GetActorForwardVector() * SpawnProjectile->InitialSpeed;
					SpawnProjectile->DamageVal = Damage;
				}
				else //client其它复制角色，使用非复制弹验证倒带，不验证倒带
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = false;
				}
			}
		}
		else
		{
			if (InstigatorPawn->HasAuthority())
			{
				SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnProjectile->bUseServerSideRewind = false;
				SpawnProjectile->DamageVal = Damage;
			}
		}
	}
}
