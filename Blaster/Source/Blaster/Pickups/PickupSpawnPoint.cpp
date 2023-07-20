// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"
#include "Pickup.h"

// Sets default values
APickupSpawnPoint::APickupSpawnPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

// Called when the game starts or when spawned
void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	/*
		在C++中，nullptr是一个指针字面量，它被转换为任何指针类型，或者可以是bool，nullptr_t。
		如果不进行类型转换，大多数情况下不会出现问题，因为编译器会自动进行隐式类型转换。
		然而，这种情况并不总是如此。在某些情况下，不进行显式类型转换可能会导致问题。
		例如，在函数重载时，如果函数的参数类型有多种可能，不进行显式类型转换可能会导致编译错误，因为编译器无法确定应该调用哪个函数。
		void foo(int* ptr) { std::cout << "Pointer overload" << std::endl; }
		void foo(int val) { std::cout << "Integer overload" << std::endl; }
		int main() {
			foo(nullptr); // 编译错误：call to 'foo' is ambiguous
			return 0;
		}
		在这个例子中，nullptr可以被隐式转换为int*或int，导致编译器无法确定应该调用哪个foo函数。
		如果我们显式地将nullptr转换为int*，编译器就能明确地知道应该调用哪个函数。
		int main() {
			foo(static_cast<int*>(nullptr)); // 调用指针重载
			return 0;
		}
	*/
	StartSpawnPickupTimer((AActor*)nullptr);
}

// Called every frame
void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void APickupSpawnPoint::SpawnPickup()
{
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0)
	{
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		if (SpawnedPickup && HasAuthority())
		{
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}
}

void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	GetWorldTimerManager().SetTimer(SpawnPickupTimer, this, &APickupSpawnPoint::SpawnPickupTimerFinished, SpawnTime);
}
