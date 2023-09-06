// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterTypes/CombatState.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	// TryGetPawnOwner() 函数获取当前控制器（Controller）所控制的角色（Pawn）对象
	// Cast<ABlasterCharacter>() 是一个由UE提供的类型转换函数模板，用于将 UObject 类型的对象转换为其他派生类的指针
	// 在这里，它将 TryGetPawnOwner() 函数返回的 Pawn 对象强制转换为 ABlasterCharacter 类型的指针
	// 注意：由于 TryGetPawnOwner() 返回的 Pawn 对象有可能不是 ABlasterCharacter 类型的，因此 Cast<ABlasterCharacter>() 的返回值有可能为空指针
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}



void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	//在 NativeUpdateAnimation() 中，由于更新动画的频率比初始化动画更高，因此 BlasterCharacter 可能会被其他非 C++ 代码修改或者设置为 nullptr。
	//因此，在每次执行 NativeUpdateAnimation() 函数时，都需要重新获取当前控制器所控制的角色对象，
	//并使用 Cast<ABlasterCharacter>() 进行强制类型转换，以确保 BlasterCharacter 指向正确的对象。
	//如果尝试重新获取控制器对象后返回的 Pawn 对象不是 ABlasterCharacter 类型，则 Cast<ABlasterCharacter>() 函数将返回空指针（nullptr），
	//这时候就需要再次判断 BlasterCharacter 是否为空，以避免使用一个无效的指针而导致程序崩溃。
	if (BlasterCharacter == nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr)
	{
		return;
	}

	FVector Velocity = BlasterCharacter->GetVelocity(); //获取当前角色对象的速度
	Velocity.Z = 0.f;	//将速度向量的 Z 分量设为 0，这样可以使得角色在水平面上的移动速度更符合实际效果
	Speed = Velocity.Size();	//计算速度向量的大小（即速度标量值）

	bIsInAir = BlasterCharacter->GetCharacterMovement()->IsFalling();	//通过是否正在坠落判断角色是否在空中

	//bIsAccelerating = BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f;
	//使用IsNearlyZero()避免进行浮点数比较，提高精度和计算速度，
	//在 FVector::IsNearlyZero() 函数中是可以传递一个 float 类型的参数 Tolerance 的，它表示允许的向量大小误差范围，
	//如果手动指定 Tolerance 参数为0.1f，表示允许的向量大小误差范围为 0.1 的平方，即 0.01，
	//如果没有手动指定 Tolerance 参数，则默认使用 FVector::ThreshVectorLen 来作为误差容忍值，
	//FVector::ThreshVectorLen 的默认值为 1.e-4f，代表接近于 0 的向量大小平方不能超过 0.0001。
	bIsAccelerating = !BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().IsNearlyZero();

	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();

	bIsCrouched = BlasterCharacter->bIsCrouched;

	bIsAiming = BlasterCharacter->IsAiming();

	TurningInPlace = BlasterCharacter->GetTurningInPlace();
	bRotateRootBone = BlasterCharacter->ShouldRotateRootBone();
	bElimmed = BlasterCharacter->IsElimmed();

	/*获取一个角色的移动方向和视角旋转之间的偏移量*/
	//获取角色的瞄准方向
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	//使用GetVelocity()获取到角色当前的移动速度和方向，并通过MakeRotFromX()方法将其转换为一个旋转值，表示当前移动方向的朝向。
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
	//下面这句代码直接返回非插值的旋转偏移量，可能会导致动画切换过程不够流畅，出现剧烈抖动等现象。这里会发生从负180过渡到正180，就会一下子扫过多个动画
	//因为在YawOffset变化过程中，每一帧角色的朝向都会发生比较大的变化，导致动画的过渡不够平滑。具体来说，当角色的朝向发生变化时，
	//可能会影响到角色的其它属性（如位置、旋转等），如果在过渡的过程中这些属性差异很大，就会导致动画出现突变或不流畅的情况，从而产生抖动。
	//使用NormalizedDeltaRotator函数计算MovementRotation和AimRotation之间的旋转差异，并提取其Yaw轴的值赋给YawOffset变量
	//	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
	//计算出MovementRotation与AimRotation之间的旋转差值
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	//DeltaRotation记录了上一帧与当前帧之间的旋转差值，利用RInterpTo()方法对它进行插值计算，将插值结果赋值给DeltaRotation。
	//该方法将根据提供的两个旋转值和时间，以及插值速度(6.f)计算出一个新的旋转值，该值逐渐从当前的旋转值向目标旋转值插值过渡。
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 6.f);
	YawOffset = DeltaRotation.Yaw;

	/*获取角色身体倾斜角度*/
	//记录上一帧的旋转值
	CharacterRotationLastFrame = CharacterRotation;
	//获取当前角色的旋转值
	CharacterRotation = BlasterCharacter->GetActorRotation();
	//计算出上一帧角色旋转值与当前帧角色旋转值之间的差值Delta。Delta.Yaw表示角色在Yaw轴上旋转的角度差值。
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	//(从上一帧到当前帧所经过的时间)得到每秒钟角色身体需要倾斜的角度量Target
	const float Target = Delta.Yaw / DeltaSeconds;
	//使用FInterpTo()方法在Lean(角色身体倾斜角度)和目标值Target之间进行插值计算，得到插值结果Interp。
	//该方法会自动处理插值过程中的超调问题，并确保插值结果在规定的时间内到达目标值。
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	//使用FMath::Clamp()方法将计算出来的插值结果限制在-90和90度之间，以防止过度倾斜。
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = BlasterCharacter->GetAO_Yaw();
	AO_Pitch = BlasterCharacter->GetAO_Pitch();
	//UE_LOG(LogTemp, Log, TEXT("GetAO_Yaw(): %i"), AO_Yaw);
	//UE_LOG(LogTemp, Log, TEXT("GetAO_Pitch(): %i"), AO_Pitch);

	/*将左手插座的位置和旋转信息与BlasterCharacter骨架中的左手骨骼同步，由于不同的武器、物品等左手的位置可能有所不同，因此程序应该能够动态调整左手的位置*/
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		//根据武器插槽获得左手变换位置，并以世界空间为基础。使用时转为骨骼上的骨骼空间
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		//左手插座位置从世界空间转换为BlasterCharacter骨架中右手骨骼的本地空间，并将转换后的位置和旋转信息存储在OutPosition和OutRotation变量中
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));


		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocalControlled = true;
			/* 设置右手骨骼  让有右手骨骼到我们命中的目标位置 */
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FVector RightHandLocation = RightHandTransform.GetLocation(); //Socket的世界坐标位置
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandLocation, RightHandLocation + (RightHandLocation - BlasterCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 30.f);
		}


		/* ===  Debug  ===  从枪口处画两条线来调试查看武器枪管的角度 */
		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//通过MuzzleTipTransform.GetRotation().Rotator()获取枪口变换的旋转值，并传入FRotationMatrix构造函数中，得到一个旋转矩阵。调用GetUnitAxis方法，获取旋转矩阵中X轴的单位向量。
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		//从枪口朝前放的线
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		//从枪口到命中目标的线
		DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), BlasterCharacter->GetHitTarget(), FColor::Orange);
	}

	bUseFABRIK = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied;
	bool bFABRIKOverride = BlasterCharacter->IsLocallyControlled() && 
		BlasterCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade && 
		BlasterCharacter->bFinishedSwapping;
	if (bFABRIKOverride)
	{
		//本地角色，单独处理，避免网络延迟带来的动画异常
		bUseFABRIK = !BlasterCharacter->GetIsLocallyReloading();
	}
	bUseAimOffsets = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
	bTransformRightHand = BlasterCharacter->GetCombatState() == ECombatState::ECS_Unoccupied && !BlasterCharacter->GetDisableGameplay();
}