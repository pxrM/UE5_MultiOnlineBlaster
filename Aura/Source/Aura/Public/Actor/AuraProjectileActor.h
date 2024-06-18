// P

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AuraProjectileActor.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class AURA_API AAuraProjectileActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraProjectileActor();

	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereCmp;


public:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementCmp;

};
