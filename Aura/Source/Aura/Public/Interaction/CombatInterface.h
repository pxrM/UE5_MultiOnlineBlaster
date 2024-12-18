// P

/*
 * UFUNCTION(BlueprintNativeEvent)
 * int32 GetPlayerLevel();
 *	1. BlueprintNativeEvent 表示这个方法有一个C++的默认实现，但也可以在蓝图中重写。
 *	2. virtual int32 GetPlayerLevel_Implementation() override;
 *	   这是 GetPlayerLevel 方法的C++默认实现部分。_Implementation 是Unreal Engine的约定，指示这是实际的C++实现。
 *	3. ICombatInterface::Execute_GetPlayerLevel(TargetAvatar)
 *		这是接口方法的执行调用方式。ICombatInterface 是一个接口类，其中定义了 GetPlayerLevel 方法。
 *		Execute_GetPlayerLevel 是一个静态方法，用于从接口中调用 GetPlayerLevel 方法，并传递 TargetAvatar 作为目标对象。
 *		hud生成的code：
 			// 定义了一个静态 FName 类型的变量 NAME_UCombatInterface_GetPlayerLevel，其值为 "GetPlayerLevel"。
			static FName NAME_UCombatInterface_GetPlayerLevel = FName(TEXT("GetPlayerLevel"));
  			int32 ICombatInterface::Execute_GetPlayerLevel(UObject* O)
			{
				// 确保传入的 UObject 指针 O 不为空。
				check(O != NULL);
				// 确保对象 O 实现了 UCombatInterface 接口。
				check(O->GetClass()->ImplementsInterface(UCombatInterface::StaticClass()));
				// 定义了一个 CombatInterface_eventGetPlayerLevel_Parms 结构体实例 Parms。
				// 这是一个用于存储方法调用参数和返回值的结构体。它通常由UE自动生成，用于存储 GetPlayerLevel 方法的返回值。
				CombatInterface_eventGetPlayerLevel_Parms Parms;
				// 查找对象 O 上的函数，包括蓝图中定义的函数。
				UFunction* const Func = O->FindFunction(NAME_UCombatInterface_GetPlayerLevel);
				if (Func)
				{
					// 不为空则使用 O->ProcessEvent 调用这个方法，并传递 Parms 作为参数。ProcessEvent 是UE的反射系统的一部分，允许在运行时调用方法。
					O->ProcessEvent(Func, &Parms);
				}
				else if (auto I = (ICombatInterface*)(O->GetNativeInterfaceAddress(UCombatInterface::StaticClass())))
				{
					// 如果没有找到 UFunction，则尝试从对象 O 的接口地址中获取 ICombatInterface 的实现。
					// GetNativeInterfaceAddress 返回实现接口的 C++ 对象的地址。调用接口 I 的 C++ 实现的 GetPlayerLevel_Implementation 方法。
					Parms.ReturnValue = I->GetPlayerLevel_Implementation();
				}
				// 返回 Parms 结构体中的 ReturnValue，即 GetPlayerLevel 方法的返回值。
				return Parms.ReturnValue;
			}
 * 这种实现方式允许你在运行时灵活地调用接口方法，并处理不同类型的实现，增强了系统的扩展性和可维护性。
 *	C++端的默认实现必须写成“”函数名“_Implementation”
 *	在C++端调用函数时要小心。如果调用时添加_Implementation，则只会调用C++端的实现。
 *	如果调用时不添加_Implementation，则只会调用Blueprint端的实现，如果此时Node端没有调用Parent，则不会执行C++端的处理。
 *	
 * 在 Unreal Engine 中，使用 Execute_<FunctionName> 的好处主要体现在以下几个方面，特别是在结合 BlueprintNativeEvent 使用时：
 *	1. 确保正确调用：
 *		这个方法的主要作用是确保调用的是目标对象上定义的实现，而不仅仅是基础类或接口中定义的虚函数。
 *		这对于多态性至关重要，因为它确保了即使对象类型被强制转换或者使用了基类指针，实际调用的也是目标对象的正确实现。
 *	2. 避免直接调用：
 *		可以避免直接调用蓝图或 C++ 中的虚函数。这种方法在编译时生成，并通过内部机制确保了正确的调用。直接调用虚函数可能不够安全，因为它可能会跳过蓝图中定义的实现。
 *	3. 简化跨语言调用：
 *		简化了从 C++ 代码调用蓝图实现的过程。它能够处理各种潜在的类型转换和封装问题，使得 C++ 代码能够安全地调用蓝图中实现的功能。
 *	4. 类型安全：
 *		这种调用方式是类型安全的，因为 Execute_<FunctionName> 会确保只有正确的类型可以调用指定的功能。它会检查对象的实际类型，确保调用的实现是正确的，减少了运行时错误的可能性。
 *	5. 提高代码清晰度：
 *		它明确了调用的是接口或基类中定义的方法，而不是任意可能存在的实现。这种方式使得代码更具可读性，并且更加符合 UE 的设计规范。
 */

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"


class UAbilitySystemComponent;
class UNiagaraSystem;


DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, UAbilitySystemComponent*);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor);


// 标签对应蒙太奇动画的接口体，为了在普攻GA里兼容多种攻击方式
USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()

	// 使用的蒙太奇 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage;

	// 对应tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	// 识别socket的tag
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SocketTag;

	// 攻击时触发伤害的骨骼插槽名
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName CombatTipSocketName;

	// 攻击音效
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};


// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API ICombatInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 获取等级
	UFUNCTION(BlueprintNativeEvent)
	int32 GetPlayerLevel();

	// 获取武器上的插槽位置 (BlueprintNativeEvent，会通过蓝图初始化虚函数，可以在蓝图中覆写（如果在蓝图中覆写，C++版本的实现将会失效）)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocation(const FGameplayTag& SocketTag);

	// 更新角色面向攻击目标的方向
	// BlueprintImplementableEvent：具体实现由蓝图来决定，而不是在C++中直接实现
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& Target);

	// 受击蒙太奇
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	// server：角色死亡
	virtual void Die(const FVector& InDeathImpulse) = 0;

	// 获取角色是否死亡
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDead() const;

	// 获取角色的Avatar
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* GetAvatar();

	// 获取角色身上设置的多个普攻动画
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FTaggedMontage> GetAttackMontages() const;

	// 获取角色流血效果
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNiagaraSystem* GetBloodEffect();

	// 根据动画标签获取 FTaggedMontage
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FTaggedMontage GetTaggedMontageByTag(const FGameplayTag& MontageTag);

	// 获取角色的小兵数量
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetMinionCount();
	// 增加小兵数量
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncrementalMinionCount(const int32 Amount);

	// 获取角色类型
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	ECharacterClassType GetCharacterType();

	// 获取角色asc组件准备成功回调
	virtual FOnASCRegistered& GetOnAscRegisteredDelegate() = 0;
	
	// 获取角色死亡回调
	virtual FOnDeath& GetOnDeathDelegate() = 0;

	// 设置当前是否处于持续施法状态
	// BlueprintImplementableEvent：具体实现由蓝图来决定，而不是在C++中直接实现
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetInShockLoop(const bool bInShockLoop);

	// 获取武器
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetWeapon();
};
