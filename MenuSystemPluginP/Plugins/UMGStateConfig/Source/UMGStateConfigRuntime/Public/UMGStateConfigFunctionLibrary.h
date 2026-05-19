#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UMGStateConfigFunctionLibrary.generated.h"

class UUserWidget;
class UUUsers\userb85bbe4d\projectsGStateConfigUserWidgetExtension;

UCLASS()
class UMGSTATECONFIGRUNTIME_API UUUsers\userb85bbe4d\projectsGStateConfigFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "UMG State Config")
	static bool ApplyUIState(UUserWidget* TargetWidget, FName StateGroupName, FName StateName);

	static UUMGStateConfigUserWidgetExtension* FindOrCreateStateConfigExtension(UUserWidget* TargetWidget);
};
