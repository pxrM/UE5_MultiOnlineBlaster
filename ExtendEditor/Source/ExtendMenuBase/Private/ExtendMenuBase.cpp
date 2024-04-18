// ExtendMenuBase.cpp

#pragma once

#include "ExtendMenuBase.h"
#include "LevelEditor.h"

IMPLEMENT_GAME_MODULE(FExtendMenuBase, ExtendMenuBase)

void FExtendMenuBase::StartupModule()
{
	IModuleInterface::StartupModule();

	ExtendMenuByFExtend();
}

void FExtendMenuBase::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

void FExtendMenuBase::ExtendMenuByFExtend()
{
	/*
	* FExtender ��չ���˵��Ļ��������ǣ�
	* ����һ�� FExtender ����
	* ����Ҫ��չ���������õ��ö����С�
	* ���ö�����ӵ��� FExtensibilityManager ����� Extenders �С�
	*/

	TSharedPtr<FExtender> MainMenuExtender = MakeShared<FExtender>();
	MainMenuExtender->AddMenuExtension(
		"EpicGamesHelp",// ���ڶ�λ��ExtensionHook����
		EExtensionHook::After,// Ӧ��ӵ��ľ���λ��(��Hook֮ǰ����֮��)
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FExtendMenuBase::MakeExistingMainMenuEntry)// ��������ť�Ĵ�����
	);

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	TSharedPtr<FExtensibilityManager> MenuExtensibilityMgr = LevelEditorModule.GetMenuExtensibilityManager();

	MenuExtensibilityMgr->AddExtender(MainMenuExtender);

}

void FExtendMenuBase::MakeExistingMainMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Existing Hook Btn"),// ��ť����
		FText::FromString("This is a existing hook extend"),// ��ť��ʾ
		FSlateIcon(), // ���ǿ���Ϊ��ť���ͼ�꣬������ʱȱʡ
		FUIAction(FExecuteAction::CreateRaw(this, &FExtendMenuBase::ExistingMainMenuEntryAction))// ��ִ�а�ť�Ĵ�����
		);
}

void FExtendMenuBase::ExistingMainMenuEntryAction()
{
	UE_LOG(LogTemp, Warning, TEXT("ExistingMainMenuEntryAction is called."));
}
