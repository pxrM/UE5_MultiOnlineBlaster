#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "UMGStateConfigData.h"

class UWidget;
struct FStreamableHandle;

class UMGSTATECONFIGRUNTIME_API FUMGStateConfigPropertyRuntimeLibrary
{
public:
	static bool CaptureCurrentValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, FUMGStateConfigPropertyValue& OutValue);
	static bool ApplyValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& Value, bool bRefreshAfterApply = true);
	static bool ArePropertyValuesEqual(EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& A, const FUMGStateConfigPropertyValue& B);
	static bool IsSerializedPropertyPathAllowed(const UWidget* TargetWidget, const FString& PropertyPath);
	static void PreloadReferencedAssets(const TArray<FSoftObjectPath>& ReferencedAssets, bool bAsync = false);
	static bool AreReferencedAssetsLoaded(const TArray<FSoftObjectPath>& ReferencedAssets);
	static TSharedPtr<FStreamableHandle> RequestPreloadReferencedAssetsAsync(const TArray<FSoftObjectPath>& ReferencedAssets, TFunction<void()> OnComplete);
	static void ResetCaches();

};
