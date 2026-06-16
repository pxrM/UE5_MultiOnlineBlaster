#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UMGStateConfigSettings.generated.h"

/**
 * Project-level policy for which widget properties UMG State Config may capture and apply.
 * Layers on top of the built-in reflection policy (editable + safety floor + danger blacklist).
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="UMG State Config"))
class UMGSTATECONFIGRUNTIME_API UUMGStateConfigSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// When true (default), any editor-editable (EditAnywhere) property is configurable,
	// minus the danger blacklist. When false, only AdditionalAllowed* entries are permitted.
	UPROPERTY(EditAnywhere, Config, Category = "Property Policy")
	bool bAllowAllEditableProperties = true;

	// Extra property leaf names allowed even if not EditAnywhere (project opt-in).
	UPROPERTY(EditAnywhere, Config, Category = "Property Policy")
	TArray<FName> AdditionalAllowedPropertyNames;

	// Extra full property paths (e.g. "Brush.DrawAs") allowed explicitly.
	UPROPERTY(EditAnywhere, Config, Category = "Property Policy")
	TArray<FString> AdditionalAllowedPropertyPaths;

	// Property leaf names always denied, even if EditAnywhere.
	UPROPERTY(EditAnywhere, Config, Category = "Property Policy")
	TArray<FName> BlockedPropertyNames;

	virtual FName GetCategoryName() const override { return FName(TEXT("Plugins")); }
};
