#include "UMGStateConfiguratorEditor.h"

#include "UMGStateControllerDetails.h"

#define LOCTEXT_NAMESPACE "FUMGStateConfiguratorEditorModule"

void FUMGStateConfiguratorEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"UIPropertyOverride", 
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUIPropertyOverrideCustomization::MakeInstance)
	);
}

void FUMGStateConfiguratorEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

IMPLEMENT_MODULE(FUMGStateConfiguratorEditorModule, UMGStateConfigurator)