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
		��C++�У�nullptr��һ��ָ��������������ת��Ϊ�κ�ָ�����ͣ����߿�����bool��nullptr_t��
		�������������ת�������������²���������⣬��Ϊ���������Զ�������ʽ����ת����
		Ȼ���������������������ˡ���ĳЩ����£���������ʽ����ת�����ܻᵼ�����⡣
		���磬�ں�������ʱ����������Ĳ��������ж��ֿ��ܣ���������ʽ����ת�����ܻᵼ�±��������Ϊ�������޷�ȷ��Ӧ�õ����ĸ�������
		void foo(int* ptr) { std::cout << "Pointer overload" << std::endl; }
		void foo(int val) { std::cout << "Integer overload" << std::endl; }
		int main() {
			foo(nullptr); // �������call to 'foo' is ambiguous
			return 0;
		}
		����������У�nullptr���Ա���ʽת��Ϊint*��int�����±������޷�ȷ��Ӧ�õ����ĸ�foo������
		���������ʽ�ؽ�nullptrת��Ϊint*��������������ȷ��֪��Ӧ�õ����ĸ�������
		int main() {
			foo(static_cast<int*>(nullptr)); // ����ָ������
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
