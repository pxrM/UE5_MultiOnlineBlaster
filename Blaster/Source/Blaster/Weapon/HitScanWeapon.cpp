// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"
#include "DrawDebugHelpers.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
		return;

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		// �������߼��
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		// ���н�ɫ
		ABlasterCharacter* HitBlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		// ���������ɫ��controller
		AController* InstigatorController = OwnerPawn->GetController();
		if (HitBlasterCharacter && InstigatorController)
		{
			if (HasAuthority() && !bUseServerSideRewind)
			{
				// �ڷ�����ֱ��ʩ���˺�
				UGameplayStatics::ApplyDamage(
					HitBlasterCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			else if (!HasAuthority() && bUseServerSideRewind)
			{
				// �ڿͻ���������������е���ʩ���˺�
				BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ?
					Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
				BlasterOwnerController = BlasterOwnerController == nullptr ?
					Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
				if (BlasterOwnerCharacter && BlasterOwnerCharacter->IsLocallyControlled() &&
					BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensationComp())
				{
					// ����ʱ����ڷ�����ʱ���ȥ���η���ʱ��
					const float HitTime = BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime;
					BlasterOwnerCharacter->GetLagCompensationComp()->ServerScoreRequest(
						HitBlasterCharacter, Start, HitTarget, HitTime, this);
				}
			}
		}
		// ������Ч
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		// ���Ż�����Ч
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
		// ����ǹ����Ч
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		// ���ſ�����Ч
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutFireHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		// ��25%��������λ�øպó���Ŀ�����ȷ���ɹ��������������������Ŀ������������ܻ�ʧȥ�赲
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		// ��ָ��ͨ�����е����������߼��
		World->LineTraceSingleByChannel(
			OutFireHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = OutFireHit.bBlockingHit ? OutFireHit.ImpactPoint : End;
		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
		// ���ŵ�����Ч
		if (BeamParticles)
		{
			UParticleSystemComponent* BeamCmp = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (BeamCmp)
			{
				BeamCmp->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
