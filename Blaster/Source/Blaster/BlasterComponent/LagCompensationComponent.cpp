// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"

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
	ShowFramePackage(Package, FColor::Orange);
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

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime, AWeapon* DamageCauser)
{
	FServerSideRewindResult Comfim = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime); 
	if (Comfim.bHitConfirmed && Character && HitCharacter && DamageCauser)
	{
		UGameplayStatics::ApplyDamage(HitCharacter, DamageCauser->GetDameage(), Character->Controller, DamageCauser, UDamageType::StaticClass());
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComp() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComp()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FServerSideRewindResult();

	// �ܻ���ɫ��֡��ʷ����
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComp()->FrameHistory;
	// ��ȡ֡������ɵ���ʷʱ��
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	// ��ȡ֡�������µ���ʷʱ��
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	// ���ʱ���������ʱ�䣬ʱ��̫��Զ�޷�����
	if (OldestHistoryTime > HitTime)
	{
		return FServerSideRewindResult();
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

	// ����OlderTime < HitTime < YoungerTime���в�ֵ
	if (bShouldInterpolate)
	{
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
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

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocaton)
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
		// ��ȡ���߽���λ��
		const FVector TraceEnd = TraceStart + (HitLocaton - TraceStart) * 1.25f;
		// ���ý�ɫ�����������ײ
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);
		// ����ͷ��box��ײ
		TMap<FName, UBoxComponent*> HitCollisionBoxes = HitCharacter->GetHitCollisionBoxs();
		UBoxComponent* HeadBox = HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		// �������߼��
		FHitResult ConfirmHitResult;
		World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);
		// �Ƿ������ͷ��box����ͷ��
		if (ConfirmHitResult.bBlockingHit)
		{
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
					HitBoxPair.Value->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
				}
			}
			// �ٴ�����
			World->LineTraceSingleByChannel(ConfirmHitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility);
			if (ConfirmHitResult.bBlockingHit)
			{
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