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
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponent/CombatComponent.h"

// Sets default values
AWeapon::AWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//bReplicates用于控制是否启用网络复制。
	//为true，则该Actor的状态和属性将在客户端和服务器之间进行复制和同步，确保它们在所有玩家之间具有相同的状态和行为。
	//为false，则Actor只存在于服务器上，客户端无法访问或控制它。
	bReplicates = true;
	//同步移动
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);	//将 WeaponMesh 设置为根组件。这意味着WeaponMesh现在是该物体的主要组件，并且它的位置取决于该物体的位置。
	/*处理武器或物品与玩家之间的交互*/
	//设置 WeaponMesh 对所有碰撞通道都具有 Block 响应, 会阻止某些物体通过它，以确保武器不会穿过玩家
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	//设置 WeaponMesh 对 Pawn 类型物体的碰撞响应为 Ignore，WeaponMesh 将忽略与 Pawn 相关的任何碰撞，从而使玩家可以通过穿过武器拾取地上的物品
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	//将 WeaponMesh 的碰撞设置为 NoCollision，WeaponMesh 不会与其他任何物体实现碰撞。
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	/*处理武器的深度值	======  蓝图设置会覆盖c++，所以需要检查蓝图设置	*/
	//置该模型的自定义深度模板值（Custom Depth Stencil Value）
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_COLOR_BLUE);
	//标记该模型的渲染状态（Render State）为“脏”（Dirty）。这将强制引擎在下一帧重新渲染该模型，以便应用新的自定义深度模板值。
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

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

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, AmmoNum);
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : BlasterOwnerCharacter;
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
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

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State; //服务器值发生改变，会同步给所有客户端，触发客户端 OnRep_WeaponState 函数

	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		//服务端调用 SetCollisionEnabled 函数之后，该函数将会在所有客户端上被调用并执行相应的操作，以使服务端和所有客户端都使用相同的碰撞设置
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);  //关闭碰撞检测
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority()) //是否在服务器
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); //将武器球形碰撞体设为可查询
		}
		WeaponMesh->SetSimulatePhysics(true); //模拟物理效果
		WeaponMesh->SetEnableGravity(true); //启用重力效果
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //设置为可查询和模拟物理效果的对象
		//设置 WeaponMesh 对所有碰撞通道都具有 Block 响应, 会阻止某些物体通过它，以确保武器不会穿过玩家
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		//设置 WeaponMesh 对 Pawn 类型物体的碰撞响应为 Ignore，WeaponMesh 将忽略与 Pawn 相关的任何碰撞，从而使玩家可以通过穿过武器拾取地上的物品
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_COLOR_BLUE);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Initial:
		break;
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		}
		break;
	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true); //模拟物理效果
		WeaponMesh->SetEnableGravity(true); //启用重力效果
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); //设置为可查询和模拟物理效果的对象
		//设置 WeaponMesh 对所有碰撞通道都具有 Block 响应, 会阻止某些物体通过它，以确保武器不会穿过玩家
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		//设置 WeaponMesh 对 Pawn 类型物体的碰撞响应为 Ignore，WeaponMesh 将忽略与 Pawn 相关的任何碰撞，从而使玩家可以通过穿过武器拾取地上的物品
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_COLOR_BLUE);
		WeaponMesh->MarkRenderStateDirty();
		EnableCustomDepth(true);
		break;
	case EWeaponState::EWS_MAX:
		break;
	default:
		break;
	}
}

void AWeapon::SpeedRound()
{
	/*服务器执行*/
	AmmoNum = FMath::Clamp(AmmoNum - 1, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	AmmoNum = FMath::Clamp(AmmoNum - AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWeapon::OnRep_AmmoNum()
{
	/*同步给客户端执行*/
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if(IsAmmoFull() && BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombatCmp())
	{
		BlasterOwnerCharacter->GetCombatCmp()->AnimJumpToShotgunEnd();
	}
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			BlasterOwnerController->SetHUDWeaponAmmo(AmmoNum);
		}
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
			}
		}
	}
	SpeedRound();
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);

	//使用 FDetachmentTransformRules 类型的 DetachRules 对象来设置武器的分离规则：
	//EDetachmentRule::KeepWorld 表示武器将会保持在世界空间中，不继承所属组件或Actor的位置、旋转和缩放信息。
	//第二个参数 true 表示保留原始缩放信息。当武器被分离时，它将保留自己的位置、旋转和缩放信息，并且可以自由地在游戏世界中移动和旋转。
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	//将武器从当前所属的组件或者Actor上分离（detach），此时，武器将变成一个独立的对象
	WeaponMesh->DetachFromComponent(DetachRules);
	//将拥有者置空
	SetOwner(nullptr);

	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}
