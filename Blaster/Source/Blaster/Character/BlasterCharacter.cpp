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

	//��ɫ����󶨰���
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
		const FRotator YawRotator(0.f, Controller->GetControlRotation().Yaw, 0.f);	//ƫ����
		const FVector Direction(FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X)); //��ȡ��ǰ����
		AddMovementInput(Direction, Value);
	}
}

void ABlasterCharacter::MoveRight(float Value)
{
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
	//ʰȡ������Ҫ����������֤
	if (CombatCmp && HasAuthority())
	{
		CombatCmp->EquipWeapon(OverlappingWeapon);
	}
}

/*--------------------------------------input--------------------------------------------*/

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//ʹ�� DOREPLIFETIME �꽫 OverlappingWeapon ������ӵ���Ҫ��������ͬ���������б��С�
	//DOREPLIFETIME ��ĵ�һ��������Ҫͬ���ı��������ڶ���������Ҫ��ӵ��� OutLifetimeProps ���顣
	//�ڶ�����Ϸ�У�ÿ�� OverlappingWeapon ��ֵ�����仯ʱ�������Զ�������dsͬ���������ͻ��ˡ��������ͻ����ϣ���ֵ���Զ�������ƥ��������ϵ�ֵ��
	//��Ҫע����ǣ������� GetLifetimeReplicatedProps ��������������⣬
	//����Ҫȷ��������Ҫͬ�������Զ�ʹ���� UPROPERTY(Replicated) ��ʶ�������˱�ǡ�������Щ���Կ����޷�����ȷͬ����
	//DOREPLIFETIME(ABlasterCharacter, OverlappingWeapon);
	// 
	//DOREPLIFETIME_CONDITION �� Unreal Engine ����������ͬ���ĺ�֮һ�����ڱ����Ҫ��������ͬ�������ԣ���ָ��ͬ����������
	//������������ͬ������������ COND_OwnerOnly,��ʾ���ھ�������Ȩ�Ŀͻ��˺ͷ�����֮��ͬ��������
	//��ʾֻ�о�������Ȩ�Ŀͻ��˺ͷ�����֮���ͬ����������ή��������������ά������һ���ԣ���Ϊͨ�������ֻ�о�������Ȩ�Ŀͻ��˿��Ը���ĳ����������ԡ�
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
