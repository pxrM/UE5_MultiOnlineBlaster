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

		// 线性射线检测
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		// 击中角色
		ABlasterCharacter* HitBlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		// 发起射击角色的controller
		AController* InstigatorController = OwnerPawn->GetController();
		if (HitBlasterCharacter && InstigatorController)
		{
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			if (HasAuthority() && bCauseAuthDamage)
			{
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;
				// server，没有开启倒带或者为本地控制角色，在服务器直接施加伤害
				UGameplayStatics::ApplyDamage(
					HitBlasterCharacter,
					DamageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			else if (!HasAuthority() && bUseServerSideRewind)
			{
				// 在客户端请求服务器进行倒带施加伤害
				BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ?
					Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
				BlasterOwnerController = BlasterOwnerController == nullptr ?
					Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
				if (BlasterOwnerCharacter && BlasterOwnerCharacter->IsLocallyControlled() &&
					BlasterOwnerController && BlasterOwnerCharacter->GetLagCompensationComp())
				{
					// 攻击时间等于服务器时间减去单次发送时间
					const float HitTime = BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime;
					BlasterOwnerCharacter->GetLagCompensationComp()->ServerScoreRequest(HitBlasterCharacter, Start, HitTarget, HitTime);
				}
			}
		}
		// 播放特效
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		// 播放击中音效
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
		// 播放枪口特效
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		// 播放开火音效
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
		// 多25%这样最终位置刚好超过目标点以确保成功，否则可能正好命中在目标表面这样可能会失去阻挡
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;
		// 沿指定通道进行单次线性射线检测
		World->LineTraceSingleByChannel(
			OutFireHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		FVector BeamEnd = OutFireHit.bBlockingHit ? OutFireHit.ImpactPoint : End;
		DrawDebugSphere(GetWorld(), BeamEnd, 16.f, 12, FColor::Orange, true);
		// 播放弹道特效
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
