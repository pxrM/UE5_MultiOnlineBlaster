// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FAVSStructs.h"

struct FAVSDataAsset;

class FAVS_API FAVSHelper
{
	/////////////////////////////////////////////// PROPERTIES ////////////////////////////////////////////
	static TMap<const UClass*, FColor> CachedColors;
	static constexpr int32 ColorOpacity = 220; //255 - 100%
	
	//////////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////
public:
	static FColor GetIconColor(const UClass* AssetClass);
	static void AddToolbarButton_Asset(FAVSDataAsset* DataAsset, const FName& Toolbar);
	static void AddToolbarButton_Level(FAVSDataAsset* DataAsset);
	static void AddToolbarButton_Sequencer(FToolBarBuilder& ToolbarBuilder, FAVSDataAsset* DataAsset);
	static EAssetType GetAssetTypeFromAssetData(const FAssetData& AssetData);
	static bool ExtractMovedFolder(const FString& OldAssetFolder, const FString& NewAssetFolder, FString& OldFolder, FString& NewFolder);
private:
	static UObject* GetEditedAsset();
	static EAssetType GetAssetTypeFromAssetData(const UObject* Asset);
};
