// P

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API IPlayerInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 向playerstate添加经验
	UFUNCTION(BlueprintNativeEvent)
	void AddToXP(int32 InXP);

	// 升级
	UFUNCTION(BlueprintNativeEvent)
	void LevelUp();
};
