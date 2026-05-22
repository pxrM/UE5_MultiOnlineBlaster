#pragma once

#include "CoreMinimal.h"
#include "UMGStateConfigData.h"

class UWidget;

class UMGSTATECONFIGRUNTIME_API FUMGStateConfigPropertyRuntimeLibrary
{
public:
	static bool CaptureCurrentValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, FUMGStateConfigPropertyValue& OutValue);
	static bool ApplyValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& Value);
	static bool ArePropertyValuesEqual(EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& A, const FUMGStateConfigPropertyValue& B);
};
