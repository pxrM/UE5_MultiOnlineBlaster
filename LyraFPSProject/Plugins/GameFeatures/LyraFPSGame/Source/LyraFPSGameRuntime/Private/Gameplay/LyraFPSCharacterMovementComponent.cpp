#include "Gameplay/LyraFPSCharacterMovementComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "GameplayAbilitySystem/LyraFPSGameplayTags.h"

ULyraFPSCharacterMovementComponent::ULyraFPSCharacterMovementComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PredictedMoveFlags = 0;
	PredictedSpeedScale = 1.0f;

	WalkSpeedMultiplier = 0.5f;
	JogSpeedMultiplier = 1.0f;
	SprintSpeedMultiplier = 1.4f;
	ADSSpeedMultiplier = 0.8f;
	CrouchSpeedMultiplier = 0.55f;

	MinPredictedSpeedScale = 0.0f;
	MaxPredictedSpeedScale = 3.0f;

	MovementDisabledTag = LyraFPSGameplayTags::Status_MovementDisabled;
	KnockedDownTag = LyraFPSGameplayTags::Status_KnockedDown;
	StunnedTag = LyraFPSGameplayTags::Status_Stunned;

	SetNetworkMoveDataContainer(LyraNetworkMoveDataContainer);
}

void ULyraFPSCharacterMovementComponent::SetPredictedMoveFlag(uint8 Flag, bool bEnabled)
{
	if (bEnabled)
	{
		PredictedMoveFlags |= Flag;
	}
	else
	{
		PredictedMoveFlags &= ~Flag;
	}

	//阻止ADS和Sprint一起触发，但放在GA里就行
	const bool bADS = (PredictedMoveFlags & FLyraFPSPredictedMoveFlags::ADS) != 0;
	const bool bSprint = (PredictedMoveFlags & FLyraFPSPredictedMoveFlags::Sprint) != 0;
	if (bADS && bSprint)
	{
		// ADS 优先，避免开镜时仍然疾跑。
		PredictedMoveFlags &= ~FLyraFPSPredictedMoveFlags::Sprint;
	}
}

void ULyraFPSCharacterMovementComponent::ClearPredictedMoveFlags()
{
	PredictedMoveFlags = 0;
	PredictedSpeedScale = 1.0f;
}

bool ULyraFPSCharacterMovementComponent::HasPredictedMoveFlag(uint8 Flag) const
{
	return (PredictedMoveFlags & Flag) != 0;
}


void ULyraFPSCharacterMovementComponent::SetPredictedSpeedScale(float NewSpeedScale)
{
	PredictedSpeedScale = FMath::Clamp(NewSpeedScale, MinPredictedSpeedScale, MaxPredictedSpeedScale);
}

void ULyraFPSCharacterMovementComponent::StartSprint()
{
	SetPredictedGaitFlag(FLyraFPSPredictedMoveFlags::Sprint);
}

void ULyraFPSCharacterMovementComponent::StopSprint()
{
	SetPredictedGaitFlag(FLyraFPSPredictedMoveFlags::None);
}

void ULyraFPSCharacterMovementComponent::StartADS()
{
	SetPredictedMoveFlag(FLyraFPSPredictedMoveFlags::ADS, true);
}

void ULyraFPSCharacterMovementComponent::StopADS()
{
	SetPredictedMoveFlag(FLyraFPSPredictedMoveFlags::ADS, false);
}

void ULyraFPSCharacterMovementComponent::SetPredictedGaitFlag(uint8 NewGaitFlag)
{
	constexpr uint16 GaitMask =
		FLyraFPSPredictedMoveFlags::Walk |
		FLyraFPSPredictedMoveFlags::Sprint;

	PredictedMoveFlags &= ~GaitMask;
	PredictedMoveFlags |= NewGaitFlag;
}

void ULyraFPSCharacterMovementComponent::ApplyNetworkMoveData(uint8 InMoveFlags,  uint16 InPackedSpeedScale)
{
	PredictedMoveFlags = InMoveFlags;
	PredictedSpeedScale = UnpackPredictedSpeedScale(InPackedSpeedScale);
}

float ULyraFPSCharacterMovementComponent::GetMaxSpeed() const
{
	float ResultSpeed = Super::GetMaxSpeed();

	if (ResultSpeed <= 0.0f)
	{
		return ResultSpeed;
	}

	// 这类是最终硬性兜底，不是 Sprint/ADS 规则裁决。
	// 是否保留取决于你的项目设计。
	if (IsMovementBlockedByGAS())
	{
		return 0.0f;
	}

	ResultSpeed *= GetSpeedScaleFromGAS();
	ResultSpeed *= GetSpeedScaleFromPredictedMoveFlags();
	ResultSpeed *= PredictedSpeedScale>0.f?PredictedSpeedScale:1.f;
	return ResultSpeed;
}

float ULyraFPSCharacterMovementComponent::GetSpeedScaleFromPredictedMoveFlags() const
{
	float Scale = 1.0f;

	if (HasPredictedMoveFlag(FLyraFPSPredictedMoveFlags::Walk))
	{
		Scale *= WalkSpeedMultiplier;
	}
	else if (HasPredictedMoveFlag(FLyraFPSPredictedMoveFlags::Sprint))
	{
		Scale *= SprintSpeedMultiplier;
	}
	else
	{
		Scale *= JogSpeedMultiplier;
	}

	
	if (HasPredictedMoveFlag(FLyraFPSPredictedMoveFlags::ADS))
	{
		Scale *= ADSSpeedMultiplier;
	}
	if (HasPredictedMoveFlag(FLyraFPSPredictedMoveFlags::Crouch))
	{
		Scale *= CrouchSpeedMultiplier;
	}
	

	return FMath::Max(0.0f, Scale);
}

float ULyraFPSCharacterMovementComponent::GetSpeedScaleFromGAS() const
{
	if (IsKnockedDownByGAS())
	{
		return KnockedDownSpeedMultiplier;
	}

	return 1.0f;
}

FNetworkPredictionData_Client* ULyraFPSCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (ClientPredictionData == nullptr)
	{
		ULyraFPSCharacterMovementComponent* MutableThis = const_cast<ULyraFPSCharacterMovementComponent*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_LyraFPS(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.0f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.0f;
	}
	return ClientPredictionData;
}

UAbilitySystemComponent* ULyraFPSCharacterMovementComponent::GetOwnerAbilitySystemComponent() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return nullptr;
	}

	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(OwnerActor);
	if (!AbilitySystemInterface)
	{
		return nullptr;
	}

	return AbilitySystemInterface->GetAbilitySystemComponent();
}

bool ULyraFPSCharacterMovementComponent::HasOwnerMatchingGameplayTag(const FGameplayTag& Tag) const
{
	if (!Tag.IsValid())
	{
		return false;
	}

	const UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
	return ASC && ASC->HasMatchingGameplayTag(Tag);
}

int32 ULyraFPSCharacterMovementComponent::GetOwnerGameplayTagCount(const FGameplayTag& Tag) const
{
	if (!Tag.IsValid())
	{
		return 0;
	}

	const UAbilitySystemComponent* ASC = GetOwnerAbilitySystemComponent();
	return ASC ? ASC->GetTagCount(Tag) : 0;
}

bool ULyraFPSCharacterMovementComponent::IsMovementBlockedByGAS() const
{
	if (HasOwnerMatchingGameplayTag(MovementDisabledTag))
	{
		return true;
	}

	if (HasOwnerMatchingGameplayTag(StunnedTag))
	{
		return true;
	}

	return false;
}

bool ULyraFPSCharacterMovementComponent::IsKnockedDownByGAS() const
{
	return HasOwnerMatchingGameplayTag(KnockedDownTag);
}

uint16 ULyraFPSCharacterMovementComponent::PackPredictedSpeedScale(float InPredictedSpeedScale)
{
	const float Clamped = FMath::Clamp(InPredictedSpeedScale, 0.0f, 10.0f);

	// 1000 表示 1.0
	// 1400 表示 1.4
	// 800 表示 0.8
	return static_cast<uint16>(FMath::RoundToInt(Clamped * 1000.0f));
}

float ULyraFPSCharacterMovementComponent::UnpackPredictedSpeedScale(uint16 InPredictedPackedSpeedScale)
{
	return static_cast<float>(InPredictedPackedSpeedScale) / 1000.0f;
}

// ============================================================================
// FSavedMove_LyraAdvanced
// ============================================================================

void ULyraFPSCharacterMovementComponent::FSavedMove_LyraFPS::Clear()
{
	Super::Clear();

	SavedPredictedMoveFlags = 0;
	SavedPackedSpeedScale = 1000;
}

uint8 ULyraFPSCharacterMovementComponent::FSavedMove_LyraFPS::GetCompressedFlags() const
{
	// 这里仍然返回 Super 的原生 Flags。
	// 自定义复杂数据不再塞进 FLAG_Custom_0/1/2/3。
	return Super::GetCompressedFlags();
}

bool ULyraFPSCharacterMovementComponent::FSavedMove_LyraFPS::CanCombineWith(
	const FSavedMovePtr& NewMove,
	ACharacter* Character,
	float MaxDelta
) const
{
	const FSavedMove_LyraFPS* NewLyraMove = static_cast<const FSavedMove_LyraFPS*>(NewMove.Get());

	if (SavedPredictedMoveFlags != NewLyraMove->SavedPredictedMoveFlags)
	{
		return false;
	}

	if (SavedPackedSpeedScale != NewLyraMove->SavedPackedSpeedScale)
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, Character, MaxDelta);
}

void ULyraFPSCharacterMovementComponent::FSavedMove_LyraFPS::SetMoveFor(
	ACharacter* Character,
	float InDeltaTime,
	const FVector& NewAccel,
	FNetworkPredictionData_Client_Character& ClientData
)
{
	Super::SetMoveFor(Character, InDeltaTime, NewAccel, ClientData);

	const ULyraFPSCharacterMovementComponent* MovementComponent =
		Cast<ULyraFPSCharacterMovementComponent>(Character->GetCharacterMovement());

	if (MovementComponent)
	{
		SavedPredictedMoveFlags = MovementComponent->PredictedMoveFlags;
		SavedPackedSpeedScale = PackPredictedSpeedScale(MovementComponent->PredictedSpeedScale);
	}
}

void ULyraFPSCharacterMovementComponent::FSavedMove_LyraFPS::PrepMoveFor(ACharacter* Character)
{
	Super::PrepMoveFor(Character);

	ULyraFPSCharacterMovementComponent* MovementComponent =
		Cast<ULyraFPSCharacterMovementComponent>(Character->GetCharacterMovement());

	if (MovementComponent)
	{
		MovementComponent->ApplyNetworkMoveData(
			SavedPredictedMoveFlags,
			SavedPackedSpeedScale
		);
	}
}

// ============================================================================
// FNetworkPredictionData_Client_LyraAdvanced
// ============================================================================

ULyraFPSCharacterMovementComponent::FNetworkPredictionData_Client_LyraFPS::FNetworkPredictionData_Client_LyraFPS(
	const UCharacterMovementComponent& ClientMovement
)
	: Super(ClientMovement)
{
}

FSavedMovePtr ULyraFPSCharacterMovementComponent::FNetworkPredictionData_Client_LyraFPS::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_LyraFPS());
}

// ============================================================================
// FCharacterNetworkMoveData_LyraAdvanced
// ============================================================================

void ULyraFPSCharacterMovementComponent::FCharacterNetworkMoveData_LyraFPS::ClientFillNetworkMoveData(
	const FSavedMove_Character& ClientMove,
	ENetworkMoveType MoveType
)
{
	Super::ClientFillNetworkMoveData(ClientMove, MoveType);

	const FSavedMove_LyraFPS& LyraMove = static_cast<const FSavedMove_LyraFPS&>(ClientMove);

	MoveFlags = LyraMove.SavedPredictedMoveFlags;
	PackedSpeedScale = LyraMove.SavedPackedSpeedScale;
}

bool ULyraFPSCharacterMovementComponent::FCharacterNetworkMoveData_LyraFPS::Serialize(
	UCharacterMovementComponent& CharacterMovement,
	FArchive& Ar,
	UPackageMap* PackageMap,
	ENetworkMoveType MoveType
)
{
	bool bLocalSuccess = Super::Serialize(CharacterMovement, Ar, PackageMap, MoveType);

	Ar.SerializeBits(&MoveFlags, 8);
	Ar << PackedSpeedScale;

	if (Ar.IsLoading())
	{
		if (ULyraFPSCharacterMovementComponent* LyraMoveComp = Cast<ULyraFPSCharacterMovementComponent>(&CharacterMovement))
		{
			LyraMoveComp->ApplyNetworkMoveData(MoveFlags, PackedSpeedScale);
		}
	}

	return bLocalSuccess && !Ar.IsError();
}

// ============================================================================
// FCharacterNetworkMoveDataContainer_LyraAdvanced
// ============================================================================

ULyraFPSCharacterMovementComponent::FCharacterNetworkMoveDataContainer_LyraFPS::FCharacterNetworkMoveDataContainer_LyraFPS()
{
	NewMoveData = &MoveData[0];
	PendingMoveData = &MoveData[1];
	OldMoveData = &MoveData[2];
}