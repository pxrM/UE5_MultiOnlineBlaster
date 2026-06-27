// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/LyraFPSCameraMode.h"

#include "FPSAnimGameplayTags.h"
#include "Gameplay/LyraFPSCharacter.h"
#include "ProcedualAnim/FPSProceduralAnimComp.h"


float ULyraFPSCameraMode::GetHeadCameraShakeAdditiveRatio() const
{
	return headCameraShadeRatio;
}




namespace LyraCameraMode_ThirdPerson_Statics
{
	static const FName NAME_IgnoreCameraCollision = TEXT("IgnoreCameraCollision");
}

ULyraFPSCameraMode::ULyraFPSCameraMode()
{
}

void ULyraFPSCameraMode::UpdateView(float DeltaTime)
{
	FVector TargetOffset = FVector::ZeroVector;

	ALyraFPSCharacter* FPSCharacter = TargetCharacter.Get();
	if (!FPSCharacter)
	{
		Super::UpdateView(DeltaTime);
		return;
	}

	if (UFPSProceduralAnimComp* ProceduralAnimComp = FPSCharacter->GetComponentByClass<UFPSProceduralAnimComp>())
	{
		TargetOffset = ProceduralAnimComp->GetTargetOffset(FPSAnimGameplayTags::Fragment_Jump, FPSAnimGameplayTags::Fragment_Target_Camera);
		TargetOffset += ProceduralAnimComp->GetTargetOffset(FPSAnimGameplayTags::Fragment_Crouch, FPSAnimGameplayTags::Fragment_Target_Camera);
	}

	FVector PivotLocation = GetPivotLocation() +  TargetOffset;
	// UKismetSystemLibrary::PrintString(this,
	// 	FString("TargetOffset:")+FString::SanitizeFloat(TargetOffset.Z,3) +FString(" PivotLoc:")+PivotLocation.ToString());
	FRotator PivotRotation = GetPivotRotation();
	PivotRotation.Pitch = FMath::ClampAngle(PivotRotation.Pitch, ViewPitchMin, ViewPitchMax);

	View.Location = PivotLocation;
	View.Rotation = PivotRotation;
	View.ControlRotation = View.Rotation;
	View.FieldOfView = FieldOfView;
}


void ULyraFPSCameraMode::OnActivation()
{
	Super::OnActivation();
	TargetCharacter = Cast<ALyraFPSCharacter>(GetTargetActor());
}
