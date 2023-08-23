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
		TMap<ABlasterCharacter*, uint32> HitMap; // ���д���

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

			//������Ч
			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}

			//Ϊÿ�����趼���Ż�����Ч����Ϊ�����ܻ�����ܱߵ�������߸�Զ�ĵط�
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

		// �����˺�
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
		TMap<ABlasterCharacter*, uint32> HitMap; // ���д���

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
			// server��û�п�����������Ϊ���ؿ��ƽ�ɫ���ڷ�����ֱ��ʩ���˺�
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
			// �ͻ��˱��ؿ��ƽ�ɫ������������е���ʩ���˺�
			TArray<ABlasterCharacter*> HitCharacters;
			HitMap.GenerateKeyArray(HitCharacters);
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
			BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
			if (BlasterOwnerController && BlasterOwnerCharacter &&
				BlasterOwnerCharacter->IsLocallyControlled() &&
				BlasterOwnerCharacter->GetLagCompensationComp())
			{
				// ����ʱ����ڷ�����ʱ���ȥ���η���ʱ��
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
	// ��ȡǹ��λ��
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	// ����� TraceStart �� HitTarget �ķ��������������е�λ������
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	// �� TraceStart Ϊ��㣬�������߷���ǰ�� DistanceToSphere �ľ��룬�õ���������ĵ�
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	// ѭ����ȡ����ǹÿ���ӵ���Ŀ���
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		// �ȵ��� RandomUnitVector ����һ�������λ������Ȼ����� 0 �� SphereRadius ��Χ�ڵ�������������õ�һ��������� RandVec
		// �����Ϳ��Ի��һ�������ƫ����������������������ĵ���Χ����ɢ��ƫ��
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		// ����������ĵ� �� ������� ��ӣ��õ����յĽ���λ�� EndLoc�������������ɢ��Ч���ļ��㣬���յĽ���λ�ý�������ƫ����������ĵ㡣
		const FVector EndLoc = SphereCenter + RandVec;
		// �� ɢ�������ս���λ�� ��ȥ ������ʼλ�ã������һ������ʼλ�õ�����λ�õ����� ToEndLoc�����������ʾ������׷�ٵķ���;��롣
		FVector ToEndLoc = EndLoc - TraceStart;
		// �������
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();
		HitTargets.Add(ToEndLoc);
	}
}
