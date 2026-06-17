#include "AssetCookManagerEditorModule.h"

#include "AssetCookPakExporter.h"
#include "SAssetCookManagerPanel.h"

#include "AssetRegistry/AssetData.h"
#include "ContentBrowserMenuContexts.h"
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

	RegisterContentBrowserMenus();
}

void FAssetCookManagerEditorModule::RegisterContentBrowserMenus()
{
	if (UToolMenu* AssetMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.AssetContextMenu"))
	{
		AssetMenu->AddDynamicSection(
			"AssetCookManagerPak",
			FNewToolMenuDelegate::CreateLambda([](UToolMenu* InMenu)
			{
				const UContentBrowserAssetContextMenuContext* Context =
					UContentBrowserAssetContextMenuContext::FindContextWithAssets(*InMenu);
				if (!Context)
				{
					return;
				}

				TArray<FName> PackageNames;
				for (const FAssetData& Asset : Context->SelectedAssets)
				{
					PackageNames.AddUnique(Asset.PackageName);
				}

				FToolMenuSection& Section = InMenu->FindOrAddSection(
					"AssetCookManager",
					LOCTEXT("AssetCookManagerSection", "Asset Cook Manager"));

				Section.AddMenuEntry(
					"AssetCookManagerExportSelectedAssetsPak",
					LOCTEXT("ExportSelectedAssetsPak", "Export Debug Pak"),
					LOCTEXT("ExportSelectedAssetsPakTip", "Create a pak from the selected assets' already-cooked files."),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Package"),
					FUIAction(FExecuteAction::CreateLambda([PackageNames]()
					{
						FAssetCookPakExporter::ExportPackagesInteractive(PackageNames, TEXT("Debug_SelectedAssets"));
					})));
			}));
	}

	if (UToolMenu* FolderMenu = UToolMenus::Get()->ExtendMenu("ContentBrowser.FolderContextMenu"))
	{
		FolderMenu->AddDynamicSection(
			"AssetCookManagerPak",
			FNewToolMenuDelegate::CreateLambda([](UToolMenu* InMenu)
			{
				const UContentBrowserFolderContext* Context = InMenu->FindContext<UContentBrowserFolderContext>();
				if (!Context || Context->SelectedPackagePaths.IsEmpty())
				{
					return;
				}

				const TArray<FString> PackagePaths = Context->SelectedPackagePaths;

				FToolMenuSection& Section = InMenu->FindOrAddSection(
					"AssetCookManager",
					LOCTEXT("AssetCookManagerFolderSection", "Asset Cook Manager"));

				Section.AddMenuEntry(
					"AssetCookManagerExportSelectedFoldersPak",
					LOCTEXT("ExportSelectedFoldersPak", "Export Debug Pak"),
					LOCTEXT("ExportSelectedFoldersPakTip", "Create a pak from already-cooked assets under the selected folder(s)."),
					FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Package"),
					FUIAction(FExecuteAction::CreateLambda([PackagePaths]()
					{
						FAssetCookPakExporter::ExportDirectoriesInteractive(PackagePaths, TEXT("Debug_SelectedFolders"));
					})));
			}));
	}
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
