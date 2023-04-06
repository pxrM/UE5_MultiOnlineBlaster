// Fill out your copyright notice in the Description page of Project Settings.
/*
ECR_Ignore: 表示碰撞响应应为忽略所有通道。
ECR_Overlap: 表示碰撞响应应为生成重叠事件（即两个物体在某个时间段内部分或全部重叠）。
ECR_Block: 表示碰撞响应应为阻止所有通道。
ECR_MAX: 表示该枚举类型的最大值。
*/

#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

// Sets default values
AWeapon::AWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;	//表示该对象可以被网络同步。

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);	//将 WeaponMesh 设置为根组件。这意味着WeaponMesh现在是该物体的主要组件，并且它的位置取决于该物体的位置。

	/*处理武器或物品与玩家之间的交互*/
	//设置 WeaponMesh 对所有碰撞通道都具有 Block 响应, 会阻止某些物体通过它，以确保武器不会穿过玩家
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//设置 WeaponMesh 对 Pawn 类型物体的碰撞响应为 Ignore，WeaponMesh 将忽略与 Pawn 相关的任何碰撞，从而使玩家可以通过穿过武器拾取地上的物品
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//将 WeaponMesh 的碰撞设置为 NoCollision，WeaponMesh 不会与其他任何物体实现碰撞。
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/*AreaSphere 的作用是确定一个区域范围，以便玩家能够从该范围内拾取武器或物品，所以不需要对该游戏对象进行碰撞响应*/
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);	//设置忽略任何碰撞通道
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);	//设置禁止产生任何碰撞

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickpWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())	// GetLocalRole() == ENetRole::ROLE_Authority
	{
		//如果该武器是服务器端，即拥有权限的那个客户端
		//则将 AreaSphere 的碰撞启用状态设置为 QueryAndPhysics，以便进行查询和物理模拟。
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		//将碰撞响应设置为 ECC_Pawn，表示 Pawn 可以与 AreaSphere 重叠。
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOerlap);
	}

	ShowPickupWidget(false);
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOerlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::ShowPickupWidget(bool bShow)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShow);
	}
}



