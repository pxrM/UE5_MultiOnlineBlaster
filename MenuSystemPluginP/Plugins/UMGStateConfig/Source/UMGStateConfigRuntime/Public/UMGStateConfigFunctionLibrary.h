#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UMGStateConfigFunctionLibrary.generated.h"

class UUserWidget;
class UUMGStateConfigUserWidgetExtension;

UCLASS()
class UMGSTATECONFIGRUNTIME_API UUMGStateConfigFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UMG State Config")
	static bool ApplyUIState(UUserWidget* TargetWidget, FName StateGroupName, FName StateName);

	static UUMGStateConfigUserWidgetExtension* FindOrCreateStateConfigExtension(UUserWidget* TargetWidget);
};
