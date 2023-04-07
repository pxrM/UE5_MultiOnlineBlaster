// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponent/CombatComponent.h"

// Sets default values 
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//-----------------------------------------------------------------------------------------------------------
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); //创建弹簧臂
	CameraBoom->SetupAttachment(GetMesh());	//将弹簧臂连接到角色mesh上，角色移动时将带动弹簧臂
	CameraBoom->TargetArmLength = 600.f;	//设置弹簧臂的长度
	CameraBoom->bUsePawnControlRotation = true;	//设置是否跟随control一起旋转

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //将相机连接到弹簧臂上
	FollowCamera->bUsePawnControlRotation = false;

	//这里也要在蓝图编辑器设置一下，不然蓝图设置会覆盖调c++设置，在BP_BlasterCharacter(self)蓝图Details面板搜索UseController
	bUseControllerRotationYaw = false;	//不让角色跟随控制器一起旋转
	//同上在BP_BlasterCharacter(CharacterMovement组件)蓝图Details面板搜索Orient Rotation
	GetCharacterMovement()->bOrientRotationToMovement = true;	//使角色朝向其运动方向进行旋转

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatCmp = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatCmp->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//角色输入绑定按键
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipBtnPressed);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
}

/*--------------------------------------input--------------------------------------------*/

void ABlasterCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0)
	{
		const FRotator YawRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);	//偏航角
		const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X)); //获取向前方向
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	const FRotator YawRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);	//偏航角
	const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y)); //创建旋转矩阵然后获得矩阵Y轴的单位向量，这个向量就是当前朝向的方向
	AddMovementInput(Direction, Value);
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);	//向偏航角添加作用值
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);	//向俯仰角添加作用值
}

void ABlasterCharacter::EquipBtnPressed()
{
	//拾取武器需要服务器来验证
	if (CombatCmp && HasAuthority())
	{
		CombatCmp->EquipWeapon(OverlappingWeapon);
	}
}

/*--------------------------------------input--------------------------------------------*/

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//使用 DOREPLIFETIME 宏将 OverlappingWeapon 变量添加到需要进行网络同步的属性列表中。
	//DOREPLIFETIME 宏的第一个参数是要同步的变量名，第二个参数是要添加到的 OutLifetimeProps 数组。
	//在多人游戏中，每当 OverlappingWeapon 的值发生变化时，它将自动由引擎ds同步到其他客户端。在其他客户端上，该值将自动更新以匹配服务器上的值。
	//需要注意的是，除了在 GetLifetimeReplicatedProps 函数中添加属性外，
	//还需要确保所有需要同步的属性都使用了 UPROPERTY(Replicated) 标识符进行了标记。否则，这些属性可能无法被正确同步。
	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
	// 
	//DOREPLIFETIME_CONDITION 是 Unreal Engine 中用于网络同步的宏之一，用于标记需要进行网络同步的属性，并指定同步的条件。
	//第三个参数是同步的条件，即 COND_OwnerOnly,表示仅在具有所有权的客户端和服务器之间同步变量。
	//表示只有具有所有权的客户端和服务器之间会同步变量。这会降低网络流量，并维护数据一致性，因为通常情况下只有具有所有权的客户端可以更改某个对象的属性。
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatCmp)
	{
		CombatCmp->Character = this;
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}
