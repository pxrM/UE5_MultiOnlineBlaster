#include "AssetCookManagerEditorModule.h"

#include "SAssetCookManagerPanel.h"

#include "Framework/Docking/TabManager.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"

#define LOCTEXT_NAMESPACE "FAssetCookManagerEditorModule"

const FName FAssetCookManagerEditorModule::PanelTabId(TEXT("AssetCookManager"));

void FAssetCookManagerEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		PanelTabId,
		FOnSpawnTab::CreateRaw(this, &FAssetCookManagerEditorModule::SpawnPanelTab))
		.SetDisplayName(LOCTEXT("PanelTabTitle", "Asset Cook Manager"))
		.SetTooltipText(LOCTEXT("PanelTabTooltip", "Manage per-directory cook rules and validate references."))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetToolsCategory())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings"));

	// Defer menu registration until ToolMenus is ready.
	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FAssetCookManagerEditorModule::RegisterMenus));
}

void FAssetCookManagerEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PanelTabId);
	}
}

void FAssetCookManagerEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools");
	if (!Menu)
	{
		return;
	}

	FToolMenuSection& Section = Menu->FindOrAddSection("Miscellaneous");
	Section.AddMenuEntry(
		"OpenAssetCookManager",
		LOCTEXT("OpenAssetCookManager", "Asset Cook Manager"),
		LOCTEXT("OpenAssetCookManagerTooltip", "Open the per-directory cook rule manager."),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings"),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(FAssetCookManagerEditorModule::PanelTabId);
		})));
}

TSharedRef<SDockTab> FAssetCookManagerEditorModule::SpawnPanelTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SAssetCookManagerPanel)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAssetCookManagerEditorModule, AssetCookManagerEditor)
