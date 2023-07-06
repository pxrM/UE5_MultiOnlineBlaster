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

		//�������߼��
		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter && InstigatorController && HasAuthority())
		{
			//�ڷ�����ʩ���˺�
			UGameplayStatics::ApplyDamage(
				BlasterCharacter,
				Damage,
				InstigatorController,
				this,
				UDamageType::StaticClass()
			);
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
		//���Ż�����Ч
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}
		//����ǹ����Ч
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		//���ſ�����Ч
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
			: TraceStart + (HitTarget - TraceStart) * 1.25f; //��25%��������λ�øպó���Ŀ�����ȷ���ɹ��������������������Ŀ������������ܻ�ʧȥ�赲
		World->LineTraceSingleByChannel(
			OutFireHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);

		//���ŵ�����Ч
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
	// ����� TraceStart �� HitTarget �ķ��������������е�λ������
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	// �� TraceStart Ϊ��㣬�������߷���ǰ�� DistanceToSphere �ľ��룬�õ���������ĵ�
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	// �ȵ��� RandomUnitVector ����һ�������λ������Ȼ����� 0 �� SphereRadius ��Χ�ڵ�������������õ�һ��������� RandVec
	// �����Ϳ��Ի��һ�������ƫ����������������������ĵ���Χ����ɢ��ƫ��
	FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	// ����������ĵ� �� ������� ��ӣ��õ����յĽ���λ�� EndLoc�������������ɢ��Ч���ļ��㣬���յĽ���λ�ý�������ƫ����������ĵ㡣
	FVector EndLoc = SphereCenter + RandVec;
	// �� ɢ�������ս���λ�� ��ȥ ������ʼλ�ã������һ������ʼλ�õ�����λ�õ����� ToEndLoc�����������ʾ������׷�ٵķ���;��롣
	FVector ToEndLoc = EndLoc - TraceStart;

	//DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	//DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	//DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), FColor::Cyan, true);

	// TRACE_LENGTH / ToEndLoc.Size() �����һ����Ϊ�˷�ֹ��ֵ̫�����
	return  FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}
