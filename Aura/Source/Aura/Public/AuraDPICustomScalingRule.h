// P

#pragma once

#include "CoreMinimal.h"
#include "Engine/DPICustomScalingRule.h"
#include "AuraDPICustomScalingRule.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API UAuraDPICustomScalingRule : public UDPICustomScalingRule
{
	GENERATED_BODY()

public:
	virtual float GetDPIScaleBasedOnSize(FIntPoint Size) const override;

	UFUNCTION(BlueprintCallable)
	static void SetDesignedSize(float InX, float InY);

	UFUNCTION(BlueprintCallable)
	static FVector2D GetDesignedSize();

protected:
	static FVector2D DesignedSize;
};
