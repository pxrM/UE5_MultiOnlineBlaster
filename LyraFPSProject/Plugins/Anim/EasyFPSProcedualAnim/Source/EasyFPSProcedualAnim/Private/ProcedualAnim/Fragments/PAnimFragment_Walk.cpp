// Fill out your copyright notice in the Description page of Project Settings.


#include "ProcedualAnim/Fragments/PAnimFragment_Walk.h"

#include "FPSAnimGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "ProcedualAnim/CurveFloat_ProceduralAnim.h"


UPAnimFragment_Walk::UPAnimFragment_Walk()
{
	Tag = FPSAnimGameplayTags::Fragment_Walk;
}

void UPAnimFragment_Walk::Initialize(UFPSProceduralAnimComp* InOwnerComp)
{
	Super::Initialize(InOwnerComp);
	RefreshShakeParams();
}

void UPAnimFragment_Walk::Tick(float DeltaTime)
{
	
	Offset = FVector::ZeroVector;
	Rot = FRotator::ZeroRotator;
	if (Data&& OwnerChar)
	{
		UpdateShakes(DeltaTime);
		UpdateLag_RealTime(DeltaTime);
	}
	
	Super::Tick(DeltaTime);
}

void UPAnimFragment_Walk::ReceiveSignal(const FGameplayTag& Signal)
{
}

bool UPAnimFragment_Walk::SetData(UDataAsset_ProceduralAnim* InData)
{
	if (auto newData = Cast<UDataAsset_Anim_Walk>(InData))
	{
		Data = newData;
		RefreshShakeParams();
		return true;
	}
	return false;
}

void UPAnimFragment_Walk::RefreshShakeParams()
{
	ShakeParams.Reset();
	ShakeValues.Reset();

	if (!Data)
	{
		return;
	}

	ShakeParams.Add(&Data->Shake_X);
	ShakeParams.Add(&Data->Shake_Y);
	ShakeParams.Add(&Data->Shake_Z);
	ShakeParams.Add(&Data->Shake_Rot);
	ShakeValues.SetNumZeroed(ShakeParams.Num());
}

void UPAnimFragment_Walk::UpdateShakes(float DeltaTime)
{
	if (!Data||!Data->bEnableShake)
	{
		return;
	}
	
	float alphaTarget = 0.f;
	if (auto moveComp = OwnerChar->GetCharacterMovement())
	{
		if (moveComp->MovementMode == MOVE_NavWalking || moveComp->MovementMode == MOVE_Walking)
		{
			alphaTarget=UKismetMathLibrary::NormalizeToRange(
					OwnerChar->GetVelocity().Length(), 0.f, Data->StandardWalkSpeed);
		}
	}
	//防止出现撞墙突然停止的情况
	//方案1：低通滤波->仅针对变化过快的情况
	if (Data->bShakeUseLowpass)
	{
		float alpha = FMath::Clamp(DeltaTime / Data->ShakeLowpassThreshold, 0.f, 1.f);
		AnimPlaySpeedAlpha = FMath::Lerp(AnimPlaySpeedAlpha, alphaTarget, alpha);
	}
	else
	{
		AnimPlaySpeedAlpha=alphaTarget;
	}
	//方案2：Interp->所有速度变化都受影响
	//无法很好判断和同时处理仍有较小速度的情况
	// if (!Data->ShakeInterpOnlyStop|| alphaTarget==0)
	// {
	// 	AnimPlaySpeedAlpha=FMath::FInterpTo(AnimPlaySpeedAlpha,alphaTarget,DeltaTime,8.f);
	// }
	// else
	// {
	// 	AnimPlaySpeedAlpha=alphaTarget;
	// }
	
	if (ShakeValues.Num() != ShakeParams.Num())
	{
		ShakeValues.SetNumZeroed(ShakeParams.Num());
	}

	for (int i = 0; i < ShakeParams.Num(); i++)
	{
		auto param = ShakeParams[i];
		if (!param || !param->Curve)
		{
			ShakeValues[i] = FProceduralWalkValues();
			continue;
		}

		ShakeValues[i].Time += DeltaTime * Data->ShakeFrequency * param->Frequency * AnimPlaySpeedAlpha;
		ShakeValues[i].Alpha = param->Curve->GetFloatValue(ShakeValues[i].Time + param->Offset);
		ShakeValues[i].Value = FMath::Lerp(param->Range.X, param->Range.Y, ShakeValues[i].Alpha)
			* AnimPlaySpeedAlpha * Data->ShakeStrength;

		if (!FMath::IsFinite(ShakeValues[i].Value))
		{
			ShakeValues[i] = FProceduralWalkValues();
		}
	}

	if (ShakeValues.Num() >= 4)
	{
		Offset += FVector(ShakeValues[0].Value, ShakeValues[1].Value, ShakeValues[2].Value);
		Rot += FRotator(ShakeValues[3].Value, 0.f, 0.f);
	}
}

void UPAnimFragment_Walk::UpdateLag_RealTime(float DeltaTime)
{
	
	if (OwnerChar&&Data&&Data->bEnableLag)
	{
		float forward = OwnerChar->GetVelocity().Dot(OwnerChar->GetActorForwardVector());
		float right = OwnerChar->GetVelocity().Dot(OwnerChar->GetActorRightVector());
		float up = OwnerChar->GetVelocity().Dot(OwnerChar->GetActorUpVector());
		FVector lagAlphaTarget=FVector(
		right/Data->StandardWalkSpeed,
		forward/(Data->StandardWalkSpeed*-1),
		up/Data->StandardWalkSpeed
		);
		//Todo: 全部一锅炖interp是否应该改为单独控制？
		//保证是突然减速
		if (Data->bLagUseLowpass)
		{
			float alpha = FMath::Clamp(DeltaTime / Data->LagLowpassThreshold, 0.f, 1.f);
			LagAlphaTarget = FMath::Lerp(LagAlphaTarget, lagAlphaTarget, alpha);
		}
		else
		{
			LagAlphaTarget=lagAlphaTarget;
		}
		
		float alpha = FMath::Clamp(DeltaTime / Data->LagOffsetLengthLowpassThreshold, 0.f, 1.f);
		LagAlpha_Length = FMath::Lerp(LagAlpha_Length, lagAlphaTarget, alpha);
		
		LagOffset=FVector::ZeroVector;
		LagRot=FRotator::ZeroRotator;
		if (Data->bEnableSpringInterp)
		{
			// FVector alhpaTarget = LagAlphaTarget;
			// FVector lagOffsetTarget;
			// lagOffsetTarget+=Data->LagOffset_Right*FVector(alhpaTarget.X);
			// lagOffsetTarget+=Data->LagOffset_Forward*FVector(alhpaTarget.Y);
			// lagOffsetTarget+=Data->LagOffset_Up*FVector(alhpaTarget.Z);
			// UKismetMathLibrary::VectorSpringInterp(
			// 	LagOffset,lagOffsetTarget,LagSpringState,Data->Stiffness,Data->CriticalDampingFactor,
			// 	DeltaTime,Data->Mass,Data->TargetVelocityAmount);
			
			LagAlpha = UKismetMathLibrary::VectorSpringInterp(
				LagAlpha,LagAlphaTarget,LagSpringState,Data->Stiffness,Data->CriticalDampingFactor,
				DeltaTime,Data->Mass,Data->TargetVelocityAmount);
			// lagOffsetTarget+=Data->LagOffset_Right*FVector(alhpaTarget.X);
			// lagOffsetTarget+=Data->LagOffset_Forward*FVector(alhpaTarget.Y);
			// lagOffsetTarget+=Data->LagOffset_Up*FVector(alhpaTarget.Z);
			
		}
		else
		{
			LagAlpha=LagAlphaTarget;
			
		}
		LagOffset += FMath::Lerp(FVector::ZeroVector,Data->LagOffset_Right,LagAlpha.X); 
		LagOffset += FMath::Lerp(FVector::ZeroVector,Data->LagOffset_Forward,LagAlpha.Y); 
		LagOffset += FMath::Lerp(FVector::ZeroVector,Data->LagOffset_Up,LagAlpha.Z); 
		LagOffset += FMath::Lerp(FVector::ZeroVector,Data->LagOffset_Length,LagAlphaTarget.Length()); 
		LagRot += FMath::Lerp(FRotator::ZeroRotator,Data->LagRot_Right,LagAlpha.X); 
		LagRot += FMath::Lerp(FRotator::ZeroRotator,Data->LagRot_Forward,LagAlpha.Y); 
		LagRot += FMath::Lerp(FRotator::ZeroRotator,Data->LagRot_Up,LagAlpha.Z); 
		
		LagOffset=UKismetMathLibrary::ClampVectorSize(LagOffset,0.f,Data->LagMaxLength);
		Offset += LagOffset;
		Rot += LagRot;
	}
}
