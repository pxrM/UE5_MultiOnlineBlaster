#pragma once

#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"


USTRUCT(BlueprintType)
struct FAuraGameplayEffectContent : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	// 返回用于序列化的实际结构体函数。
	// 序列化是将对象的状态转换为可以存储或传输的形式的过程，反序列化则是相反的过程。在游戏开发和网络通信中，序列化和反序列化是非常重要的。
	virtual UScriptStruct* GetScriptStruct() const override
	{
		return FGameplayEffectContext::StaticStruct();
	}

	// 覆写用于序列化配置的函数（确认该结构体的网络序列化方式），将对应的参数保存，这样在服务器端，可以完整的复原这个实例。
	// FArchive& Ar: 是一个文件存档对象的引用，用于实际的序列化和反序列化操作。
	//				Ar.SerializeBits(&RepBits, 7); 告诉这个地址的数据前七位数据有效
	// UPackageMap* Map: 是一个包映射对象的指针（将对象映射到网络通信数据索引），用于处理包含的对象序列化。
	// bool& bOutSuccess: 一个输出参数，用于指示序列化是否成功完成。
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	bool GetIsCriticalHit() const { return bIsCriticalHit; }
	bool GetIsBlockedHit() const { return bIsBlockedHit; }

	void SetIsCriticalHit(const bool bInCriticalHit) { bIsCriticalHit = bInCriticalHit; }
	void SetIsBlockedHit(const bool bInBlockedHit) { bIsBlockedHit = bInBlockedHit; }

	
protected:
	// 是否成功格挡
	UPROPERTY()
	bool bIsBlockedHit = false;

	// 是否暴击
	UPROPERTY()
	bool bIsCriticalHit = false;
};
