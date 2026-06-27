// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSElementFolder.h"
#include "ContentBrowserModule.h"
#include "FAVSStyle.h"
#include "IContentBrowserSingleton.h"

TSharedPtr<SButton> FAVSElementFolder::AddOpenAssetBtn(TSharedRef<SHorizontalBox> _HorizontalBox){return nullptr;}

void FAVSElementFolder::AddCBBtn(TSharedRef<SHorizontalBox> _HorizontalBox)
{
	if(Asset.IsValid)
	{		
		_HorizontalBox->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(10,0)
		[
			SNew(SButton)
			.ButtonStyle(&FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("OpenInCBBtn"))
			.OnClicked(this, &FAVSElementFolder::ShowInCB)
		];
	}
}

FReply FAVSElementFolder::ShowInCB()
{
	if (Asset.IsValidFolder())
	{
		GetParent()->GetParent()->CBModule->Get().SyncBrowserToFolders({Asset.AssetPath.ToString()});
	}
	
	return FReply::Handled();
}

FReply FAVSElementFolder::OnEyeClicked()
{
	ShowInCB();
	return FReply::Handled();
}

FReply FAVSElementFolder::OpenAsset()
{
	ShowInCB();
	return FReply::Handled();
}

FReply FAVSElementFolder::CloseAsset()
{
	return FReply::Handled();
}

