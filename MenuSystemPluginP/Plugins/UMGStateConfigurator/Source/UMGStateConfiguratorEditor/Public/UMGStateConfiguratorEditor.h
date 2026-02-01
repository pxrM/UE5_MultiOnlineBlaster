#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UUMGStateController;

class FUMGStateConfiguratorEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	static void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event);
	static void SyncToBlueprintAsset(UUserWidget* PreviewWidget, const UUMGStateController* PreviewController);
};
