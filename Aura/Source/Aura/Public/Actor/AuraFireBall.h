// P

#pragma once

#include "CoreMinimal.h"
#include "Actor/AuraProjectileActor.h"
#include "AuraFireBall.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraFireBall : public AAuraProjectileActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	
};
