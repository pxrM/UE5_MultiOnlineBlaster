// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVS.h"
#include "FAVSStyle.h"
#include "FAVSCommands.h"
#include "FAVSHelper.h"
#include "FAVSSettings.h"
#include "ISequencerModule.h"
#include "ISettingsModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "ToolMenus.h"
#include "FAVS/Data/FAVSDataAsset.h"
#include "FAVS/UIElements/FAVSTab.h"
#include "Interfaces/IPluginManager.h"
#include "Internationalization/Culture.h"

static const FName FAVS_TAB = "FAVS";
#define LOCTEXT_NAMESPACE "FFAVSModule"

void FFAVSModule::StartupModule()
{
	FAVSStyle::Initialize();
	FAVSStyle::ReloadTextures();

	FFAVSCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FFAVSCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FFAVSModule::OpenFAVS),
		FCanExecuteAction());
	
	DataAsset = &FAVSDataAsset::GetOrCreateData();
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FFAVSModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FAVS_TAB, FOnSpawnTab::CreateRaw(this, &FFAVSModule::CreateFAVSTab))
		.SetDisplayName(LOCTEXT("FFAVSTabTitle", "FAVS"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	CurrentCultureName = FInternationalization::Get().GetCurrentCulture()->GetName();
	RegisterLocalization();
	
	CultureChangedHandle = FInternationalization::Get().OnCultureChanged().AddRaw(this, &FFAVSModule::OnCultureChanged);
	
	ISettingsModule& SettingsModule = FModuleManager::LoadModuleChecked<ISettingsModule>("Settings");
	SettingsModule.RegisterSettings("Project", "Plugins", "FAVS",
		LOCTEXT("FAVSEditorSettingsName", "FAVS"),
		LOCTEXT("FAVSEditorSettingsDescription", "Configure settings for the FAVS plugin."),
		GetMutableDefault<UFAVSSettings>()
	);
}

void FFAVSModule::RegisterLocalization()
{
	TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("FAVS"));
	if (!Plugin.IsValid())
	{
		return;
	}
    
	const FString PluginBaseDir = Plugin->GetBaseDir();
	const FString LocalizationTargetPath = FPaths::Combine(PluginBaseDir, TEXT("Content/Localization/FAVS"));
    
	if (!FPaths::DirectoryExists(LocalizationTargetPath))
	{
		return;
	}
    
	TArray<FString> PathsToMount;
	PathsToMount.Add(LocalizationTargetPath);
	FTextLocalizationManager::Get().HandleLocalizationTargetsMounted(PathsToMount);
}


void FFAVSModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FAVSStyle::Shutdown();

	FFAVSCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FAVS_TAB);

	UToolMenus::UnregisterOwner(this);

	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	if (SettingsModule)
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "FAVS");
	}
	if (CultureChangedHandle.IsValid())
	{
		FInternationalization::Get().OnCultureChanged().Remove(CultureChangedHandle);
	}
}

TSharedRef<SDockTab> FFAVSModule::CreateFAVSTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(ETabRole::NomadTab)
	[
		SNew(FAVSTab)
		.TestString(FString("FAVS HEADER"))
		.FAVSData(DataAsset)
	];
}

void FFAVSModule::OpenFAVS()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FAVS_TAB);
}

void FFAVSModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FFAVSCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}
	FAVSHelper::AddToolbarButton_Level(DataAsset);

	FAVSHelper::AddToolbarButton_Asset(DataAsset, FName("AssetEditor.DefaultToolBar"));
		
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");

	TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();
	FAVSDataAsset* LocalDataAsset = DataAsset;
	
	ToolbarExtender->AddToolBarExtension(
		"BaseCommands",
		EExtensionHook::After,
		nullptr,
		FToolBarExtensionDelegate::CreateLambda([LocalDataAsset](FToolBarBuilder& ToolbarBuilder)
		{
			FAVSHelper::AddToolbarButton_Sequencer(ToolbarBuilder, LocalDataAsset);
		})
	);

	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	
}

void FFAVSModule::ExtendBlueprintEditorToolbar(const TSharedRef<FUICommandList> CommandList, const TSharedRef<FAssetEditorToolkit> EditorToolkit)
{
	FToolMenuOwnerScoped OwnerScoped(this);
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FFAVSCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}
}

void FFAVSModule::OnCultureChanged()
{
	FString NewCultureName = FInternationalization::Get().GetCurrentCulture()->GetName();
    
	if (NewCultureName == CurrentCultureName)
	{
		return;
	}
    
	CurrentCultureName = NewCultureName;
    RegisterLocalization();
	
}



#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFAVSModule, FAVS)