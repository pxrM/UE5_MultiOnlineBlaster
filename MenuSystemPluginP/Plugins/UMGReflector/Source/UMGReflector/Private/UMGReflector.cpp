// Copyright Epic Games, Inc. All Rights Reserved.

#include "UMGReflector.h"

#include "UMGReflectorStyle.h"
#include "UMGReflectorCommands.h"
#include "ToolMenus.h"
#include "UMGReflectorTree.h"

#define LOCTEXT_NAMESPACE "FUMGReflectorModule"

static const FName UMGReflectorTabName(TEXT("UMGReflector"));

void FUMGReflectorModule::StartupModule()
{
	FUMGReflectorStyle::Initialize();
	FUMGReflectorStyle::ReloadTextureResources();
	FUMGReflectorCommands::Register();
	PluginCommands = MakeShareable(new FUICommandList());
	PluginCommands->MapAction(
		FUMGReflectorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUMGReflectorModule::OnPluginBtnClicked)
	);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUMGReflectorModule::RegisterMenus));

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UMGReflectorTabName,
	                                                  FOnSpawnTab::CreateRaw(this, &FUMGReflectorModule::OnSpawnPluginTab))
	                        .SetDisplayName(LOCTEXT("UMGReflectorTabTitle", "UMGReflector"))
	                        .SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FUMGReflectorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FUMGReflectorStyle::Shutdown();
	FUMGReflectorCommands::Unregister();
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UMGReflectorTabName);
}

void FUMGReflectorModule::OnPluginBtnClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UMGReflectorTabName);
}

TSharedRef<SDockTab> FUMGReflectorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	TSharedRef<SDockTab> DockTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SUMGReflectorTree)
		];

	return DockTab;
}

void FUMGReflectorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection(UMGReflectorTabName);
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUMGReflectorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUMGReflectorModule, UMGReflector)
