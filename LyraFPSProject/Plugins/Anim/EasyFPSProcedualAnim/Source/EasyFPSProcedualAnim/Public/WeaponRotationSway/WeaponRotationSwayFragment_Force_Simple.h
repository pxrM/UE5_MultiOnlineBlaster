#pragma once

#include "CoreMinimal.h"
#include "WeaponRotationSwayFragment.h"
#include "WeaponRotationSwayFragment_Force_Simple.generated.h"

/**
 * 仅实验阶段，不可实用
 */
UCLASS(NotBlueprintable,NotBlueprintType,Abstract)
class EASYFPSPROCEDUALANIM_API UWeaponRotationSwayFragment_Force_Simple : public UWeaponRotationSwayFragment
{
	GENERATED_BODY()
	
public:
	UWeaponRotationSwayFragment_Force_Simple();
	
	virtual void WeaponRotationSwayTick_Implementation(float DeltaSeconds,FRotator CurRot,bool bHasInput,USkeletalMeshComponent* WeaponMesh=nullptr,bool bInvertRollPitch=false,bool bDebug=false);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ClampMin=0.f),Category="Force|Straighten")
	float Straighten_TotalTime=4.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ClampMin=0.f),Category="Force|Straighten")
    float Straighten_NoneLinear=.3f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite,meta=(ClampMin=0.f),Category="Force|Straighten")
    float Straighten_MinValue=.3f;
	UPROPERTY(BlueprintReadWrite,Category="Force|Straighten")
	float curInputForce=0.f;
	UPROPERTY(BlueprintReadWrite,Category="Force|Straighten")
	float returnForce=0.f;
	UPROPERTY(BlueprintReadWrite,Category="Force|Straighten")
	float curForce=0.f;
	float NoInputBlendTime = 0.f;
	UPROPERTY(BlueprintReadWrite,Category="Force|Straighten")
	float InputTime=0.f;
	
	UPROPERTY(BlueprintReadWrite)
	FRotator WeaponAngVel;
	
	UPROPERTY(BlueprintReadWrite)
	FRotator WeaponRot;
	
	UPROPERTY(BlueprintReadWrite)
	FVector2D AngularSpeed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Force")
	FRotator InputForce=FRotator(-28.f,28.f,-36.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Force")
	FRotator ReturnForce_HasInput=FRotator(40.f,180.f,180.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Force")
	FRotator ReturnForce_NoInput=FRotator(40.f,180.f,180.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Force",meta=(ClampMin=0.f))
	FRotator Damp= FRotator(14.f);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="Force")
	FRotator WeaponRotClamp=FRotator::ZeroRotator;
};
