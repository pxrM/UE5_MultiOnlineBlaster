// Copyright Epic Games, Inc. All Rights Reserved.

#include "SwitchLanguageBtn.h"
#include "SwitchLanguageBtnStyle.h"
#include "SwitchLanguageBtnCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"
#include "Interfaces/IMainFrameModule.h"
#include "Editor/InternationalizationSettings/Classes/InternationalizationSettingsModel.h"
#include "Internationalization/Internationalization.h"

static const FName SwitchLanguageBtnTabName("SwitchLanguageBtn");

#define LOCTEXT_NAMESPACE "FSwitchLanguageBtnModule"

void FSwitchLanguageBtnModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FSwitchLanguageBtnStyle::Initialize();
	FSwitchLanguageBtnStyle::ReloadTextures();

	FSwitchLanguageBtnCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSwitchLanguageBtnCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FSwitchLanguageBtnModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSwitchLanguageBtnModule::RegisterMenus));
}

void FSwitchLanguageBtnModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FSwitchLanguageBtnStyle::Shutdown();

	FSwitchLanguageBtnCommands::Unregister();
}

void FSwitchLanguageBtnModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	//FText DialogText = FText::Format(
	//						LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
	//						FText::FromString(TEXT("FSwitchLanguageBtnModule::PluginButtonClicked()")),
	//						FText::FromString(TEXT("SwitchLanguageBtn.cpp"))
	//				   );
	//FMessageDialog::Open(EAppMsgType::Ok, DialogText);

	FString NowLanguage = "";
	UInternationalizationSettingsModel* SettingsModel = NewObject<UInternationalizationSettingsModel>();
	SettingsModel->GetEditorLanguage(NowLanguage);
	if (NowLanguage == "en")
	{
		NowLanguage = "zh-Hans";
	}
	else
	{
		NowLanguage = "en";
	}
	SettingsModel->SetEditorLanguage(NowLanguage);
	FInternationalization& I18N = FInternationalization::Get();
	I18N.SetCurrentLanguage(NowLanguage);
}

void FSwitchLanguageBtnModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FSwitchLanguageBtnCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FSwitchLanguageBtnCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSwitchLanguageBtnModule, SwitchLanguageBtn)