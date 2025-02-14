// P

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HighlightInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, BlueprintType)
class UHighlightInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class AURA_API IHighlightInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	 * 高亮显示Actor
	 */
	UFUNCTION(BlueprintNativeEvent)
	void HighlightActor();

	/**
	 * 取消高亮显示Actor
	 */
	UFUNCTION(BlueprintNativeEvent)
	void UnHighlightActor();

	/**
	 * 设置角色的移动目标位置
	 * @param OutDestination 设置目标位置值
	 */
	UFUNCTION(BlueprintNativeEvent)
	void SetMoveToLocation(FVector& OutDestination);
};
