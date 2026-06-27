// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSElementAsset.h"
#include "ContentBrowserModule.h"
#include "FAVSStyle.h"
#include "IContentBrowserSingleton.h"
#include "AssetRegistry/AssetRegistryModule.h"


TSharedPtr<SButton> FAVSElementAsset::AddOpenAssetBtn(TSharedRef<SHorizontalBox> _HorizontalBox)
{
	if(Asset.IsValid)
	{
		TSharedPtr<SButton> _OpenAssetBtn;
		_HorizontalBox->AddSlot().AutoWidth().VAlign(VAlign_Center)
		[
			SAssignNew(_OpenAssetBtn, SButton)
			.ButtonStyle(&FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("OpenAssetBtn"))
			.OnClicked(this, &FAVSElementAsset::OnEyeClicked)
		];
		return _OpenAssetBtn;
	}
	return nullptr;
}

FReply FAVSElementAsset::OnEyeClicked()
{
	AssetIsOpened ?  CloseAsset(): OpenAsset();
	return FReply::Handled();
}

FReply FAVSElementAsset::OpenAsset()
{
	GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Asset.AssetPath);   //todo tu mialem crasha przy levelSequence
	return FReply::Handled();
}

FReply FAVSElementAsset::CloseAsset()
{
	UObject* AssetObject = Asset.AssetPath.ResolveObject();
	if (!AssetObject)
	{
		return FReply::Handled();;
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	AssetEditorSubsystem->CloseAllEditorsForAsset(AssetObject);
		
	return FReply::Handled();
}

void FAVSElementAsset::AddCBBtn(TSharedRef<SHorizontalBox> _HorizontalBox)
{
	if(Asset.IsValid)
	{		
		_HorizontalBox->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(10,0)
		[
			SNew(SButton)
			.ButtonStyle(&FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("OpenInCBBtn"))
			.OnClicked(this, &FAVSElementAsset::ShowInCB)
		];
	}
}

FReply FAVSElementAsset::ShowInCB()
{
	if (!Asset.IsValidAsset())
	{
		return FReply::Handled();
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(Asset.AssetPath);

	GetParent()->GetParent()->CBModule->Get().SyncBrowserToAssets({ AssetData });
	
	return FReply::Handled();
}