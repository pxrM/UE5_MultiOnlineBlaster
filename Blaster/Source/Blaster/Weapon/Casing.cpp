// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"

// Sets default values
ACasing::ACasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);
}

// Called when the game starts or when spawned
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACasing::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

