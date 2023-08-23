// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotgunWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void AShotgunWeapon::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
		return;

	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");

	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();
		TMap<ABlasterCharacter*, uint32> HitMap; // 击中次数

		for (uint32 i = 0; i < NumberOfPellets; i++)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter && InstigatorController && HasAuthority())
			{
				if (HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}
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

			//为每个弹丸都播放击中音效，因为它可能会击中周边的物体或者更远的地方
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					0.5f,
					FMath::FRandRange(-0.5f, 0.5f)
				);
			}
		}

		// 计算伤害
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && InstigatorController && HasAuthority())
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}

void AShotgunWeapon::FireShotgun(const TArray<FVector_NetQuantize> HitTargets)
{
	AWeapon::Fire(FVector());
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		TMap<ABlasterCharacter*, uint32> HitMap; // 击中次数

		for (FVector_NetQuantize HitTargetItem : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTargetItem, FireHit);

			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter)
			{
				if (HitMap.Contains(BlasterCharacter))
				{
					HitMap[BlasterCharacter]++;
				}
				else
				{
					HitMap.Emplace(BlasterCharacter, 1);
				}
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}

			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(
					this,
					HitSound,
					FireHit.ImpactPoint,
					0.5f,
					FMath::FRandRange(-0.5f, 0.5f)
				);
			}
		}

		bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
		if (HasAuthority() && bCauseAuthDamage)
		{
			// server，没有开启倒带或者为本地控制角色，在服务器直接施加伤害
			for (auto HitPair : HitMap)
			{
				if (HitPair.Key && InstigatorController && HasAuthority())
				{
					UGameplayStatics::ApplyDamage(
						HitPair.Key,
						Damage * HitPair.Value,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}
		else if (!HasAuthority() && bUseServerSideRewind)
		{
			// 客户端本地控制角色请求服务器进行倒带施加伤害
			TArray<ABlasterCharacter*> HitCharacters;
			HitMap.GenerateKeyArray(HitCharacters);
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
			BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
			if (BlasterOwnerController && BlasterOwnerCharacter &&
				BlasterOwnerCharacter->IsLocallyControlled() &&
				BlasterOwnerCharacter->GetLagCompensationComp())
			{
				// 攻击时间等于服务器时间减去单次发送时间
				const float HitTime = BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime;
				BlasterOwnerCharacter->GetLagCompensationComp()->ShotgunServerScoreRequest(
					HitCharacters, Start, HitTargets, HitTime
				);
			}
		}
	}
}

void AShotgunWeapon::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	// 获取枪口位置
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	// 计算从 TraceStart 到 HitTarget 的方向向量，并进行单位化处理
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	// 以 TraceStart 为起点，沿着射线方向前进 DistanceToSphere 的距离，得到球体的中心点
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	// 循环获取霰弹枪每个子弹的目标点
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		// 先调用 RandomUnitVector 生成一个随机单位向量，然后乘在 0 到 SphereRadius 范围内的随机浮点数，得到一个随机向量 RandVec
		// 这样就可以获得一个随机的偏移向量，用于在球体的中心点周围进行散射偏移
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		// 将球体的中心点 和 随机向量 相加，得到最终的结束位置 EndLoc。这样就完成了散射效果的计算，最终的结束位置将会稍稍偏离球体的中心点。
		const FVector EndLoc = SphereCenter + RandVec;
		// 将 散射后的最终结束位置 减去 射线起始位置，计算出一个从起始位置到结束位置的向量 ToEndLoc。这个向量表示了射线追踪的方向和距离。
		FVector ToEndLoc = EndLoc - TraceStart;
		// 避免溢出
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		HitTargets.Add(ToEndLoc);
	}
}
