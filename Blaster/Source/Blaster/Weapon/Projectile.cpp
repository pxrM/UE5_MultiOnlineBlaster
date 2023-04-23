// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	//设置碰撞类型为 ECC_WorldDynamic，表示该碰撞体与 WorldDynamic 类型的物体产生碰撞。
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	//设置碰撞可否，QueryAndPhysics 表示该碰撞体既能发生物理碰撞，又可以被其他物体查询是否有碰撞。
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//设置该碰撞体与所有 Channel 的碰撞响应方式为 ECR_Ignore，即忽略所有 Channel 的碰撞。
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//分别针对 ECC_Visibility 和 ECC_WorldStatic 两个 Channel 重新设置碰撞响应方式为 ECR_Block，
	//表示该碰撞体和 Visibility、WorldStatic 这两种类型的物体发生碰撞后，将被阻挡。
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

