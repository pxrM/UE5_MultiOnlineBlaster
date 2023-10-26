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
	//SpawnCollisionHandlingMethod���ڿ��������ɵ�Actor�ڲ�����ײʱ�Ĵ���ʽ��
	//ESpawnActorCollisionHandlingMethodö�����Ͷ�����Actor����ʱ���������ļ��ֲ�ͬ����ײ����ʽ��������
	//	Undefined��δ�������ײ����ʽ���������κδ���
	//	AlwaysSpawn��ʼ�������µ�Actor�������κ���ײ��⡣
	//	AdjustIfPossibleButAlwaysSpawn����������ɵ�Actor�����е�Actor������ײ�����Զ������ɵ�Actorλ�ý��е����Ա�����ײ������޷�������ײ������Ȼ�����µ�Actor��
	//	AdjustIfPossibleButDontSpawnIfColliding����������ɵ�Actor�����е�Actor������ײ�����Զ������ɵ�Actorλ�ý��е����Ա�����ײ������޷�������ײ���򲻻������µ�Actor��
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom")); //�������ɱ�
	CameraBoom->SetupAttachment(GetMesh());	//�����ɱ����ӵ���ɫmesh�ϣ���ɫ�ƶ�ʱ���������ɱ�
	CameraBoom->TargetArmLength = 600.f;	//���õ��ɱ۵ĳ���
	CameraBoom->bUsePawnControlRotation = true;	//�����Ƿ����controlһ����ת

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //��������ӵ����ɱ���
	FollowCamera->bUsePawnControlRotation = false;

	//����ҲҪ����ͼ�༭������һ�£���Ȼ��ͼ���ûḲ�ǵ�c++���ã���BP_BlasterCharacter(self)��ͼDetails�������UseController
	bUseControllerRotationYaw = false;	//���ý�ɫ���������һ����ת
	//ͬ����BP_BlasterCharacter(CharacterMovement���)��ͼDetails�������Orient Rotation
	GetCharacterMovement()->bOrientRotationToMovement = true;	//ʹ��ɫ�������˶����������ת

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

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;	//��ͼҲ������
	//��������ɫ�ͱ���ɫ�������ײʱ���������Ŵ�Ч����ʹ�ú����������ײ�İ취�����ͬʱҲҪ�ڽ�ɫ��ͼ��Ӧ�����Collision Response������
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	/*
	* SetCollisionResponseToChannel()������Actor����ײ��Ӧ���������ڿ�����ײ���ʱActor������ָ��ͨ��������֮��Ľ�����ʽ��
	*	 ������˵����������������ǽ���Actor��ָ�����͵Ķ�����Ϊ����ײ����ָ������ײ��Ӧ��ʽ��
	*		����1��ʾҪ���õ���ײͨ��������2����ָ����ͨ���������������ײ��Ӧ��ʽ��
	* ע�⣺��ͼ��ҲҪ�������
	*/
	//��ײͨ��������ΪECC_Visibility�����ڴ���������͵Ŀɼ��Ժ�ͶӰ���⡣ECR_Block����Ϊ��ײ��Ӧ��ʽ����ʾ��ͨ���ϵ���ײ����谭�κζԿ�Խͨ�����˶����е���ײ��
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 820.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;	//Actor ����ͬ�������Ƶ��
	MinNetUpdateFrequency = 33.f;	//����ͬ������СƵ��

	/*
		�������˵����ķ���
	*/
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head")); //����ڶ��������ǹ�����
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

	// ��δ������ڷ���ˣ���ӵ������Ȩ�Ļ������������е��õģ�����ע�ᴦ���ɫ�յ��κ��˺��¼��ĺ�����
	// ���ȣ�HasAuthority()�����жϵ�ǰִ�еĴ����Ƿ��ڷ�����С�
	// ���ţ�OnTakeAnyDamage��һ�������ɫ�ܵ��κ������˺��¼���ί�У�Delegate������ί�л��ڽ�ɫ�ܵ��κ����͵��˺�ʱ��������
	// AddDynamic�������ڶ�̬�ؽ��ܵ��κ������˺��¼���AblasterCharacter::ReceiveDamage�����󶨣��Ա����յ��˺��¼�ʱ�Զ�����ReceiveDamage�����������˺�Ч����
	// ���this��ʾ��ǰ�ű����ڵĽ�ɫ��������ָ������ɫ�ܵ��κ����͵��˺�ʱ������øý�ɫ�����ReceiveDamage������
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
	PollInit(); //��ʼ��֡��ʱ��������ݿ��ܻ�û��ʼ�������Էŵ�tick��
}

// Called to bind functionality to input
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//��ɫ����󶨰���
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

	//ʹ�� DOREPLIFETIME �꽫 OverlappingWeapon ������ӵ���Ҫ��������ͬ���������б��С�
	//DOREPLIFETIME ��ĵ�һ��������Ҫͬ���ı��������ڶ���������Ҫ��ӵ��� OutLifetimeProps ���顣
	//�ڶ�����Ϸ�У�ÿ�� OverlappingWeapon ��ֵ�����仯ʱ�������Զ�������ds������ͬ���������ͻ��ˡ��������ͻ����ϣ���ֵ���Զ�������ƥ��������ϵ�ֵ��
	//��Ҫע����ǣ������� GetLifetimeReplicatedProps ��������������⣬
	//����Ҫȷ��������Ҫͬ�������Զ�ʹ���� UPROPERTY(Replicated) ��ʶ�������˱�ǡ�������Щ���Կ����޷�����ȷͬ����
	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
	// 
	//DOREPLIFETIME_CONDITION �� Unreal Engine ����������ͬ���ĺ�֮һ�����ڱ����Ҫ��������ͬ�������ԣ���ָ��ͬ����������
	//������������ͬ������������ COND_OwnerOnly,��ʾ���ھ�������Ȩ�Ŀͻ��˺ͷ�����֮��ͬ��������
	//��ʾֻ�о�������Ȩ�Ŀͻ��˺ͷ�����֮���ͬ����������ή��������������ά������һ���ԣ���Ϊͨ�������ֻ�о�������Ȩ�Ŀͻ��˿��Ը���ĳ����������ԡ�
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
		const FRotator YawRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);	//ƫ����
		const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X)); //��ȡ��ǰ����
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return;
	const FRotator YawRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);	//ƫ����
	const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y)); //������ת����Ȼ���þ���Y��ĵ�λ����������������ǵ�ǰ����ķ���
	AddMovementInput(Direction, Value);
}

void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);	//��ƫ�����������ֵ
}

void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);	//�������������ֵ
}

void ABlasterCharacter::EquipBtnPressed()
{
	if (bDisableGameplay) return;
	//ʰȡ������Ҫ����������֤
	if (CombatCmp && CombatCmp->CombatState == ECombatState::ECS_Unoccupied)
	{
		if (CombatCmp->bHoldingTheFlag) return;
		//��װ����ť����ʱ�������ɫ��ǰ�Ƿ���ˣ���ֱ�ӵ��� CombatCmp ����� EquipWeapon ������
		//���򣬽��ú�������� ServerEquipBtnPressed_Implementation ��Զ�̹��̵��ã�RPC���汾���Ա��ɷ�������֤��ִ����Ӧ�Ĳ�����
		//ͨ�����ַ�ʽ��ȷ��װ����ť�����¼������пͻ��˺ͷ�����֮����ȷͬ������������Ҫ���ʷ�������Դ��ִ�����в���ʱ���ɷ���˽�����֤�Ϳ��ƣ��Ӷ������Ϸ�İ�ȫ�ԺͿɿ��ԡ�
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

//ֻ�ڷ���˱���������Ӧ RPC ����
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
	//���������ACharacter->bIsCrouched
	//��������ý�ɫ������Ĵ�С�����������ڼ䰴~������ShowCollision�鿴
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
		// ���ػ����ϵ��������
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
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f); //ʹ�� FInterpTo ����ʵ��ƽ����ƫ������ֵ
		AO_Yaw = InterpAO_Yaw; //�� InterpAO_Yaw ��ֵ�� AO_Yaw��ʹ�ý�ɫ�� AIM Offset ƫ�������Ų�ֵ������ƽ���ı仯
		//�� AO_Yaw ƫ����С��һ����ֵ��15 �ȣ�ʱ����ʾԭ��ת����ɣ��� TurningInPlace ����Ϊ ETIP_NotTurning��
		//�����½� StartingAimRotation ��Ϊ��ǰ��ɫ�Ļ�����׼��ת���Ա���һ��ԭ��ת��ʱ�Ĳο���
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
	if (AO_Pitch > 90.f && !IsLocallyControlled()) //�ɷ��������Ƶĳ����£����Ǳ�����ҿ��Ƶ�����²Ż���Ч
	{
		//��Ϊ�ͻ���ͬ������������ʱ������PackYawAndPitchTo32���Ƚ�Pitch��Yaw������ӳ�䵽 16 λ�޷���������Χ��0~65535����Ȼ����ϳ�һ��32λ�������أ���
		//�����ݽ���ѹ���Լ��ٴ���ʹ洢�ĸ���������������Ҫ����ת��Pitchֵ��ԭʼ��Χ[270, 360]ӳ�䵽�·�Χ[-90, 0]��
		//���ӽǵĸ����Ƕ�������-90�ȵ�0��֮�䡣����·�Χ�൱�ڽ��ӽǵ����¿��ӷ�Χ����Ϊ��ǰ����ˮƽ�����£�ʹ��Ϸ������������ʵ������
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
	OverlappingWeapon = Weapon;	//�����ı䣬��ͬ�������пͻ��ˣ������ͻ��� OnRep_OverlappingWeapon ����
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
		//Ѫ�����ٲ���������̫��
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
	//GetAuthGameMode<T>()��һ����UE�����ṩ��ģ�庯�������ڻ�ȡ��ǰ�����д��ڵĲ��̳���T(AGameMode)����
	BlasterGameMode = BlasterGameMode ? BlasterGameMode : GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	// bElimmed ��ֹ�ظ�����̭��ʩ���˺�
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

	MulticastElim(bPlayerLeftGame); //����ಥ
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

	// ������̭��̫��
	PlayElimMontage();
	// ��ʼ�ܽ���Ч
	StartDisslove();
	// �رս�ɫ�ƶ�
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	// �ر�����
	bDisableGameplay = true;
	//if (BlasterPlayerController)
	//{
	//	DisableInput(BlasterPlayerController);
	//}
	// �ر��Զ�����
	if (CombatCmp)
	{
		CombatCmp->FireBtnPressed(false);
	}
	// �ر���ײ
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//������̭���ջ����� ��Ч
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());
	}
	// ������̭���ջ����� ��Ч
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}
	// �ر���׼ui
	bool bHideSniperScope = IsLocallyControlled() && CombatCmp && CombatCmp->bAiming &&
		CombatCmp->EquippedWeapon && CombatCmp->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	// �رյ�һ�Ļʹ�
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	// ���������ʱ��
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
	if (CombatCmp && CombatCmp->EquippedWeapon && bMatchNotInProgress)  //���������Ϸ������״̬ɾ������
	{
		CombatCmp->EquippedWeapon->Destroy();
	}
}


void ABlasterCharacter::SpawnDefaultWeapon()
{
	// ���ڷ�����GameMode�᷵��null
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