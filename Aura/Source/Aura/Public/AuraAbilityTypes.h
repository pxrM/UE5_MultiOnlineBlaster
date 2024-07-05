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

	// 覆写用于序列化配置的函数（确认该结构体的序列化方式），将对应的参数保存，这样在服务器端，可以完整的复原这个实例。
	virtual bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess) override;

	bool GetIsCriticalHit() const { return bIsCriticalHit; }
	bool GetIsBlockedHit() const { return bIsBlockedHit; }

	void SetIsCriticalHit(const bool bInCriticalHit) { bIsCriticalHit = bInCriticalHit; }
	void SetIsBlockedHit(const bool bInBlockedHit) { bIsCriticalHit = bInBlockedHit; }

protected:
	// 是否成功格挡
	UPROPERTY()
	bool bIsBlockedHit = false;

	// 是否暴击
	UPROPERTY()
	bool bIsCriticalHit = false;
};
