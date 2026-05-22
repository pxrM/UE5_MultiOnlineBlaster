#include "WidgetAnimTimelineEditor.h"

#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "WidgetAnimTimelineEntryCustomization.h"
#include "WidgetAnimTimelinePhaseCustomization.h"

#define LOCTEXT_NAMESPACE "FWidgetAnimTimelineEditorModule"

void FWidgetAnimTimelineEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("WidgetAnimTimelineEntry", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWidgetAnimTimelineEntryCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("WidgetAnimTimelinePhase", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWidgetAnimTimelinePhaseCustomization::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();
}

void FWidgetAnimTimelineEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("WidgetAnimTimelineEntry");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("WidgetAnimTimelinePhase");
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWidgetAnimTimelineEditorModule, WidgetAnimTimelineEditor)
