// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcedualAnim/ProceduralAnimFragment.h"

#include "GameFramework/Character.h"
#include "ProcedualAnim/FPSProceduralAnimComp.h"
#include "ProcedualAnim/DataAsset_ProceduralAnim.h"



void UProceduralAnimFragment::Initialize(UFPSProceduralAnimComp* InOwnerComp)
{
	OwnerComp=InOwnerComp;
	if (!OwnerComp)
	{
		return;
	}

	OwnerChar = Cast<ACharacter>(OwnerComp->GetOwner());
	OwnerMoveComp = OwnerChar ? OwnerChar->GetCharacterMovement() : nullptr;
}

void UProceduralAnimFragment::Tick(float DeltaTime)
{
}


void UProceduralAnimFragment::SendEvent(FGameplayTag EventTag, FGameplayEventData Payload)
{
	OwnerComp->SendEvent(EventTag,Payload);
}

FVector UProceduralAnimFragment::GetOffset(FGameplayTag TargetType)
{
	
	if (GetData())
	{
		if (TargetType!=FGameplayTag::EmptyTag)
		{
			if (float* ratePtr=GetData()->TargetOffsetRate.Find(TargetType))
			{
				return Offset * (*ratePtr);
			}
			else
			{
				return FVector::ZeroVector;
			}
		}
	}
	return Offset;
}

FRotator UProceduralAnimFragment::GetRot(FGameplayTag TargetType)
{
	if (GetData())
	{
		if (TargetType!=FGameplayTag::EmptyTag)
		{
			if (float* ratePtr=GetData()->TargetOffsetRate.Find(TargetType))
			{
				return Rot * (*ratePtr);
			}
			else
			{
				return FRotator::ZeroRotator;
			}
		}
	}
	return Rot;
}

float UProceduralAnimFragment::GetAlpha()
{
	return Alpha;
}


