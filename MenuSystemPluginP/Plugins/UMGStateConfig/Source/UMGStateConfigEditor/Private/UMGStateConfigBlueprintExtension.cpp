#include "UMGStateConfigBlueprintExtension.h"

#include "Blueprint/UserWidget.h"
#include "WidgetBlueprint.h"
#include "UMGStateConfigUserWidgetExtension.h"

UUMGStateConfigBlueprintExtension* UUMGStateConfigBlueprintExtension::Request(UWidgetBlueprint* WidgetBlueprint)
{
	return WidgetBlueprint ? UWidgetBlueprintExtension::RequestExtension<UUMGStateConfigBlueprintExtension>(WidgetBlueprint) : nullptr;
}

UUMGStateConfigBlueprintExtension* UUMGStateConfigBlueprintExtension::Find(const UWidgetBlueprint* WidgetBlueprint)
{
	return WidgetBlueprint ? UWidgetBlueprintExtension::GetExtension<UUMGStateConfigBlueprintExtension>(WidgetBlueprint) : nullptr;
}

void UUMGStateConfigBlueprintExtension::HandleCopyTermDefaultsToDefaultObject(UObject* DefaultObject)
{
	UUserWidget* DefaultWidget = Cast<UUserWidget>(DefaultObject);
	if (!DefaultWidget)
	{
		return;
	}

	DefaultWidget->RemoveExtensions(UUMGStateConfigUserWidgetExtension::StaticClass());
	UUMGStateConfigUserWidgetExtension* RuntimeExtension = Cast<UUMGStateConfigUserWidgetExtension>(DefaultWidget->AddExtension(UUMGStateConfigUserWidgetExtension::StaticClass()));
	if (RuntimeExtension)
	{
		RuntimeExtension->RuntimeData = ConfigData;
	}
}
