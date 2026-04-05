// Copyright Epic Games, Inc. All Rights Reserved.

#include "UMGReflector.h"

#include "UMGReflectorCommands.h"
#include "UMGReflectorStyle.h"
#include "UMGReflectorTree.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

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

	RegisterTabSpawners();
}

void FUMGReflectorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UMGReflectorTabName);
	FUMGReflectorCommands::Unregister();
	FUMGReflectorStyle::Shutdown();
}

void FUMGReflectorModule::OnPluginBtnClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(UMGReflectorTabName);
}

TSharedRef<SDockTab> FUMGReflectorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SUMGReflectorTree)
		];
}

void FUMGReflectorModule::RegisterTabSpawners()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		UMGReflectorTabName,
		FOnSpawnTab::CreateRaw(this, &FUMGReflectorModule::OnSpawnPluginTab))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsDebugCategory())
		.SetDisplayName(LOCTEXT("UMGReflectorTabDisplayName", "UMG Reflector"))
		.SetTooltipText(LOCTEXT("UMGReflectorTabTooltip", "Open UMG Reflector window"))
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "WidgetReflector.TabIcon"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUMGReflectorModule, UMGReflector)
