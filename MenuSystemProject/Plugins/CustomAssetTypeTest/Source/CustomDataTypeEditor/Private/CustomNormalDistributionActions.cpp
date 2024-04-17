// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNormalDistributionActions.h"
#include "CustomNormalDistribution.h"
#include "Framework/Commands/UIAction.h"
#include "Textures/SlateIcon.h"
#include "CustomNDEditorToolkit.h"
#include "EditorReimportHandler.h"


FCustomNormalDistributionActions::FCustomNormalDistributionActions(EAssetTypeCategories::Type InAssetCategory)
	:AssetCategory(InAssetCategory)
{
}

FCustomNormalDistributionActions::~FCustomNormalDistributionActions()
{
	
}

UClass* FCustomNormalDistributionActions::GetSupportedClass() const
{
	return UCustomNormalDistribution::StaticClass();
}

FText FCustomNormalDistributionActions::GetName() const
{
	return INVTEXT("Custom Normal Distribution");
}

FColor FCustomNormalDistributionActions::GetTypeColor() const
{
	return FColor::Orange;
}

uint32 FCustomNormalDistributionActions::GetCategories()
{
	return AssetCategory;
}

void FCustomNormalDistributionActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	FAssetTypeActions_Base::GetActions(InObjects, MenuBuilder);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Custom Action TestBtn"),
		FText::FromString("This is a custom action"),
		FSlateIcon(),
		FUIAction()
	);
	MenuBuilder.AddMenuEntry(
		FText::FromString("ReimportBtn"),
		FText::FromString("Reimport"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([=]()
			{
				FCustomNormalDistributionActions::ExecuteReimport(InObjects);
			}))
	);
}

bool FCustomNormalDistributionActions::HasActions(const TArray<UObject*>& InObjects) const
{
	// HasActions() 必须返回 true 以使 GetActions() 有效
	return true;
}

void FCustomNormalDistributionActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	MakeShared<FCustomNDEditorToolkit>()->InitEditor(InObjects);
}

void FCustomNormalDistributionActions::ExecuteReimport(const TArray<UObject*>& InObjects)
{
	for(UObject* Object: InObjects)
	{
		if (Object)
		{
			FReimportManager::Instance()->Reimport(Object, /*bAskForNewFileIfMissing=*/true);
		}
	}
}
