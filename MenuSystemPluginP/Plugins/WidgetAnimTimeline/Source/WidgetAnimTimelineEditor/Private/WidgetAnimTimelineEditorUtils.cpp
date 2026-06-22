#include "WidgetAnimTimelineEditorUtils.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "PropertyHandle.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprint.h"

UWidgetBlueprint* FWidgetAnimTimelineEditorUtils::ResolveWidgetBlueprint(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	if (!PropertyHandle.IsValid())
	{
		return nullptr;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);
	for (UObject* OuterObject : OuterObjects)
	{
		if (UWidgetBlueprint* WidgetBlueprint = ResolveWidgetBlueprintFromObject(OuterObject))
		{
			return WidgetBlueprint;
		}
	}

	return nullptr;
}

UClass* FWidgetAnimTimelineEditorUtils::ResolveOwnerWidgetClass(TSharedPtr<IPropertyHandle> PropertyHandle, UWidgetBlueprint* WidgetBlueprint)
{
	if (WidgetBlueprint != nullptr)
	{
		return WidgetBlueprint->GeneratedClass;
	}

	if (!PropertyHandle.IsValid())
	{
		return nullptr;
	}

	TArray<UObject*> OuterObjects;
	PropertyHandle->GetOuterObjects(OuterObjects);
	for (UObject* OuterObject : OuterObjects)
	{
		for (UObject* Object = OuterObject; Object != nullptr; Object = Object->GetOuter())
		{
			if (UClass* Class = Cast<UClass>(Object))
			{
				return Class;
			}

			UClass* ObjectClass = Object->GetClass();
			if (ObjectClass != nullptr && ObjectClass->ClassGeneratedBy != nullptr)
			{
				return ObjectClass;
			}
		}
	}

	return nullptr;
}

UClass* FWidgetAnimTimelineEditorUtils::ResolveTargetWidgetClass(UWidgetBlueprint* WidgetBlueprint, UClass* OwnerClass, FName TargetWidgetName)
{
	if (TargetWidgetName.IsNone())
	{
		return OwnerClass;
	}

	if (WidgetBlueprint != nullptr && WidgetBlueprint->WidgetTree != nullptr)
	{
		UWidget* TargetWidget = WidgetBlueprint->WidgetTree->FindWidget(TargetWidgetName);
		if (TargetWidget != nullptr && TargetWidget->GetClass()->IsChildOf(UUserWidget::StaticClass()))
		{
			return TargetWidget->GetClass();
		}
	}

	if (OwnerClass != nullptr)
	{
		for (TFieldIterator<FObjectProperty> It(OwnerClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			if (It->GetFName() == TargetWidgetName && It->PropertyClass != nullptr && It->PropertyClass->IsChildOf(UUserWidget::StaticClass()))
			{
				return It->PropertyClass;
			}
		}
	}

	return nullptr;
}

UWidgetAnimation* FWidgetAnimTimelineEditorUtils::ResolveAnimation(UUserWidget* TargetWidget, FName AnimationName)
{
	if (TargetWidget == nullptr || AnimationName.IsNone())
	{
		return nullptr;
	}

	for (TFieldIterator<FObjectProperty> It(TargetWidget->GetClass(), EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->PropertyClass != UWidgetAnimation::StaticClass())
		{
			continue;
		}

		if (StripInstSuffix(It->GetName()) == AnimationName.ToString())
		{
			return Cast<UWidgetAnimation>(It->GetObjectPropertyValue_InContainer(TargetWidget));
		}
	}

	return nullptr;
}

UWidgetAnimation* FWidgetAnimTimelineEditorUtils::ResolveAnimationFromClassDefaultObject(UClass* TargetClass, FName AnimationName)
{
	if (TargetClass == nullptr || AnimationName.IsNone())
	{
		return nullptr;
	}

	UObject* DefaultObject = TargetClass->GetDefaultObject();
	if (DefaultObject == nullptr)
	{
		return nullptr;
	}

	for (TFieldIterator<FObjectProperty> It(TargetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->PropertyClass != UWidgetAnimation::StaticClass())
		{
			continue;
		}

		if (StripInstSuffix(It->GetName()) == AnimationName.ToString())
		{
			return Cast<UWidgetAnimation>(It->GetObjectPropertyValue_InContainer(DefaultObject));
		}
	}

	return nullptr;
}

void FWidgetAnimTimelineEditorUtils::CollectAnimationNames(UClass* TargetClass, TArray<FName>& OutAnimationNames)
{
	if (TargetClass == nullptr)
	{
		return;
	}

	for (TFieldIterator<FObjectProperty> It(TargetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->PropertyClass == UWidgetAnimation::StaticClass())
		{
			OutAnimationNames.Add(FName(*StripInstSuffix(It->GetName())));
		}
	}
}

bool FWidgetAnimTimelineEditorUtils::HasAnimation(UClass* TargetClass, FName AnimationName)
{
	return ResolveAnimationFromClassDefaultObject(TargetClass, AnimationName) != nullptr;
}

FString FWidgetAnimTimelineEditorUtils::StripInstSuffix(const FString& AnimationName)
{
	const FString InstSuffix = TEXT("_INST");
	return AnimationName.EndsWith(InstSuffix) ? AnimationName.LeftChop(InstSuffix.Len()) : AnimationName;
}

UWidgetBlueprint* FWidgetAnimTimelineEditorUtils::ResolveWidgetBlueprintFromObject(UObject* Object)
{
	for (; Object != nullptr; Object = Object->GetOuter())
	{
		if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Object))
		{
			return WidgetBlueprint;
		}

		if (UClass* Class = Cast<UClass>(Object))
		{
			if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Class->ClassGeneratedBy))
			{
				return WidgetBlueprint;
			}
		}

		if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Object->GetClass()->ClassGeneratedBy))
		{
			return WidgetBlueprint;
		}
	}

	return nullptr;
}
