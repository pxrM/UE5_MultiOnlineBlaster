#pragma once

#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"


class UGameplayEffect;

/*
 * 给目标应用负面效果的参数结构
 */
USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams(){}

	UPROPERTY()
	TObjectPtr<UObject> WorldContextObject = nullptr; // 当前场景的上下文对象

	UPROPERTY()
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr; // 需要应用的ge

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemCmp; // 源asc

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemCmp; // 目标asc

	UPROPERTY()
	TMap<FGameplayTag, float> DamageTypesValues; //技能造成的多种伤害类型的伤害

	UPROPERTY()
	float AbilityLevel = 1.f; // 技能等级

	UPROPERTY()
	float DeBuffChance = 0.f; // 减益效果触发概率

	UPROPERTY()
	float DeBuffDamage = 0.f; // 减益效果触发伤害

	UPROPERTY()
	float DeBuffDuration = 0.f; // 减益效果触发间隔时间

	UPROPERTY()
	float DeBuffFrequency = 0.f; // 减益效果触发持续时间
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	// 返回用于序列化的实际结构体函数。
	// 序列化是将对象的状态转换为可以存储或传输的形式的过程，反序列化则是相反的过程。在游戏开发和网络通信中，序列化和反序列化是非常重要的。
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayEffectContext::StaticStruct();
	}

	// 首先进行浅拷贝复制基本数据成员，然后根据需要进行命中结果的深拷贝
	virtual FGameplayEffectContext* Duplicate() const override
	{
		FGameplayEffectContext* NewContext = new FGameplayEffectContext();
		*NewContext = *this;	// 浅拷贝
		if (GetHitResult())
		{
			// Does a deep copy of the hit result
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	// 覆写用于序列化配置的函数（确认该结构体的网络序列化方式），将对应的参数保存，这样在服务器端，可以完整的复原这个实例。
	// FArchive& Ar: 是一个文件存档对象的引用，用于实际的序列化和反序列化操作。
	//				Ar.SerializeBits(&RepBits, 7); 告诉这个地址的数据前七位数据有效
	// UPackageMap* Map: 是一个包映射对象的指针（将对象映射到网络通信数据索引），用于处理包含的对象序列化。
	// bool& bOutSuccess: 一个输出参数，用于指示序列化是否成功完成。
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

public:
	bool GetIsCriticalHit() const { return bIsCriticalHit; }
	bool GetIsBlockedHit() const { return bIsBlockedHit; }
	bool GetIsSuccessfulDeBuff() const { return bIsSuccessfulDeBuff; }
	float GetDeBuffDamage() const { return DeBuffDamage; }
	float GetDeBuffDuration() const { return DeBuffDuration; }
	float GetDeBuffFrequency() const { return DeBuffFrequency; }
	TSharedPtr<FGameplayTag> GetDamageType() const { return DamageType; }

	void SetIsCriticalHit(const bool bInCriticalHit) { bIsCriticalHit = bInCriticalHit; }
	void SetIsBlockedHit(const bool bInBlockedHit) { bIsBlockedHit = bInBlockedHit; }
	void SetIsSuccessfulDeBuff(const bool bInSuccessfulDeBuff) { bIsSuccessfulDeBuff = bInSuccessfulDeBuff; }
	void SetDeBuffDamage(const float InDeBuffDamage) { DeBuffDamage = InDeBuffDamage; }
	void SetDeBuffDuration(const float InDeBuffDuration) { DeBuffDuration = InDeBuffDuration; }
	void SetDeBuffFrequency(const float InDeBuffFrequency) { DeBuffFrequency = InDeBuffFrequency; }
	void SetDamageType(const TSharedPtr<FGameplayTag>& InDamageType) { DamageType = InDamageType; }
	
protected:
	// 是否成功格挡
	UPROPERTY()
	bool bIsBlockedHit = false;

	// 是否暴击
	UPROPERTY()
	bool bIsCriticalHit = false;

	// debuff是否成功应用
	UPROPERTY()
	bool bIsSuccessfulDeBuff = false;
	// debuff伤害
	UPROPERTY()
	float DeBuffDamage = 0.f;
	// debuff持续时间
	UPROPERTY()
	float DeBuffDuration = 0.f;
	// debuff触发间隔时间
	UPROPERTY()
	float DeBuffFrequency = 0.f;

	// 伤害类型
	TSharedPtr<FGameplayTag> DamageType;
};



/*
 * 特化一个自定义的结构体（struct）通常是为了在特定的情境下定制其行为和功能。以下是几个常见需要特化结构体的场景：
 *	定制化序列化和反序列化：
 *		在使用类似于 Unreal Engine 或其他游戏引擎的开发环境中，结构体的序列化（将结构体转换为可传输或存储的格式）
 *		和反序列化（从存储或传输的格式还原为结构体）是非常常见的操作。特化结构体可以确保它们能够正确地进行序列化和反序列化，以适应引擎的数据管理机制。
 *	网络传输：
 *		当结构体需要在网络上进行传输时，特化可以确保它们可以被正确地序列化和反序列化，以便在客户端和服务器之间进行数据交换。
 *	复制和比较：
 *		某些结构体可能需要自定义的复制行为，例如深度复制或特定的浅复制行为。特化可以确保结构体在复制时的行为符合预期。
 *		特化可以定义结构体的比较操作，以确保结构体能够正确地进行相等性比较或排序操作。
 *	性能优化：
 *		在需要处理大量结构体实例的情况下，通过特化可以优化结构体的内存布局或访问模式，以提高运行时性能。
 *	与第三方库集成：
 *		当结构体需要与外部的第三方库或模块进行交互时，特化可以确保结构体在与这些库集成时能够按照预期进行操作。
 *	接口适配：
 *		结构体需要适配特定的接口或协议，以便与现有的代码或系统进行交互。特化可以帮助确保结构体在接口适配方面具有良好的兼容性和一致性。
 *---------
 *	UE里通过 TStructOpsTypeTraits / TStructOpsTypeTraitsBase2 来实现具体 UStruct 类型的萃取，
 *	通过特化一个自定义的 struct 来告诉 UE 自己的类型信息，实现了什么什么对应的接口，以供其调用，一般是最后指针强转调用。
 *---------
 *	为什么ue里要使用Traits(特化)：
 *		因为对于继承UObject 的类来说，比如 同步，初始化，序列化等等功能，都有定义的接口用于重载，所以对这些继承UObject 的类，可以有很大的空间去实现自定义的方法。
 *		但对于 UStruct 的类，并没有一个统一的基类，并没有一个上层的抽象接口，来用于统一调用。
 *		因此类似于 STL 里面的 Traits，通过添加一个中间层，实现具体容器类型的萃取，在编译期就能得到类型信息。
 *	  比如 WithIdenticalViaEquality 就是告诉它自己有 EStructFlags::STRUCT_IdenticalNative 标记，实现了自定义 IsIdentical 的方法，可以直接使用 operator== 来判定。
 */
template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true,
	};
};
