// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSElementSetting.h"
#include "FAVSStyle.h"
#include "ISettingsModule.h"


void FAVSElementSetting::AddRemoveBtn(TSharedRef<SHorizontalBox> _HorizontalBox)
{
}

TSharedPtr<SButton> FAVSElementSetting::AddOpenAssetBtn(TSharedRef<SHorizontalBox> _HorizontalBox)
{
	TSharedPtr<SButton> _OpenAssetBtn;
	_HorizontalBox->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0,0,7,0)
	[
		 SAssignNew(_OpenAssetBtn, SButton)
		.ButtonStyle(&FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("OpenAssetBtn"))
		.OnClicked(this, &FAVSElementSetting::OnEyeClicked)
	];
	return _OpenAssetBtn;
}

FReply FAVSElementSetting::OnEyeClicked()
{
	OpenAsset();
	return FReply::Handled();
}

FReply FAVSElementSetting::OpenAsset()
{
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
	ESettingsWindow WindowType = Asset.SettingType;
	
	if (!SettingsModule)
	{
		return FReply::Handled();
	}
	
	if (WindowType == ESettingsWindow::EditorPreferences)
	{
		SettingsModule->ShowViewer("Editor", "General", "Appearance");
	}
	else if (WindowType == ESettingsWindow::ProjectSettings)
	{
		SettingsModule->ShowViewer("Project", "Engine", "Animation");
	}
	return FReply::Handled();
}

FReply FAVSElementSetting::CloseAsset()
{
	return FReply::Handled();
}
