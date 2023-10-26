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
		FrameHistory.AddHead(ThisFrame); // 添加在头部
	}
	else
	{
		// 获取最新帧包和最旧帧包之间的时间差
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength > MaxRecordTime)
		{
			// 删除过期的旧包
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

	// 受击角色的帧历史数据
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComp()->FrameHistory;
	// 获取帧数据最旧的历史时间
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	// 获取帧数据最新的历史时间
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	// 最旧时间大于命中时间，时间太久远无法倒带
	if (OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}

	// 是否需要进行差值
	bool bShouldInterpolate = true;

	FFramePackage FrameToCheck;
	// 最旧的历史时间刚好等于命中时间
	if (OldestHistoryTime == HitTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = History.GetTail()->GetValue();
	}
	// 最新的历史时间小于等于命中时间
	if (NewestHistoryTime <= HitTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = History.GetHead()->GetValue();
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	auto Older = Younger;
	// 如果Older的时间比击中时间大
	while (Older->GetValue().Time > HitTime)
	{
		// 结果：... < OlderTime < HitTime < YoungerTime < ...
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	// 不太可能，但需要检查一下
	if (Older->GetValue().Time == HitTime)
	{
		bShouldInterpolate = false;
		FrameToCheck = Older->GetValue();
	}

	if (bShouldInterpolate)
	{
		// 根据 OlderTime < HitTime < YoungerTime 进行插值
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter;

	return FrameToCheck;
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrmae, const FFramePackage& YoungerFrame, float HitTime)
{
	// 两个帧之间的时间差 
	const float Distance = YoungerFrame.Time - OlderFrmae.Time;
	// 得到插值系数 InterpFraction。这个系数代表了 HitTime 在两个帧之间所占的比例，可以用于计算在插值时，两个帧之间的某个属性的值。
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
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent; // 由于盒子的范围不会变小，所以可以任意用一个

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
		// 保存boxes的当前位置
		CacheBoxPositions(HitCharacter, CurrentFrame);
		// 移动到倒带位置
		MoveBoxes(HitCharacter, Package);
		// 禁用角色本身的网格碰撞
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
		// 启用头部box碰撞
		TMap<FName, UBoxComponent*> HitCollisionBoxes = HitCharacter->GetHitCollisionBoxs();
		UBoxComponent* HeadBox = HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
		// 获取射线结束位置
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		// 进行射线检测
		FHitResult ConfirmHitResult;
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECC_HitBox);
		// 是否击中了头部box（爆头）
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
			// 没有击中头部，开始其他部位检测
			for (auto& HitBoxPair : HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			// 再次射线
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
		// 默认恢复
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
		// 保存boxes的当前位置
		CacheBoxPositions(HitCharacter, CurrentFrame);
		// 移动到倒带位置
		MoveBoxes(HitCharacter, Package);
		// 禁用角色本身的网格碰撞
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
		// 启用头部box碰撞
		TMap<FName, UBoxComponent*> HitCollisionBoxes = HitCharacter->GetHitCollisionBoxs();
		UBoxComponent* HeadBox = HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

		// 存储预测路径的参数
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
		// 存储预测路径的结果
		FPredictProjectilePathResult PathResult;
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		// 是否击中了头部box（爆头）
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
			// 没有击中头部，开始其他部位检测
			for (auto& HitBoxPair : HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			// 再次预测射弹路径，检查碰撞
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
		// 默认恢复
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	}
	return FServerSideRewindResult{ false, false };
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	// 每个角色的有效性
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
		// 缓存每个角色的当前桢数据
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
		// 启用每个角色的头部box碰撞
		for (auto& Frame : FramePackages)
		{
			TMap<FName, UBoxComponent*> HitCollisionBoxes = Frame.Character->GetHitCollisionBoxs();
			UBoxComponent* HeadBox = HitCollisionBoxes[FName("head")];
			HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
		}
		// 检测是否击中每个角色头部
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
		// 启用每个角色的身体box碰撞，关闭头部box碰撞
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
		// 检测是否击中每个身体
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
		// 恢复每个角色到初始桢
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