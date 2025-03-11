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

void AShotgunWeapon::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	// 调用父类方法，播放开火动画
	AWeapon::Fire(FVector());

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;

	AController* InstigatorController = OwnerPawn->GetController();
	// 获取枪口位置
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		// 击中次数
		TMap<ABlasterCharacter*, uint32> HitMap;
		// 爆头击中次数
		TMap<ABlasterCharacter*, uint32> HeadShotHitMap;
		// 遍历每个射击目标
		for (const FVector_NetQuantize& HitTargetItem : HitTargets)
		{
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTargetItem, FireHit);

		 	// 如果击中了玩家角色
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter)
			{
				// 检查是否爆头（通过骨骼名称判断）
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");
				if (bHeadShot)
				{
					// 如果爆头击中次数统计中包含该角色，则增加其爆头击中次数
					if (HeadShotHitMap.Contains(BlasterCharacter)) HeadShotHitMap[BlasterCharacter]++;
					// 否则，将该角色添加到爆头击中次数统计中，并设置其爆头击中次数为1
					else HeadShotHitMap.Emplace(BlasterCharacter, 1);
				}
				else
				{
					// 如果普通击中次数统计中包含该角色，则增加其普通击中次数
					if (HitMap.Contains(BlasterCharacter)) HitMap[BlasterCharacter]++;
					// 否则，将该角色添加到普通击中次数统计中，并设置其普通击中次数为1
					else HitMap.Emplace(BlasterCharacter, 1);
				}

				// 播放命中特效
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}

				// 播放命中音效
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
		}
		// 被击中到的角色，服务器倒带使用
		TArray<ABlasterCharacter*> HitCharacters;
		// 被击中角色所受到的总伤害
		TMap<ABlasterCharacter*, float> DamageMap;
		// 计算普通命中造成的伤害
		for (auto& HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				HitCharacters.AddUnique(HitPair.Key);
			}
		}
		// 计算爆头造成的伤害
		for (auto& HeadShotHitPair : HeadShotHitMap)
		{
			if (HeadShotHitPair.Key)
			{
				if (DamageMap.Contains(HeadShotHitPair.Key)) DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				else DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);

				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}
	
		// 判断是否应该在此处应用伤害（服务器或本地控制且未启用服务器倒带）
		bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
		if (HasAuthority() && bCauseAuthDamage)
		{
			for (auto& DamagePair : DamageMap)
			{
				UGameplayStatics::ApplyDamage(
					DamagePair.Key,
					DamagePair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
		
		// 客户端本地控制角色请求服务器进行倒带施加伤害
		if (!HasAuthority() && bUseServerSideRewind)
		{
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
	// 获取武器枪口的 Socket，用于确定射击起始点
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	// 获取枪口的世界坐标变换
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	// 获取射击的起始位置（枪口位置）
	const FVector TraceStart = SocketTransform.GetLocation();

	// 计算从 TraceStart 到 HitTarget 的方向向量，并进行单位化处理（枪口指向目标的单位向量）
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	// 以 TraceStart 为起点，沿着射线方向前进 DistanceToSphere 的距离，得到球体的中心点
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	// 循环生成每一颗散射子弹的终点位置
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		// 先调用 RandomUnitVector 生成一个随机单位向量，然后乘在 0 到 SphereRadius 范围内的随机浮点数，得到一个随机向量 RandVec
		// 这样就可以获得一个随机的偏移向量，用于在球体的中心点周围进行散射偏移
		// 	生成随机偏移向量：
		// 	1. RandomUnitVector() 生成一个随机方向的单位向量
		// 	2. FRandRange(0.f, SphereRadius) 生成 0 到球体半径之间的随机距离
		// 	3. 两者相乘得到在球体内的随机偏移向量
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		// 将球体的中心点 和 随机向量 相加，得到最终的结束位置 EndLoc。这样就完成了散射效果的计算，最终的结束位置将会稍稍偏离球体的中心点。
		// （计算散射点：球体中心点 + 随机偏移）
		const FVector EndLoc = SphereCenter + RandVec;
		// 将 散射后的最终结束位置 减去 射线起始位置，计算出一个从起始位置到结束位置的向量 ToEndLoc。这个向量表示了射线追踪的方向和距离。
		// （计算从起始点到散射点的方向向量）
		FVector ToEndLoc = EndLoc - TraceStart;
		// 避免溢出
		// 标准化射程：
		// 1. 保持方向不变
		// 2. 将距离调整为 TRACE_LENGTH
		// 3. 防止射程过长导致的性能问题
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		// 将计算出的散射后的目标点添加到 HitTargets 数组中
		HitTargets.Add(ToEndLoc);
	}
}
