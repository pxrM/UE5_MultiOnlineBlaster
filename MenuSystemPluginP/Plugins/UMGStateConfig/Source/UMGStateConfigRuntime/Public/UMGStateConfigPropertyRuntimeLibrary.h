#pragma once

#include "CoreMinimal.h"
#include "UMGStateConfigData.h"

class UWidget;

class UMGSTATECONFIGRUNTIME_API FUMGStateConfigPropertyRuntimeLibrary
{
public:
	static bool CaptureCurrentValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, FUMGStateConfigPropertyValue& OutValue);
	static bool ApplyValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& Value, bool bRefreshAfterApply = true);
	static bool ArePropertyValuesEqual(EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& A, const FUMGStateConfigPropertyValue& B);
	static bool IsSerializedPropertyPathAllowed(const UWidget* TargetWidget, const FString& PropertyPath);
	static void PreloadReferencedAssets(const TArray<FSoftObjectPath>& ReferencedAssets, bool bAsync = false);
	static void ResetCaches();

};
