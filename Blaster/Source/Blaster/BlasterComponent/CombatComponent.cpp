// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
//#include "Blaster/HUD/BlasterHUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/ShotgunWeapon.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefultFOV = Character->GetFollowCamera()->FieldOfView;	//获取当前视野
			CurrentFOV = DefultFOV;
		}

		if (Character->HasAuthority())
		{
			//只在服务器上执行
			InitializeCarriedAmmo();
		}
	}
}

// Called every frame 
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCroshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//角色装备武器由服务器控制，这里只同步了自己，但是敌人也需要看到你的武器，所以这里加上同步给所有客户端
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	//这里是由玩家自己控制变量更改的，只有服务器玩家更改了变量会通知给其他客户端，
	//但是其他客户端变量更改后因为是在本地，所以不会触发，这里需要从客户端获取信息同步到服务器，服务器更改变量才会解决
	//使用RPC解决，像ABlasterCharacter::ServerEquipBtnPressed_Implementation()一样
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
	DOREPLIFETIME(UCombatComponent, bHoldingTheFlag);
	//只同步到对应的客户端
	DOREPLIFETIME_CONDITION(UCombatComponent, CurWeaponCarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFireBtnPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_Reloading:
		if (Character && !Character->IsLocallyControlled()) //本地机器已经处理过了
		{
			HandleReloadMag();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		// 不是本地的controller才播放，因为本地的已经播放过了
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
			ShowAttachedGrenade(true);
		}
		break;
	case ECombatState::ECS_SwappingWeapons:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage();
		}
		break;
	}
}



void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flag)
	{
		Character->Crouch();
		bHoldingTheFlag = true;
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachFlagToLeftHand(WeaponToEquip);
		WeaponToEquip->SetOwner(Character);
	}
	else
	{
		if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
		{
			EquipSecondaryWeapon(WeaponToEquip);
		}
		else
		{
			EquipPrimaryWeapon(WeaponToEquip);
		}
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::SwapWeapons()
{
	if (Character == nullptr || CombatState != ECombatState::ECS_Unoccupied) return;

	Character->PlaySwapMontage();
	CombatState = ECombatState::ECS_SwappingWeapons;
	Character->bFinishedSwapping = false;

	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false);
}

bool UCombatComponent::IsShouldSwapWeapons()
{
	return EquippedWeapon && SecondaryWeapon;
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;

	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	AttachActorToRightHand(EquippedWeapon);

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	UpdateCarriedAmmo();

	PlayEquipWeaponSound(EquippedWeapon);

	ReloadEmptyWeapon();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;

	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(WeaponToEquip);
	WeaponToEquip->SetOwner(Character);
	PlayEquipWeaponSound(WeaponToEquip);
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;	//角色将不再自动面向移动方向
		Character->bUseControllerRotationYaw = true;  //角色将使用控制器的输入来控制yaw旋转方向，而不是默认的按照移动方向转向
		PlayEquipWeaponSound(EquippedWeapon);
		EquippedWeapon->SetHUDAmmo();
	}
}

void UCombatComponent::OnRep_SecsondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		AttachActorToRightHand(SecondaryWeapon);
		PlayEquipWeaponSound(SecondaryWeapon);
	}
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped(); //如果有武器时需要先丢掉，再装备其它武器
	}
}

void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	//获取名为 RightHand_Socket 的手持骨骼插槽
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName(TEXT("RightHand_Socket")));
	if (HandSocket)
	{
		/*
		* 由于 AttachActor 函数是用于将一个 Actor 对象附加到目标组件上的函数，
		* 因此它通常要求被附加的 Actor 对象不应该拥有物理效果（SimulatePhysics=false）。
		* 如果被附加的 Actor 对象拥有物理效果，并且此时它在游戏世界中处于运动状态，那么它很有可能会与目标组件产生冲突或者碰撞，
		* 导致 AttachActor 函数失败或者造成不必要的破坏甚至崩溃。
		*
		* 这里在服务器上调用EquipWeapon，调用EquippedWeapon->SetWeaponState后会同步给客户端关闭武器的碰撞，但是这里不能保证网络速度，
		* 所以存在客户端武器的碰撞还没关 HandSocket->AttachActor执行失败，所以要在OnRep_EquippedWeapon函数中再设置一次。
		*/
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;

	bool bUsePistolScoket = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	FName SocketName = bUsePistolScoket ? FName("PistolSocket") : FName("LeftHand_Socket");
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;

	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName(TEXT("BackpackSocket")));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::AttachFlagToLeftHand(AWeapon* Flag)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || Flag == nullptr) return;

	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName(TEXT("FlagSocket")));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(Flag, Character->GetMesh());
	}
}

void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CurWeaponCarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr && Character->Controller ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CurWeaponCarriedAmmo);
	}
}

void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSouund)
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponToEquip->EquipSouund, Character->GetActorLocation());
	}
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsAmmoEmpty())
	{
		ReloadMag();
	}
}

void UCombatComponent::OnRep_HoldingTheFlag()
{
	if (bHoldingTheFlag && Character && Character->IsLocallyControlled())
	{
		Character->Crouch();
	}
}



void UCombatComponent::FireBtnPressed(bool bPressed)
{
	bFireBtnPressed = bPressed;
	if (bFireBtnPressed && EquippedWeapon)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.75f;
			switch (EquippedWeapon->GetFireType())
			{
			case EFireType::EFT_Projectile:
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan:
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun:
				FireShotgunWeapon();
				break;
			default:
				break;
			}
		}
		StartFireTimer();
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		// 在调用本地和服务器之前设置命中目标的值
		HitTarget = EquippedWeapon->bUseSactter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority())
		{
			LocalFire(HitTarget); //在自己机器上直接走流程
		}
		ServerFire(HitTarget, EquippedWeapon->FireDelay); //调用服务器函数 ServerFire_Implementation
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		// 在调用本地和服务器之前设置命中目标的值
		HitTarget = EquippedWeapon->bUseSactter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		if (!Character->HasAuthority())
		{
			LocalFire(HitTarget);
		}
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

void UCombatComponent::FireShotgunWeapon()
{
	if (EquippedWeapon && Character)
	{
		AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(EquippedWeapon);
		if (Shotgun)
		{
			TArray<FVector_NetQuantize> HitTargets;
			Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
			if (!Character->HasAuthority())
			{
				LocalShotgunFire(HitTargets);
			}
			ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	MulticastFire(TraceHitTarget); //通知所有客户端
}

bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	// ServerFire rpc验证函数
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//在自己机器上已经走玩流程了
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;

	LocalFire(TraceHitTarget);
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	MulticastShotgunFire(TraceHitTargets);
}

bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTarget, float FireDelay)
{
	// ServerShotgunFire rpc验证函数
	if (EquippedWeapon)
	{
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;

	LocalShotgunFire(TraceHitTargets);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	// LocalShotgunFire() 专门处理
	//if (CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
	//{
	//	Character->PlayFireMontage(bAiming);
	//	EquippedWeapon->Fire(TraceHitTarget);
	//	CombatState = ECombatState::ECS_Unoccupied;
	//	return;
	//}

	if (CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(EquippedWeapon);
	if (Character == nullptr || Shotgun == nullptr) return;

	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming);
		Shotgun->FireShotgun(TraceHitTargets);
		CombatState = ECombatState::ECS_Unoccupied;
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr)
		return false;

	//霰弹枪特殊处理
	if (!EquippedWeapon->IsAmmoEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading
		&& EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun)
		return true;

	if (bLocallyReloading)
		return false;

	return !EquippedWeapon->IsAmmoEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	if (EquippedWeapon->bAutomatic)
	{
		// FTimerHandle 的结构体，用于标识该定时器
		// 定时器到期后要调用的函数的对象
		// 定时器到期后要调用的函数的名称
		// 定时器的持续时间，即 FireDelay 参数，以秒为单位。
		Character->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
	}
	else
	{
		bCanFire = true;
		ReloadEmptyWeapon();
	}
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr)return;

	bCanFire = true;
	if (bFireBtnPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}

	ReloadEmptyWeapon();
}



void UCombatComponent::ThrowGrenade()
{
	// 当前机器本地执行逻辑
	if (Grenades == 0) return;
	if (Character == nullptr || EquippedWeapon == nullptr || CombatState != ECombatState::ECS_Unoccupied) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	Character->PlayThrowGrenadeMontage();
	AttachActorToLeftHand(EquippedWeapon);
	ShowAttachedGrenade(true);
	if (!Character->HasAuthority())
	{
		//如果是在客户端就通知服务器
		ServerThrowGrenade();
	}
	else
	{
		//服务器机器直接处理
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenades();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenades();
}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	// 投掷手榴弹结束时在所有机器上调用
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	/*开始扔出手榴弹*/
	//隐藏手上的手榴弹
	ShowAttachedGrenade(false);
	if (Character->IsLocallyControlled())
	{
		// 如果是本地控制器，调用服务器函数并传入当前攻击目标点
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (GrenadeClass && Character && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		FVector ToTarget = Target - StartingLocation;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;
		UWorld* World = GetWorld();
		if (World)
		{
			// 生成手榴弹
			World->SpawnActor<AProjectile>(GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParams);
		}
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades();
}

void UCombatComponent::UpdateHUDGrenades()
{
	Controller = Controller == nullptr && Character->Controller ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades);
	}
}



void UCombatComponent::TraceUnderCroshairs(FHitResult& TraceHitResult)
{
	//1.获取当前游戏视口大小并计算交叉准心位置
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize); //从引擎中获取了当前游戏视口的大小，并将其存储在 ViewportSize 变量中
	}
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); //准心位置在屏幕中心的位置
	//2.将屏幕坐标转为世界坐标的点和方向
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection
	);
	//3.进行射线
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		if (Character)
		{
			//获取起始位置和角色得差距，将起始位置移到角色前面，避免射线射向角色后面
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 60.f);
			//DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red, false);
		}
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;
		//从Start到End的射线进行检测，并将结果存储在TraceHitResult变量中。ECC_Visibility表示射线只会与阻挡视线的物体发生碰撞，忽略不可见的物体，提高射线检测的性能。
		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (!TraceHitResult.bBlockingHit)
		{
			//如果射线没有击中任何阻挡视线的物体（即没有产生阻挡碰撞），则将 "TraceHitResult.ImpactPoint" 设置为终点 "End"。
			TraceHitResult.ImpactPoint = End;
		}

		//检查actor是否有效并实现了InteractWithCrosshairsInterface接口
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr)return;

	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			// 十字准线的扩散
			// FMath::GetMappedRangeValueClamped()	 [0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				CrosshairInAimFactor = FMath::FInterpTo(CrosshairInAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				CrosshairInAimFactor = FMath::FInterpTo(CrosshairInAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread = 0.5f/*基础值*/ + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairInAimFactor + CrosshairShootingFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}


void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	if (Character->IsLocallyControlled())
	{
		bAimBtnPressed = bIsAiming;
	}
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimBtnPressed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)return;

	if (Character && Character->GetFollowCamera())
	{
		if (bAiming)
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
		}
		else
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, DefultFOV, DeltaTime, ZoomInterpSpeed);
		}

		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}


void UCombatComponent::ReloadMag()
{
	if (CurWeaponCarriedAmmo > 0 &&
		CombatState == ECombatState::ECS_Unoccupied &&
		EquippedWeapon && !EquippedWeapon->IsAmmoFull() &&
		!bLocallyReloading)
	{
		ServerReloadMag();
		HandleReloadMag(); //直接本地机器先处理
		bLocallyReloading = true;
	}
}

void UCombatComponent::ServerReloadMag_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	if (!Character->IsLocallyControlled()) //本地机器已经处理
	{
		HandleReloadMag();
	}
}

void UCombatComponent::HandleReloadMag()
{
	if (Character)
	{
		Character->PlayReloadMagMontage();
	}
}

int32 UCombatComponent::AmountToReloadMag()
{
	if (EquippedWeapon)
	{
		int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmoNum();
		if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		{
			int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
			int32 Least = FMath::Min(RoomInMag, AmountCarried);
			return FMath::Clamp(RoomInMag, 0, Least);
		}
	}
	return 0;
}

void UCombatComponent::FinishReloadMag()
{
	if (Character == nullptr)return;
	bLocallyReloading = false;
	if (Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireBtnPressed)
	{
		Fire();
	}
}

void UCombatComponent::FinishSwapMontage()
{
	if (Character && Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		Character->bFinishedSwapping = true;
	}

	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true);
}

void UCombatComponent::FinishSwapAttachWeapon()
{
	if (Character == nullptr || !Character->HasAuthority()) return;

	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	int32 ReloadAmount = AmountToReloadMag();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount; //更新携带武器的弹药量
		CurWeaponCarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr && Character->Controller ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CurWeaponCarriedAmmo);
	}
	EquippedWeapon->AddAmmo(ReloadAmount); //弹夹添加子弹
}

void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1;
		CurWeaponCarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr && Character->Controller ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CurWeaponCarriedAmmo);
	}
	EquippedWeapon->AddAmmo(1);
	bCanFire = true; //霰弹枪支持添加子弹途中切换到开火
	if (EquippedWeapon->IsAmmoFull() || CurWeaponCarriedAmmo == 0)
	{
		AnimJumpToShotgunEnd();
	}
}

void UCombatComponent::OnRep_CurWeaponCarriedAmmo()
{
	Controller = Controller == nullptr && Character->Controller ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CurWeaponCarriedAmmo);
	}

	bool bJumpToShotgunEnd = CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CurWeaponCarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		AnimJumpToShotgunEnd();
	}
}

void UCombatComponent::AnimJumpToShotgunEnd()
{
	//弹夹满了，直接跳转的换单结束动画
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::ShotgunShellReload()
{
	if (Character == nullptr)return;
	if (Character->HasAuthority())
	{
		UpdateShotgunAmmoValues();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	//Map.Emplace是ue中用于往 Map 容器中添加元素的一种方法。
	//它的作用类似于 Map.Add() 或 Map.Insert(), 但是相比于这两种方法，Map.Emplace() 有以下几个优势：
	//	 避免重复的创建和销毁元素对象，从而提高代码的性能和效率。
	//	 使用移动构造函数和完美转发技术进行元素的构造和插入，从而减少内存分配和拷贝操作。
	//	 支持变长参数列表，从而方便地向容器中添加不同类型和数量的元素。
	//需要注意的是，在使用Emplace方法添加元素时，应该确保元素的键值不会重复。
	//如果键值已经存在于容器中，则Emplace方法将忽略该插入操作，并返回指向已有元素的迭代器。
	//如果键值不存在，则Emplace方法将创建一个新元素并返回指向该元素的迭代器。

	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLacuncher, StartingGrenadeAmmo);
}


void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo();
	}
	//添加弹药后，检查当前装备武器的弹夹是否是空的，如果空且有弹药，自动更换弹夹
	if (EquippedWeapon && EquippedWeapon->IsAmmoEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		ReloadMag();
	}
}