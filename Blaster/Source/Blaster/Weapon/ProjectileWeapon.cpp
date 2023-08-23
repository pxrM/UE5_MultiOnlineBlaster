// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	UWorld* World = GetWorld();
	// ����������
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	/*������ǹ��λ������Ͷ�������*/
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
				if (InstigatorPawn->IsLocallyControlled()) //server���ƽ�ɫ��ʹ�ø��Ƶ�
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = false;
					SpawnProjectile->DamageVal = Damage;
				}
				else //server�����ͻ��˿��ƽ�ɫ��ʹ�÷Ǹ��Ƶ�����֤����
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = true;
				}
			}
			else //client, using SSR
			{
				if (InstigatorPawn->IsLocallyControlled()) //client���ؿ��ƽ�ɫ��ʹ�÷Ǹ��Ƶ���֤����
				{
					SpawnProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnProjectile->bUseServerSideRewind = true;
					SpawnProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnProjectile->InitialVelocity = SpawnProjectile->GetActorForwardVector() * SpawnProjectile->InitialSpeed;
					SpawnProjectile->DamageVal = Damage;
				}
				else //client�������ƽ�ɫ��ʹ�÷Ǹ��Ƶ���֤����������֤����
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
