// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"
#include "DrawDebugHelpers.h"

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
		return;

	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		//线性射线检测
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter && InstigatorController && HasAuthority())
		{
			//在服务器施加伤害
			UGameplayStatics::ApplyDamage(
				BlasterCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
		}
		//播放特效
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		//播放击中音效
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
		//播放枪口特效
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		//播放开火音效
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
		FVector End = bUseSactter ? TraceEndWithScatter(TraceStart, HitTarget)
			: TraceStart + (HitTarget - TraceStart) * 1.25f; //多25%这样最终位置刚好超过目标点以确保成功，否则可能正好命中在目标表面这样可能会失去阻挡
		World->LineTraceSingleByChannel(
			OutFireHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		//播放弹道特效
		if (BeamParticles)
		{
			FVector BeamEnd = OutFireHit.bBlockingHit ? OutFireHit.ImpactPoint : End;
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

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	// 计算从 TraceStart 到 HitTarget 的方向向量，并进行单位化处理
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	// 以 TraceStart 为起点，沿着射线方向前进 DistanceToSphere 的距离，得到球体的中心点
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	// 先调用 RandomUnitVector 生成一个随机单位向量，然后乘在 0 到 SphereRadius 范围内的随机浮点数，得到一个随机向量 RandVec
	// 这样就可以获得一个随机的偏移向量，用于在球体的中心点周围进行散射偏移
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	// 将球体的中心点 和 随机向量 相加，得到最终的结束位置 EndLoc。这样就完成了散射效果的计算，最终的结束位置将会稍稍偏离球体的中心点。
	FVector EndLoc = SphereCenter + RandVec;
	// 将 散射后的最终结束位置 减去 射线起始位置，计算出一个从起始位置到结束位置的向量 ToEndLoc。这个向量表示了射线追踪的方向和距离。
	FVector ToEndLoc = EndLoc - TraceStart;

	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	//DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	//DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), FColor::Cyan, true);

	// TRACE_LENGTH / ToEndLoc.Size() 这里除一下是为了防止数值太大溢出
	return  FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}
