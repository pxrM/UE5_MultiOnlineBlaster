#pragma once

#include "CoreMinimal.h"
#include "WidgetBlueprintExtension.h"
#include "UMGStateConfigData.h"
#include "UMGStateConfigBlueprintExtension.generated.h"

UCLASS()
class UMGSTATECONFIGEDITOR_API UUMGStateConfigBlueprintExtension : public UWidgetBlueprintExtension
{
	GENERATED_BODY()

public:
	static UUMGStateConfigBlueprintExtension* Request(UWidgetBlueprint* WidgetBlueprint);
	static UUMGStateConfigBlueprintExtension* Find(const UWidgetBlueprint* WidgetBlueprint);

	UPROPERTY(EditAnywhere, Category = "UMG State Config")
	FUMGStateConfigRuntimeData ConfigData;

protected:
	virtual void HandleCopyTermDefaultsToDefaultObject(UObject* DefaultObject) override;
};
