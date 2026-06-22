#pragma once

#include "CoreMinimal.h"

class IPropertyHandle;
class UUserWidget;
class UWidgetAnimation;
class UWidgetBlueprint;

class FWidgetAnimTimelineEditorUtils
{
public:
	static UWidgetBlueprint* ResolveWidgetBlueprint(TSharedPtr<IPropertyHandle> PropertyHandle);
	static UClass* ResolveOwnerWidgetClass(TSharedPtr<IPropertyHandle> PropertyHandle, UWidgetBlueprint* WidgetBlueprint);
	static UClass* ResolveTargetWidgetClass(UWidgetBlueprint* WidgetBlueprint, UClass* OwnerClass, FName TargetWidgetName);
	static UWidgetAnimation* ResolveAnimation(UUserWidget* TargetWidget, FName AnimationName);
	static UWidgetAnimation* ResolveAnimationFromClassDefaultObject(UClass* TargetClass, FName AnimationName);
	static void CollectTargetWidgetNames(UWidgetBlueprint* WidgetBlueprint, UClass* OwnerClass, TArray<FName>& OutTargetWidgetNames);
	static void CollectAnimationNames(UClass* TargetClass, TArray<FName>& OutAnimationNames);
	static void CollectChildPhaseNames(UClass* TargetClass, FName ExcludedPhaseName, TArray<FName>& OutPhaseNames);
	static bool HasAnimation(UClass* TargetClass, FName AnimationName);
	static FString StripInstSuffix(const FString& AnimationName);

private:
	static UWidgetBlueprint* ResolveWidgetBlueprintFromObject(UObject* Object);
};
