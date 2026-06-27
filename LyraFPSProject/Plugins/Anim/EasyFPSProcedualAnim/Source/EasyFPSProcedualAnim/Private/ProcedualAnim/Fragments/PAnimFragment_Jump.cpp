// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcedualAnim/Fragments/PAnimFragment_Jump.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "FPSAnimGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ProcedualAnim/CurveFloat_ProceduralAnim.h"
#include "ProcedualAnim/FPSProceduralAnimComp.h"

UPAnimFragment_Jump::UPAnimFragment_Jump()
{
	Tag= FPSAnimGameplayTags::Fragment_Jump;
}

void UPAnimFragment_Jump::Initialize(UFPSProceduralAnimComp* InOwnerComp)
{
	Super::Initialize(InOwnerComp);
	
	AutoBindLandEvent();
}

void UPAnimFragment_Jump::Tick(float DeltaTime)
{
	

	//起跳/着陆，采用曲线
	if (DipState > EJumpState::ENone)
	{
		DipUpdate(DeltaTime);
	}
	//In Air
	else
	{
		InAirCheck();
	}
	Offset.Z = CurDipOffset;
	
	Super::Tick(DeltaTime);
}

void UPAnimFragment_Jump::ReceiveSignal(const FGameplayTag& Signal)
{
	if (Signal == FPSAnimGameplayTags::Signal_Jump)
	{
		OnStartJump();
	}
	if (Signal == FPSAnimGameplayTags::Signal_Land)
	{
		FHitResult Hit;
		OnLanded(Hit);
	}
}

void UPAnimFragment_Jump::AutoBindLandEvent()
{
	if (OwnerChar)
	{
		OwnerChar->LandedDelegate.AddDynamic(this, &ThisClass::OnLanded);
	}
}

bool UPAnimFragment_Jump::SetData(UDataAsset_ProceduralAnim* InData)
{
	if (auto newData=Cast<UDataAsset_Anim_Jump>(InData))
	{
		Data=newData;
		return true;
	}
	return false;
}

void UPAnimFragment_Jump::OnStartJump()
{
	CurDipOffset = 0.f;
	if (Data)
	{
		Dip(Data->JumpCurve, Data->JumpDipTime, Data->JumpTargetZ, EJumpState::EJump);
		if (OwnerChar)
		{
			SendEvent(FPSAnimGameplayTags::Event_JumpStart,FGameplayEventData());
		}
	}
}

void UPAnimFragment_Jump::InAirCheck()
{
	if (Data && OwnerMoveComp && OwnerMoveComp->MovementMode == MOVE_Falling)
	{
		if (DipState != EJumpState::EInAir)
		{
			Dip(Data->InAirCurve, Data->MaxInAirTime, Data->InAirTarget, EJumpState::EInAir);
			if (OwnerChar)
			{
				SendEvent(FPSAnimGameplayTags::Event_InAir,FGameplayEventData());
			}
		}
	}
}

// PRAGMA_DISABLE_OPTIMIZATION

void UPAnimFragment_Jump::OnLanded(const FHitResult& Hit)
{
	if (OwnerMoveComp&&Data)
	{
		float strengthRate = 1.f;
		float timeRate = 1.f;
		float jumpVelocityZ = FVector(0, 0, OwnerMoveComp->GetLastUpdateVelocity().Z).Length();
		float velocityRate = abs(jumpVelocityZ / OwnerMoveComp->JumpZVelocity);
		strengthRate = velocityRate;
		//超速时，计算Strength乘数
		if (velocityRate > 1.f)
		{
			float velocity_Overflow = velocityRate - 1;
			//1-1/(x+1)->[0,+∞)->[0,1]
			float velocity_Add = 1 - Data->LandDropTimeCurvePower / (velocity_Overflow + Data->LandDropTimeCurvePower);
			timeRate = 1 + velocity_Add * (Data->LandDropTimeRate - 1);
			strengthRate = 1 + velocity_Add * (Data->LandDropStrengthRate - 1);
		}
		float strength = Data->LandDipStrength * strengthRate;
		// 从曲线strength计算体感strength
		if (Data->LandCurve)
		{
			FVector2D curveValueRange;
			Data->LandCurve->GetValueRange(curveValueRange.X, curveValueRange.Y);
			//curveValueRange>=1 
			float overFlow = (curveValueRange.Y - 1) * (strength - 1);
			strength = 1 + overFlow;
		}
		Dip(Data->LandCurve, Data->LandDipTime * timeRate, 0, EJumpState::ELand, strength);
		if (OwnerChar)
		{
			SendEvent(FPSAnimGameplayTags::Event_Landed,FGameplayEventData());
		}
	}
}

// PRAGMA_ENABLE_OPTIMIZATION

void UPAnimFragment_Jump::Dip(UCurveFloat_ProceduralAnim* Curve, float Time, float Target, EJumpState State,
                              float strength)
{
	if (Time > 0.f)
	{
		DipTotalTime = Time;
		DipCurve = Curve;
		DipTarget = Target;
		DipState = State;
		DipStrength = strength;
		DipStart = CurDipOffset;
		CurDipTime = 0.f;
		
		bCurveIsOverflow=false;
		if (DipCurve)
		{
			FVector2D curveValueRange;
			DipCurve->GetValueRange(curveValueRange.X, curveValueRange.Y);
			if (curveValueRange.Y > 1.f)
			{
				bCurveIsOverflow = true;
			}
		}
		
		TotalBlendTime=-1.f;
		DipStrengthBlendAlpha=0.f;
	}
}

void UPAnimFragment_Jump::DipUpdate(float DeltaSeconds)
{
	if (DipState > EJumpState::ENone)
	{
		DeltaSeconds *= 1 / DipTotalTime;
		CurDipTime += DeltaSeconds;
		DipAlpha = DipCurve ? DipCurve->GetFloatValue(CurDipTime) : CurDipTime;
		
		//Land曲线
		//保证曲线结果一定是1.问题：无法全域*DipStrength
		//方案：越接近1 Strength影响越小
		if (DipStrength!=1.f)
		{
			//Land曲线
			if (bCurveIsOverflow)
			{
				if (DipCurve->GetVelocity(CurDipTime) <= 0 ||TotalBlendTime!=-1.f)
				{
					if (TotalBlendTime==-1.f)
					{
						TotalBlendTime= 1 - CurDipTime;
					}
					DipStrengthBlendAlpha = (TotalBlendTime - (1-CurDipTime))/TotalBlendTime;
				}
				DipAlpha *= FMath::Lerp(DipStrength, 1, DipStrengthBlendAlpha);
			}
			else
			{
				//普通曲线
				DipAlpha *= FMath::Pow(FMath::Lerp(DipStrength, 1, CurDipTime), 3);
			}
		}
		
		CurDipOffset = FMath::Lerp(DipStart, DipTarget, DipAlpha);
	}

	if (CurDipTime >= 1.f)
	{
		DipState = EJumpState::ENone;
	}
}

