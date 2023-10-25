// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 旗帜，继承武器方便使用武器的拾取功能
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:
	AFlag();

private:
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* FlagMesh;
	
};
