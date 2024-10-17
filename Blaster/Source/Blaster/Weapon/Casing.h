// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"


/*
 * 开火时掉落的子弹外壳
 */
UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACasing();
	// Called every frame
	virtual void Tick(float DeltaTime) override;


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
		virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


private:	
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere)
		float ShellEjectImpulse;

	UPROPERTY(EditAnywhere)
		class USoundCue* ShellSound;
};
