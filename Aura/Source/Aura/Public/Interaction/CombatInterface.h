// P

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatInterface.generated.h"

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
	virtual int32 GetPlayerLevel();
	// 获取武器上的插槽位置
	virtual FVector GetCombatSocketLocation();

	// 更新角色面向攻击目标的方向
	// BlueprintImplementableEvent：具体实现由蓝图来决定，而不是在C++中直接实现
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFaceingTarget(const FVector& Target);
};
