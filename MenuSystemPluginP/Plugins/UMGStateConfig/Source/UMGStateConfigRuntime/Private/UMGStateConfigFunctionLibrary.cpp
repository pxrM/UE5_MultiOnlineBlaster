#include "UMGStateConfigFunctionLibrary.h"

#include "Blueprint/UserWidget.h"
#include "UMGStateConfigData.h"
#include "UMGStateConfigUserWidgetExtension.h"

bool UUMGStateConfigFunctionLibrary::ApplyUIState(UUserWidget* TargetWidget, FName StateGroupName, FName StateName)
{
	UUMGStateConfigUserWidgetExtension* Extension = FindOrCreateStateConfigExtension(TargetWidget);
	if (!Extension)
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("ApplyUIState failed: state config extension not found."));
		return false;
	}

	return Extension->ApplyUIState(StateGroupName, StateName);
}

UUMGStateConfigUserWidgetExtension* UUMGStateConfigFunctionLibrary::FindOrCreateStateConfigExtension(UUserWidget* TargetWidget)
{
	if (!TargetWidget)
	{
		return nullptr;
	}

	if (UUMGStateConfigUserWidgetExtension* ExistingExtension = Cast<UUMGStateConfigUserWidgetExtension>(TargetWidget->GetExtension(UUMGStateConfigUserWidgetExtension::StaticClass())))
	{
		return ExistingExtension;
	}

	const UUserWidget* DefaultWidget = TargetWidget->GetClass() ? Cast<UUserWidget>(TargetWidget->GetClass()->GetDefaultObject()) : nullptr;
	const UUMGStateConfigUserWidgetExtension* DefaultExtension = DefaultWidget
		? Cast<UUMGStateConfigUserWidgetExtension>(DefaultWidget->GetExtension(UUMGStateConfigUserWidgetExtension::StaticClass()))
		: nullptr;

	if (!DefaultExtension)
	{
		return nullptr;
	}

	UUMGStateConfigUserWidgetExtension* NewExtension = Cast<UUMGStateConfigUserWidgetExtension>(TargetWidget->AddExtension(UUMGStateConfigUserWidgetExtension::StaticClass()));
	if (NewExtension)
	{
		NewExtension->RuntimeData = DefaultExtension->RuntimeData;
	}
	return NewExtension;
}
