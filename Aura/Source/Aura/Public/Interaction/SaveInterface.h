// P

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USaveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API ISaveInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// Actor是否需要修改位置变换
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsShouldLoadTransform();

	// 在存档读取完数据后，调用更新Actor
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoadActor();
};
