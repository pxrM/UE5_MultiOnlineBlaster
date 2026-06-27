// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


struct FAVSDataAsset;
class FToolBarBuilder;
class FMenuBuilder;

class FFAVSModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	void RegisterLocalization();
	virtual void ShutdownModule() override;
	
	void OpenFAVS();
		
private:
	void RegisterMenus();
	void ExtendBlueprintEditorToolbar(const TSharedRef<FUICommandList> CommandList, const TSharedRef<FAssetEditorToolkit> EditorToolkit);
	void OnCultureChanged();
		
	FString CurrentCultureName; 
	FDelegateHandle CultureChangedHandle;
	
	FAVSDataAsset* DataAsset = nullptr;
	TSharedRef<class SDockTab> CreateFAVSTab(const class FSpawnTabArgs& SpawnTabArgs);
	TSharedPtr<class FUICommandList> PluginCommands;
};
