// Fill out your copyright notice in the Description page of Project Settings.
/*
	开火时掉落的子弹外壳
*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

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

private:	
	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* CasingMesh;

};
