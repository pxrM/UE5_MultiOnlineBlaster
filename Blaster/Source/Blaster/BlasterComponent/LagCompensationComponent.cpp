// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Blaster.h"

// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	FFramePackage Package;
	SaveFramePackage(Package);
	//ShowFramePackage(Package, FColor::Orange);
}

// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	SaveFramePackage();
}

void ULagCompensationComponent::SaveFramePackage()
{
	if (Character == nullptr || !Character->HasAuthority()) return;
	FFramePackage ThisFrame;
	if (FrameHistory.Num() <= 1)
	{
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame); // �����ͷ��
	}
	else
	{
		// ��ȡ����֡�������֡��֮���ʱ���
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			// ɾ�����ڵľɰ�
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame);
		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		TMap<FName, UBoxComponent*> HitCollisionBoxes = Character->GetHitCollisionBoxs();
		for (const auto& BoxPair : HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FServerSideRewindResult Comfim = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	AWeapon* EquippedWeapon = Character->GetEquippedWeapon();
	if (Comfim.bHitConfirmed && Character && HitCharacter && EquippedWeapon)
	{
		const float Damage = Comfim.bHeadShot ? EquippedWeapon->GetHeadShotDameage() : EquippedWeapon->GetDameage();
		UGameplayStatics::ApplyDamage(HitCharacter, Damage, Character->Controller, EquippedWeapon, UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FServerSideRewindResult Comfim = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);
	AWeapon* EquippedWeapon = Character->GetEquippedWeapon();
	if (Comfim.bHitConfirmed && Character && HitCharacter && EquippedWeapon)
	{
		const float Damage = Comfim.bHeadShot ? EquippedWeapon->GetHeadShotDameage() : EquippedWeapon->GetDameage();
		UGameplayStatics::ApplyDamage(HitCharacter, Damage, Character->Controller, EquippedWeapon, UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	if (Character->Controller == nullptr) return;

	FShotgunServerSideRewindResult Comfim = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr) continue;
		float TotalDamage = 0.f;
		if (Comfim.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Comfim.HeadShots[HitCharacter] * Character->GetEquippedWeapon()->GetHeadShotDameage();
			TotalDamage += HeadShotDamage;
		}
		if (Comfim.BodyShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Comfim.BodyShots[HitCharacter] * Character->GetEquippedWeapon()->GetDameage();
			TotalDamage += HeadShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FrameToChecks;
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FrameToChecks.Add(GetFrameToCheck(HitCharacter, HitTime));
	}
	return ShotgunConfirmHit(FrameToChecks, TraceStart, HitLocations);
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FFramePackage();

	// �ܻ���ɫ��֡��ʷ����
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComp()->FrameHistory;
	// ��ȡ֡������ɵ���ʷʱ��
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	// ��ȡ֡�������µ���ʷʱ��
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	// ���ʱ���������ʱ�䣬ʱ��̫��Զ�޷�����
	if (OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}

	// �Ƿ���Ҫ���в�ֵ
	bool bShouldInterpolate = true;

	FFramePackage FrameToCheck;
	// ��ɵ���ʷʱ��պõ�������ʱ��
	if (OldestHistoryTime == HitTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = History.GetTail()->GetValue();
	}
	// ���µ���ʷʱ��С�ڵ�������ʱ��
	if (NewestHistoryTime <= HitTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = History.GetHead()->GetValue();
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	auto Older = Younger;
	// ���Older��ʱ��Ȼ���ʱ���
	while (Older->GetValue().Time > HitTime)
	{
		// �����... < OlderTime < HitTime < YoungerTime < ...
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	// ��̫���ܣ�����Ҫ���һ��
	if (Older->GetValue().Time == HitTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = Older->GetValue();
	}

	if (bShouldInterpolate)
	{
		// ���� OlderTime < HitTime < YoungerTime ���в�ֵ
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;

	return FrameToCheck;
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrmae, const FFramePackage& YoungerFrame, float HitTime)
{
	// ����֮֡���ʱ��� 
	const float Distance = YoungerFrame.Time - OlderFrmae.Time;
	// �õ���ֵϵ�� InterpFraction�����ϵ�������� HitTime ������֮֡����ռ�ı������������ڼ����ڲ�ֵʱ������֮֡���ĳ�����Ե�ֵ��
	const float InterpFraction = (HitTime - OlderFrmae.Time) / Distance;

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		const FBoxInformation& OlderBox = OlderFrmae.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerPair.Value;

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent; // ���ں��ӵķ�Χ�����С�����Կ���������һ��

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	UWorld* World = GetWorld();
	if (World)
	{
		FFramePackage CurrentFrame;
		// ����boxes�ĵ�ǰλ��
		CacheBoxPositions(HitCharacter, CurrentFrame);
		// �ƶ�������λ��
		MoveBoxes(HitCharacter, Package);
		// ���ý�ɫ�����������ײ
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
		// ����ͷ��box��ײ
		TMap<FName, UBoxComponent*> HitCollisionBoxes = HitCharacter->GetHitCollisionBoxs();
		UBoxComponent* HeadBox = HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
		// ��ȡ���߽���λ��
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		// �������߼��
		FHitResult ConfirmHitResult;
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);
		// �Ƿ������ͷ��box����ͷ��
		if (ConfirmHitResult.bBlockingHit)
		{
			/*if (ConfirmHitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				}
			}*/
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		}
		else
		{
			// û�л���ͷ������ʼ������λ���
			for (auto& HitBoxPair : HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			// �ٴ�����
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);
			if (ConfirmHitResult.bBlockingHit)
			{
				/*if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
					}
				}*/
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
		// Ĭ�ϻָ�
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	}
	return FServerSideRewindResult{ false, false };
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	UWorld* World = GetWorld();
	if (World)
	{
		FFramePackage CurrentFrame;
		// ����boxes�ĵ�ǰλ��
		CacheBoxPositions(HitCharacter, CurrentFrame);
		// �ƶ�������λ��
		MoveBoxes(HitCharacter, Package);
		// ���ý�ɫ�����������ײ
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
		// ����ͷ��box��ײ
		TMap<FName, UBoxComponent*> HitCollisionBoxes = HitCharacter->GetHitCollisionBoxs();
		UBoxComponent* HeadBox = HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

		// �洢Ԥ��·���Ĳ���
		FPredictProjectilePathParams PathParams;
		PathParams.bTraceWithCollision = true;
		PathParams.MaxSimTime = MaxRecordTime;
		PathParams.LaunchVelocity = InitialVelocity;
		PathParams.StartLocation = TraceStart;
		PathParams.SimFrequency = 15.f;
		PathParams.ProjectileRadius = 5.f;
		PathParams.TraceChannel = ECC_HitBox;
		PathParams.ActorsToIgnore.Add(GetOwner());
		PathParams.DrawDebugTime = 5.f;
		PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;
		// �洢Ԥ��·���Ľ��
		FPredictProjectilePathResult PathResult;
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		// �Ƿ������ͷ��box����ͷ��
		if (PathResult.HitResult.bBlockingHit)
		{
			/*if (PathResult.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if (Box)
				{
					DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
				}
			}*/
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{ true, true };
		}
		else
		{
			// û�л���ͷ������ʼ������λ���
			for (auto& HitBoxPair : HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			// �ٴ�Ԥ���䵯·���������ײ
			UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
			if (PathResult.HitResult.bBlockingHit)
			{
				/*if (PathResult.HitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
					if (Box)
					{
						DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
					}
				}*/
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{ true, false };
			}
		}
		// Ĭ�ϻָ�
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	}
	return FServerSideRewindResult{ false, false };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	// ÿ����ɫ����Ч��
	for (auto& Frame : FramePackages)
	{
		if (Frame.Character == nullptr)
		{
			return FShotgunServerSideRewindResult();
		}
	}

	FShotgunServerSideRewindResult ShotgunResult;
	UWorld* World = GetWorld();
	if (World)
	{
		// ����ÿ����ɫ�ĵ�ǰ������
		TArray<FFramePackage> CurrentFrames;
		for (auto& Frame : FramePackages)
		{
			FFramePackage CurrentFrame;
			CurrentFrame.Character = Frame.Character;
			CacheBoxPositions(Frame.Character, CurrentFrame);
			MoveBoxes(Frame.Character, Frame);
			EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
			CurrentFrames.Add(CurrentFrame);
		}
		// ����ÿ����ɫ��ͷ��box��ײ
		for (auto& Frame : FramePackages)
		{
			TMap<FName, UBoxComponent*> HitCollisionBoxes = Frame.Character->GetHitCollisionBoxs();
			UBoxComponent* HeadBox = HitCollisionBoxes[FName("head")];
			HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
		}
		// ����Ƿ����ÿ����ɫͷ��
		for (auto& HitLocation : HitLocations)
		{
			const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
			FHitResult ConfirmHitResult;
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);
			ABlasterCharacter* HitBlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (HitBlasterCharacter)
			{
				/*if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Red, false, 8.f);
					}
				}*/
				if (ShotgunResult.HeadShots.Contains(HitBlasterCharacter))
				{
					ShotgunResult.HeadShots[HitBlasterCharacter]++;
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(HitBlasterCharacter, 1);
				}
			}
		}
		// ����ÿ����ɫ������box��ײ���ر�ͷ��box��ײ
		for (auto& Frame : FramePackages)
		{
			TMap<FName, UBoxComponent*> HitCollisionBoxes = Frame.Character->GetHitCollisionBoxs();
			for (auto& HitBoxPair : HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			UBoxComponent* HeadBox = HitCollisionBoxes[FName("head")];
			HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		// ����Ƿ����ÿ������
		for (auto& HitLocation : HitLocations)
		{
			const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
			FHitResult ConfirmHitResult;
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);
			ABlasterCharacter* HitBlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (HitBlasterCharacter)
			{
				/*if (ConfirmHitResult.Component.IsValid())
				{
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(World, Box->GetComponentLocation(), Box->GetScaledBoxExtent(), FQuat(Box->GetComponentRotation()), FColor::Blue, false, 8.f);
					}
				}*/
				if (ShotgunResult.BodyShots.Contains(HitBlasterCharacter))
				{
					ShotgunResult.BodyShots[HitBlasterCharacter]++;
				}
				else
				{
					ShotgunResult.BodyShots.Emplace(HitBlasterCharacter, 1);
				}
			}
		}
		// �ָ�ÿ����ɫ����ʼ��
		for (auto& CurrentFrame : CurrentFrames)
		{
			ResetHitBoxes(CurrentFrame.Character, CurrentFrame);
			EnableCharacterMeshCollision(CurrentFrame.Character, ECollisionEnabled::QueryAndPhysics);
		}
	}

	return ShotgunResult;
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFrameackage)
{
	if (HitCharacter == nullptr) return;
	TMap<FName, UBoxComponent*> HitCollisionBoxes = HitCharacter->GetHitCollisionBoxs();
	for (auto& HitBoxPair : HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFrameackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	TMap<FName, UBoxComponent*> HitCollisionBoxes = HitCharacter->GetHitCollisionBoxs();
	for (auto& HitBoxPair : HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	TMap<FName, UBoxComponent*> HitCollisionBoxes = HitCharacter->GetHitCollisionBoxs();
	for (auto& HitBoxPair : HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}