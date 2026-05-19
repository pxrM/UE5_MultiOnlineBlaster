#pragma once

#include "CoreMinimal.h"

class UWidgetBlueprint;

class FUMGStateConfigValidator
{
public:
	static void Validate(const UWidgetBlueprint* WidgetBlueprint, TArray<FText>& OutErrors, TArray<FText>& OutWarnings, TArray<FText>& OutHints);
};
