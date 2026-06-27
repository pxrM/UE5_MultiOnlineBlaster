#pragma once

#include "CoreMinimal.h"

class IPropertyHandle;
class UUserWidget;
class UWidgetAnimation;
class UWidgetBlueprint;

class FWidgetAnimTimelineEditorUtils
{
public:
	static UWidgetBlueprint* ResolveWidgetBlueprint(const TSharedPtr<IPropertyHandle>& PropertyHandle);
	static UClass* ResolveOwnerWidgetClass(const TSharedPtr<IPropertyHandle>& PropertyHandle, UWidgetBlueprint* WidgetBlueprint);
	static UClass* ResolveTargetWidgetClass(const UWidgetBlueprint* WidgetBlueprint, UClass* OwnerClass, FName TargetWidgetName);
	static UWidgetAnimation* ResolveAnimation(const UUserWidget* TargetWidget, FName AnimationName);
	static UWidgetAnimation* ResolveAnimationFromClassDefaultObject(const UClass* TargetClass, FName AnimationName);
	static void CollectTargetWidgetNames(const UWidgetBlueprint* WidgetBlueprint, const UClass* OwnerClass, TArray<FName>& OutTargetWidgetNames);
	static void CollectAnimationNames(const UClass* TargetClass, TArray<FName>& OutAnimationNames);
	static void CollectChildPhaseNames(const UClass* TargetClass, FName ExcludedPhaseName, TArray<FName>& OutPhaseNames);
	static bool HasAnimation(const UClass* TargetClass, FName AnimationName);
	static FString StripInstSuffix(const FString& AnimationName);

private:
	static UWidgetBlueprint* ResolveWidgetBlueprintFromObject(UObject* Object);
};
