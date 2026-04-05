#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/Docking/TabManager.h"

class UUMGStateController;
class UUserWidget;

class FUMGStateConfiguratorEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	void RegisterTabSpawner();
	void UnregisterTabSpawner();

	static TSharedRef<SDockTab> SpawnStateEditorTab(const FSpawnTabArgs& Args);
	
	void ExtendWidgetBlueprintMenu();
	
	static void OpenStateEditorWindow();
	
	static void GetCurrentEditingContext(UUserWidget*& OutWidget, UUMGStateController*& OutController);

private:
	static void OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event);
	static void SyncToBlueprintAsset(UUserWidget* PreviewWidget, const UUMGStateController* PreviewController);
	
	static const FName StateEditorTabName;
	TSharedPtr<FExtender> MenuExtender;
};
