// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UIScriptPayload.generated.h"

/**
 * Script-runtime-neutral payload for passing structured data through UI requests.
 * UnLua/Puerts adapters may subclass this or use Json plus named UObject references.
 */
UCLASS(BlueprintType, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUIScriptPayload : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Scripting")
	FName TypeName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Scripting", meta = (MultiLine = true))
	FString Json;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI|Scripting")
	TMap<FName, TObjectPtr<UObject>> ObjectReferences;
};
