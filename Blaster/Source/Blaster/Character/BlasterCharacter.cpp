// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponent/CombatComponent.h"
#include "Blaster/BlasterComponent/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GamePlayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "Blaster/BlasterComponent/LagCompensationComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Blaster/GameState/BlasterGameState.h"

// Sets default values 
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//-----------------------------------------------------------------------------------------------------------
	//SpawnCollisionHandlingMethod用于控制新生成的Actor在产生碰撞时的处理方式。
	//ESpawnActorCollisionHandlingMethod枚举类型定义了Actor生成时可能遇到的几种不同的碰撞处理方式，包括：
	//	Undefined：未定义的碰撞处理方式，不进行任何处理。
	//	AlwaysSpawn：始终生成新的Actor，忽略任何碰撞检测。
	//	AdjustIfPossibleButAlwaysSpawn：如果新生成的Actor与现有的Actor发生碰撞，则尝试对新生成的Actor位置进行调整以避免碰撞。如果无法避免碰撞，则仍然生成新的Actor。
	//	AdjustIfPossibleButDontSpawnIfColliding：如果新生成的Actor与现有的Actor发生碰撞，则尝试对新生成的Actor位置进行调整以避免碰撞。如果无法避免碰撞，则不会生成新的Actor。
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
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

	BuffCmp = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffCmp->SetIsReplicated(true);

	LagCompensationCmp = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

	DissolveTimelineCmp = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;	//蓝图也可设置
	//在其他角色和本角色的相机碰撞时会出现相机放大效果，使用忽略相机的碰撞的办法解决，同时也要在角色蓝图对应组件的Collision Response里设置
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	/*
	* SetCollisionResponseToChannel()是设置Actor的碰撞响应函数，用于控制碰撞检测时Actor与其他指定通道的物体之间的交互方式。
	*	 具体来说，这个函数的作用是将该Actor与指定类型的对象设为可碰撞，并指定其碰撞响应方式。
	*		参数1表示要设置的碰撞通道，参数2则是指定该通道与其他对象的碰撞响应方式。
	* 注意：蓝图中也要检查设置
	*/
	//碰撞通道被设置为ECC_Visibility，用于处理各种类型的可见性和投影问题。ECR_Block设置为碰撞响应方式，表示该通道上的碰撞体会阻碍任何对跨越通道的运动进行的碰撞。
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 820.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;	//Actor 网络同步的最大频率
	MinNetUpdateFrequency = 33.f;	//网络同步的最小频率

	/*
		服务器端倒带的方框
	*/
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head")); //这里第二个参数是骨骼名
	HitConllisionBoxs.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitConllisionBoxs.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitConllisionBoxs.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitConllisionBoxs.Add(FName("spine_03"), spine_03);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitConllisionBoxs.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitConllisionBoxs.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitConllisionBoxs.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitConllisionBoxs.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitConllisionBoxs.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitConllisionBoxs.Add(FName("hand_r"), hand_r);

	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitConllisionBoxs.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitConllisionBoxs.Add(FName("backpack"), backpack);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitConllisionBoxs.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitConllisionBoxs.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitConllisionBoxs.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitConllisionBoxs.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitConllisionBoxs.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitConllisionBoxs.Add(FName("foot_r"), foot_r);

	for (auto& Box : HitConllisionBoxs)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

// Called when the game starts or when spawned
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();
	UpdateHUDAmmo();
	UpdateHUDHealth();
	UpdateHUDShield();

	// 这段代码是在服务端（即拥有所有权的机器或主机）中调用的，用于注册处理角色收到任何伤害事件的函数。
	// 首先，HasAuthority()函数判断当前执行的代码是否在服务端中。
	// 接着，OnTakeAnyDamage是一个代表角色受到任何类型伤害事件的委托（Delegate），该委托会在角色受到任何类型的伤害时被触发。
	// AddDynamic函数用于动态地将受到任何类型伤害事件与AblasterCharacter::ReceiveDamage方法绑定，以便在收到伤害事件时自动调用ReceiveDamage方法来处理伤害效果。
	// 最后，this表示当前脚本所在的角色对象本身，即指定当角色受到任何类型的伤害时都会调用该角色对象的ReceiveDamage方法。
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}

	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

// Called every frame
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit(); //初始几帧的时候玩家数据可能还没初始化，所以放到tick里
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//角色输入绑定按键
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipBtnPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchBtnPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimBtnPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimBtnReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireBtnPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireBtnReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadMagBtnPressed);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeBtnPressed);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatCmp)
	{
		CombatCmp->Character = this;
	}
	if (BuffCmp)
	{
		BuffCmp->Character = this;
		BuffCmp->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		BuffCmp->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
	if (LagCompensationCmp)
	{
		LagCompensationCmp->Character = this;
		if (Controller)
		{
			LagCompensationCmp->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//使用 DOREPLIFETIME 宏将 OverlappingWeapon 变量添加到需要进行网络同步的属性列表中。
	//DOREPLIFETIME 宏的第一个参数是要同步的变量名，第二个参数是要添加到的 OutLifetimeProps 数组。
	//在多人游戏中，每当 OverlappingWeapon 的值发生变化时，它将自动由引擎ds服务器同步到其他客户端。在其他客户端上，该值将自动更新以匹配服务器上的值。
	//需要注意的是，除了在 GetLifetimeReplicatedProps 函数中添加属性外，
	//还需要确保所有需要同步的属性都使用了 UPROPERTY(Replicated) 标识符进行了标记。否则，这些属性可能无法被正确同步。
	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
	// 
	//DOREPLIFETIME_CONDITION 是 Unreal Engine 中用于网络同步的宏之一，用于标记需要进行网络同步的属性，并指定同步的条件。
	//第三个参数是同步的条件，即 COND_OwnerOnly,表示仅在具有所有权的客户端和服务器之间同步变量。
	//表示只有具有所有权的客户端和服务器之间会同步变量。这会降低网络流量，并维护数据一致性，因为通常情况下只有具有所有权的客户端可以更改某个对象的属性。
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);

	DOREPLIFETIME(ABlasterCharacter, CurHealth);
	DOREPLIFETIME(ABlasterCharacter, CurShield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

/*--------------------------------------input start--------------------------------------------*/

void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;
	if (Controller && Value != 0)
	{
		const FRotator YawRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);	//偏航角
		const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X)); //获取向前方向
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
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
	if (bDisableGameplay) return;
	//拾取武器需要服务器来验证
	if (CombatCmp && CombatCmp->CombatState == ECombatState::ECS_Unoccupied)
	{
		if (CombatCmp->bHoldingTheFlag) return;
		//当装备按钮按下时，如果角色当前是服务端，则直接调用 CombatCmp 组件的 EquipWeapon 函数；
		//否则，将该函数代理给 ServerEquipBtnPressed_Implementation 的远程过程调用（RPC）版本，以便由服务器验证并执行相应的操作。
		//通过这种方式，确保装备按钮按下事件在所有客户端和服务器之间正确同步，并且在需要访问服务器资源或执行敏感操作时，由服务端进行验证和控制，从而提高游戏的安全性和可靠性。
		/*if (HasAuthority())
		{
			if (OverlappingWeapon)
			{
				CombatCmp->EquipWeapon(OverlappingWeapon);
			}
			else if (CombatCmp->IsShouldSwapWeapons())
			{
				CombatCmp->SwapWeapons();
			}
		}
		else
		{
			ServerEquipBtnPressed();
		}*/
		ServerEquipBtnPressed();

		bool bSwap = CombatCmp->IsShouldSwapWeapons() && !HasAuthority() && OverlappingWeapon == nullptr;
		if (bSwap)
		{
			PlaySwapMontage();
			CombatCmp->CombatState = ECombatState::ECS_SwappingWeapons;
			bFinishedSwapping = false;
		}
	}
}

//只在服务端被调用以响应 RPC 请求
void ABlasterCharacter::ServerEquipBtnPressed_Implementation()
{
	if (CombatCmp)
	{
		if (OverlappingWeapon)
		{
			CombatCmp->EquipWeapon(OverlappingWeapon);
		}
		else if (CombatCmp->IsShouldSwapWeapons())
		{
			CombatCmp->SwapWeapons();
		}
	}
}

void ABlasterCharacter::CrouchBtnPressed()
{
	if (CombatCmp && CombatCmp->bHoldingTheFlag) return;
	if (bDisableGameplay) return;
	//这里会设置ACharacter->bIsCrouched
	//这里会设置角色胶囊体的大小，可在运行期间按~键输入ShowCollision查看
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void ABlasterCharacter::AimBtnPressed()
{
	if (bDisableGameplay) return;
	if (CombatCmp)
	{
		if (CombatCmp->bHoldingTheFlag) return;
		CombatCmp->SetAiming(true);
	}
}

void ABlasterCharacter::AimBtnReleased()
{
	if (bDisableGameplay) return;
	if (CombatCmp)
	{
		if (CombatCmp->bHoldingTheFlag) return;
		CombatCmp->SetAiming(false);
	}
}

float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	if (CombatCmp && CombatCmp->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		return;
	}

	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		// 本地机器上的其他玩家
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (CombatCmp && CombatCmp->EquippedWeapon == nullptr)return;

	float Speed = CalculateSpeed();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
			//UE_LOG(LogTemp, Log, TEXT("GetAO_Yaw(): %i"), InterpAO_Yaw);
		}
		//UE_LOG(LogTemp, Warning, TEXT("GetAO_Yaw(): %i"), AO_Yaw);
		bUseControllerRotationYaw = false;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir)
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		InterpAO_Yaw = AO_Yaw;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f); //使用 FInterpTo 函数实现平滑的偏移量插值
		AO_Yaw = InterpAO_Yaw; //将 InterpAO_Yaw 赋值给 AO_Yaw，使得角色的 AIM Offset 偏移量随着插值而发生平滑的变化
		//在 AO_Yaw 偏移量小于一定阈值（15 度）时，表示原地转向完成，将 TurningInPlace 设置为 ETIP_NotTurning，
		//并重新将 StartingAimRotation 设为当前角色的基础瞄准旋转，以便下一次原地转向时的参考。
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
	//UE_LOG(LogTemp, Log, TEXT("AO_Yaw: %i   %i"), AO_Yaw, TurningInPlace);
}


void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled()) //由服务器控制的场景下，即非本地玩家控制的情况下才会生效
	{
		//因为客户端同步给服务器的时候会调用PackYawAndPitchTo32（先将Pitch和Yaw浮点数映射到 16 位无符号整数范围（0~65535），然后组合成一个32位整数返回），
		//将数据进行压缩以减少传输和存储的负担。所以这里需要进行转换Pitch值从原始范围[270, 360]映射到新范围[-90, 0]，
		//将视角的俯仰角度限制在-90度到0度之间。这个新范围相当于将视角的上下可视范围限制为正前方到水平线以下，使游戏场景更符合现实场景。
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (CombatCmp == nullptr || CombatCmp->EquippedWeapon == nullptr)return;

	bRotateRootBone = false;
	float Speed = CalculateSpeed();
	if (Speed > .0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	ProxyRotationLastFrame = ProxyRotationCur;
	ProxyRotationCur = GetActorRotation();
	ProxyYawOffset = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotationCur, ProxyRotationLastFrame).Yaw;
	UE_LOG(LogTemp, Warning, TEXT("ProxyYawOffset: %f"), ProxyYawOffset);
	if (FMath::Abs(ProxyYawOffset) > TurnThreshold)
	{
		if (ProxyYawOffset > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYawOffset < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireBtnPressed()
{
	if (bDisableGameplay) return;
	if (CombatCmp)
	{
		if (CombatCmp->bHoldingTheFlag) return;
		CombatCmp->FireBtnPressed(true);
	}
}

void ABlasterCharacter::FireBtnReleased()
{
	if (bDisableGameplay) return;
	if (CombatCmp)
	{
		if (CombatCmp->bHoldingTheFlag) return;
		CombatCmp->FireBtnPressed(false);
	}
}

void ABlasterCharacter::ReloadMagBtnPressed()
{
	if (bDisableGameplay) return;
	if (CombatCmp)
	{
		if (CombatCmp->bHoldingTheFlag) return;
		CombatCmp->ReloadMag();
	}
}

void ABlasterCharacter::GrenadeBtnPressed()
{
	if (CombatCmp)
	{
		if (CombatCmp->bHoldingTheFlag) return;
		CombatCmp->ThrowGrenade();
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled())return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatCmp && CombatCmp->EquippedWeapon && CombatCmp->EquippedWeapon->GetWeaponMesh())
		{
			CombatCmp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (CombatCmp && CombatCmp->SecondaryWeapon && CombatCmp->SecondaryWeapon->GetWeaponMesh())
		{
			CombatCmp->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatCmp && CombatCmp->EquippedWeapon && CombatCmp->EquippedWeapon->GetWeaponMesh())
		{
			CombatCmp->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (CombatCmp && CombatCmp->SecondaryWeapon && CombatCmp->SecondaryWeapon->GetWeaponMesh())
		{
			CombatCmp->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

//void ABlasterCharacter::MulticastHit_Implementation()
//{
//	PlayHitReactMontage();
//}

/*--------------------------------------input end--------------------------------------------*/

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;	//发生改变，会同步给所有客户端，触发客户端 OnRep_OverlappingWeapon 函数
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool ABlasterCharacter::IsWeaponEquipped()
{
	return (CombatCmp && CombatCmp->EquippedWeapon);
}

bool ABlasterCharacter::IsAiming()
{
	return (CombatCmp && CombatCmp->bAiming);
}

AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	if (CombatCmp == nullptr)return nullptr;
	return CombatCmp->EquippedWeapon;
}

void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatCmp == nullptr || CombatCmp->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMagMontage()
{
	if (CombatCmp == nullptr || CombatCmp->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMagMontage)
	{
		AnimInstance->Montage_Play(ReloadMagMontage);
		FName SectionName;
		switch (CombatCmp->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLacuncher:
			SectionName = FName("GrenadeLacuncher");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (CombatCmp == nullptr || CombatCmp->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (CombatCmp == nullptr)return FVector();
	return CombatCmp->HitTarget;
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

void ABlasterCharacter::OnRep_CurHealth(float LastHealth)
{
	UpdateHUDHealth();
	if (CurHealth < LastHealth)
	{
		//血量减少播放受伤蒙太奇
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_CurShield(float LastShield)
{
	UpdateHUDShield();
	if (CurShield < LastShield)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	//GetAuthGameMode<T>()是一个由UE引擎提供的模板函数，用于获取当前场景中存在的并继承自T(AGameMode)类型
	BlasterGameMode = BlasterGameMode ? BlasterGameMode : GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	// bElimmed 防止重复向淘汰者施加伤害
	if (bElimmed || BlasterGameMode == nullptr) return; 

	Damage = BlasterGameMode->CalculateDamage(InstigatorController, Controller, Damage);
	float DamageToHealth = Damage;
	if (CurHealth > 0.f)
	{
		if (CurShield >= Damage)
		{
			CurShield = FMath::Clamp(CurShield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - CurShield, 0.f, Damage);
			CurShield = 0.f;
		}
	}
	CurHealth = FMath::Clamp(CurHealth - DamageToHealth, 0.f, MaxHealth);

	UpdateHUDHealth();
	UpdateHUDShield();
	PlayHitReactMontage();

	if (CurHealth == 0.f)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
		BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
	}
}

void ABlasterCharacter::UpdateHUDHealth()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(CurHealth, MaxHealth);
	}
}

void ABlasterCharacter::UpdateHUDShield()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(CurShield, MaxShield);
	}
}

void ABlasterCharacter::UpdateHUDAmmo()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController && CombatCmp && CombatCmp->EquippedWeapon)
	{
		BlasterPlayerController->SetHUDCarriedAmmo(CombatCmp->CurWeaponCarriedAmmo);
		BlasterPlayerController->SetHUDWeaponAmmo(CombatCmp->EquippedWeapon->GetAmmoNum());
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToScore(0.f);
			BlasterPlayerState->AddToDefeats(0);
			SetTeamColor(BlasterPlayerState->GetTeam());

			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
}

void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	if (CombatCmp)
	{
		DropOrDestroyWeapon(CombatCmp->EquippedWeapon);
		DropOrDestroyWeapon(CombatCmp->SecondaryWeapon);
	}

	MulticastElim(bPlayerLeftGame); //网络多播
}

void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr) return;

	switch (Team)
	{
	case ETeam::ET_RedTeam:
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMatInstance = RedDissolveMatInst;
		break;
	case ETeam::ET_BlueTeam:
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMatInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_NoTeam:
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMatInstance = BlueDissolveMatInst;
		break;
	}
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		if (Weapon->bDestroyWeapon)
		{
			Weapon->Destroy();
		}
		else
		{
			Weapon->Dropped();
		}
	}
}

void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}

	bElimmed = true;

	// 播放淘汰蒙太奇
	PlayElimMontage();
	// 开始溶解特效
	StartDisslove();
	// 关闭角色移动
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	// 关闭输入
	bDisableGameplay = true;
	//if (BlasterPlayerController)
	//{
	//	DisableInput(BlasterPlayerController);
	//}
	// 关闭自动开火
	if (CombatCmp)
	{
		CombatCmp->FireBtnPressed(false);
	}
	// 关闭碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//生成淘汰回收机器人 特效
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}
	// 生成淘汰回收机器人 音效
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}
	// 关闭瞄准ui
	bool bHideSniperScope = IsLocallyControlled() && CombatCmp && CombatCmp->bAiming &&
		CombatCmp->EquippedWeapon && CombatCmp->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	// 关闭第一的皇冠
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	// 开启复活计时器
	GetWorldTimerManager().SetTimer(ElimTimer, this, &ABlasterCharacter::ElimTimerFinished, ElimDelay);
}

void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode ? BlasterGameMode : GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->ResquestRespawn(this, Controller);
	}

	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

void ABlasterCharacter::ServerLeavaGame_Implementation()
{
	BlasterGameMode = BlasterGameMode ? BlasterGameMode : GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	BlasterPlayerState = BlasterPlayerState ? BlasterPlayerState : GetPlayerState<ABlasterPlayerState>();
	if (BlasterGameMode)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

void ABlasterCharacter::UpdataDissloveMaterial(float DissloveVal)
{
	if (DynamicDissolveMatInstance)
	{
		DynamicDissolveMatInstance->SetScalarParameterValue(TEXT("Dissolve"), DissloveVal);
	}
}

void ABlasterCharacter::StartDisslove()
{
	if (DissolveMatInstance)
	{
		DynamicDissolveMatInstance = UMaterialInstanceDynamic::Create(DissolveMatInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMatInstance);
		DynamicDissolveMatInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMatInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}

	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdataDissloveMaterial);
	if (DissolveCurve && DissolveTimelineCmp)
	{
		DissolveTimelineCmp->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimelineCmp->Play();
	}
}

ECombatState ABlasterCharacter::GetCombatState() const
{
	if (CombatCmp == nullptr) return ECombatState::ECS_MAX;
	return CombatCmp->CombatState;
}

bool ABlasterCharacter::GetIsLocallyReloading() const
{
	if (CombatCmp) return CombatCmp->bLocallyReloading;
	return false;
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	BlasterGameMode = BlasterGameMode ? BlasterGameMode : GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	if (CombatCmp && CombatCmp->EquippedWeapon && bMatchNotInProgress)  //如果不是游戏进行中状态删除武器
	{
		CombatCmp->EquippedWeapon->Destroy();
	}
}


void ABlasterCharacter::SpawnDefaultWeapon()
{
	// 不在服务器GameMode会返回null
	BlasterGameMode = BlasterGameMode ? BlasterGameMode : Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	if (BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true;
		if (CombatCmp)
		{
			CombatCmp->EquipWeapon(StartingWeapon);
		}
	}
}

void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return;
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem,
			GetCapsuleComponent(),
			FName(),
			GetActorLocation() + FVector(0.f, 0.f, 110.f),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}



bool ABlasterCharacter::IsHoldingTheFlag() const
{
	if (CombatCmp)
	{
		CombatCmp->bHoldingTheFlag;
	}
	return false;
}