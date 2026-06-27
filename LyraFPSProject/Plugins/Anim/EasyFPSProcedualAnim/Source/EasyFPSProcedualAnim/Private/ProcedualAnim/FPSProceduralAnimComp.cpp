// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcedualAnim/FPSProceduralAnimComp.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "ProcedualAnim/ProceduralAnimFragment.h"


// Sets default values for this component's properties
UFPSProceduralAnimComp::UFPSProceduralAnimComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UFPSProceduralAnimComp::InitializeComponent()
{
	Super::InitializeComponent();
	// UE_LOG(LogTemp,Warning,TEXT("UFPSProceduralAnimComp Init"));
	
}


// Called when the game starts
void UFPSProceduralAnimComp::BeginPlay()
{
	Super::BeginPlay();
	
	for (auto tmpFragment:Fragments)
	{
		if (tmpFragment)
		{
			tmpFragment->Initialize(this);
		}
	}
}


// Called every frame
void UFPSProceduralAnimComp::TickComponent(float DeltaTime, ELevelTick TickType,
                                           FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	for (auto tmpFragment:Fragments)
	{
		if (tmpFragment)
		{
			tmpFragment->Tick(DeltaTime);
		}
	}
}

void UFPSProceduralAnimComp::AddFragments(UProceduralAnimFragment* NewFragments)
{
	Fragments.Add(NewFragments);
	NewFragments->Initialize(this);
}

void UFPSProceduralAnimComp::SendSignal(const FGameplayTag Signal)
{
	for (auto tmpFragment:Fragments)
	{
		if (tmpFragment)
		{
			tmpFragment->ReceiveSignal(Signal);
		}
	}
}

void UFPSProceduralAnimComp::InitInhanceInput()
{
	if (GetOwner())
	{
		//直接绑定InputComp
		// auto inputComp= GetOwner()->FindComponentByClass<UEnhancedInputComponent>();
		// UEnhancedInputComponent* EnhancedIC =
	 //   Cast<UEnhancedInputComponent>(PlayerInputComponent);
	 //
		// check(EnhancedIC);
	 //
		// EnhancedIC->BindAction(IA_Move,ETriggerEvent::Triggered, this, &AFPSCharacter::HandleMove);
	 //
		// EnhancedIC->BindAction(IA_Move,ETriggerEvent::Completed, this, &AFPSCharacter::HandleMove);
	}
	
}

void UFPSProceduralAnimComp::SendEvent(FGameplayTag EventTag, FGameplayEventData Payload) const
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(),EventTag,Payload);
	AnimEvent.Broadcast(EventTag,Payload);
}

FVector UFPSProceduralAnimComp::GetTargetOffset(FGameplayTag FragmentTag,FGameplayTag TargetType)
{
	FVector offset = FVector::ZeroVector;
	for (auto tmpFragment:Fragments)
	{
		if (tmpFragment)
		{
			if (FragmentTag != FGameplayTag::EmptyTag && !FragmentTag.MatchesTag(tmpFragment->GetTag()))
			{
				continue;
			}

			const FVector fragmentOffset = tmpFragment->GetOffset(TargetType);
			if (!fragmentOffset.ContainsNaN())
			{
				offset += fragmentOffset;
			}
		}
	}
	return offset;
}

FRotator UFPSProceduralAnimComp::GetTargetRot(FGameplayTag FragmentTag, FGameplayTag TargetType)
{
	FRotator outRot = FRotator::ZeroRotator;
	for (auto tmpFragment:Fragments)
	{
		if (tmpFragment)
		{
			//有Tag->根据Tag决定
			if (FragmentTag!=FGameplayTag::EmptyTag)
			{
				if (FragmentTag.MatchesTag(tmpFragment->GetTag()))
				{
					const FRotator fragmentRot = tmpFragment->GetRot(TargetType);
					if (!fragmentRot.ContainsNaN())
					{
						outRot += fragmentRot;
					}
				}
			}
			//直接添加
			else
			{
				const FRotator fragmentRot = tmpFragment->GetRot(TargetType);
				if (!fragmentRot.ContainsNaN())
				{
					outRot += fragmentRot;
				}
			}
		}
	}
	return outRot;
}

UAnimMontage* UFPSProceduralAnimComp::GetTargetMontage(FGameplayTag FragmentTag)
{
	for (auto tmpFragment:Fragments)
	{
		if (tmpFragment)
		{
			if (FragmentTag!=FGameplayTag::EmptyTag)
			{
				if (FragmentTag.MatchesTag(tmpFragment->GetTag()))
				{
					return tmpFragment->GetMontage();
				}
			}
		}
	}
	return nullptr;
}

float UFPSProceduralAnimComp::GetTargetAlpha(FGameplayTag FragmentTag)
{
	if (FragmentTag!=FGameplayTag::EmptyTag)
	{
		if (auto tmpFrag=FindFragment(FragmentTag))
		{
			return tmpFrag->GetAlpha();
		}
	}
	return 0.f;
}

void UFPSProceduralAnimComp::UpdateData(TMap<FGameplayTag, UDataAsset_ProceduralAnim*>& Data)
{
	for (auto tmpFragment:Fragments)
	{
		if (ensureAlways(tmpFragment))
		{
			if (auto data = Data.Find(tmpFragment->Tag) )
			{
				tmpFragment->SetData(*data);
			}
		}
	}
}

UProceduralAnimFragment* UFPSProceduralAnimComp::FindFragment(FGameplayTag FragmentTag)
{
	for (auto tmpFragment:Fragments)
	{
		if (tmpFragment)
		{
			if (tmpFragment->Tag==FragmentTag)
			{
				return tmpFragment;
			}
		}
	}
	return nullptr;
}
