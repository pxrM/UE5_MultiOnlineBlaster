// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"
#include "BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	RefreshBlasterCharacter();
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
		RefreshBlasterCharacter();
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
}

void UBlasterAnimInstance::RefreshBlasterCharacter()
{
	//通过将获取 BlasterCharacter 的逻辑封装到 RefreshBlasterCharacter() 函数中，并在需要使用 BlasterCharacter 指针时调用该函数，
	//可以避免在每次执行 NativeUpdateAnimation() 函数时都重新获取对象并进行类型转换的操作。
	// 
	// TryGetPawnOwner() 函数获取当前控制器（Controller）所控制的角色（Pawn）对象
	// Cast<ABlasterCharacter>() 是一个由UE提供的类型转换函数模板，用于将 UObject 类型的对象转换为其他派生类的指针
	// 在这里，它将 TryGetPawnOwner() 函数返回的 Pawn 对象强制转换为 ABlasterCharacter 类型的指针
	// 注意：由于 TryGetPawnOwner() 返回的 Pawn 对象有可能不是 ABlasterCharacter 类型的，因此 Cast<ABlasterCharacter>() 的返回值有可能为空指针
	BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
}
