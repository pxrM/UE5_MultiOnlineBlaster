// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "FAVSStructs.generated.h"

UENUM()
enum class ESettingsWindow : uint8
{
	None,
	EditorPreferences,
	ProjectSettings
};

UENUM(BlueprintType)
enum class EAssetType : uint8
{
	//BLUEPRINT
	Blueprint,
	BlueprintFunctionLibrary,
	BlueprintInterface,
			
	//MISCELLANEOUS
	DataAsset,
				
	//TEXTURE
	Texture,
		
	//BASIC ASSETS
	WidgetBlueprint,
	StaticMesh,
	Level,
	
	//INNE
	Folder,
	Settings,
	Unknown
};

USTRUCT()
struct FAVSAssetData
{
	GENERATED_BODY()
		
	UPROPERTY() FString DisplayName;
	UPROPERTY() FSoftObjectPath AssetPath;
	UPROPERTY() EAssetType AssetType;
	const UClass* AssetClass;
	UPROPERTY() FColor IconColor = FColor::White;
	UPROPERTY() bool IsValid = true;
	UPROPERTY() ESettingsWindow SettingType = ESettingsWindow::None;
	
	FAVSAssetData() : DisplayName(FString()), AssetType(EAssetType::Unknown), AssetClass(nullptr) {}   
	FAVSAssetData(const FString& DisplayName, const FSoftObjectPath& AssetPath,  EAssetType AssetType, const UClass* AssetClass, const FColor& IconColor) : DisplayName(DisplayName), AssetPath(AssetPath), AssetType(AssetType), AssetClass(AssetClass), IconColor(IconColor){}

	void UpdateData(const FAVSAssetData& AssetData)
	{
		DisplayName = AssetData.DisplayName;
		AssetPath = AssetData.AssetPath;
	}
		
	bool operator==(const FAVSAssetData& Other) const
	{
		return AssetPath == Other.AssetPath;
	}
	
	bool Matches(UObject* InAsset) const
	{
		return AssetPath == FSoftObjectPath(InAsset);
	}
	
	bool IsValidAsset() const
	{
		if(AssetType == EAssetType::Settings) return true;
		if(AssetType == EAssetType::Folder) return IsValidFolder();
		return AssetPath.IsValid();
	}
	bool IsValidFolder() const { return AssetType == EAssetType::Folder && !AssetPath.ToString().IsEmpty(); }  //todo to nie wiem
};

struct FPendingElement
{
	int32 SegmentID = -1;
	FAVSAssetData Element;

	FPendingElement() = default;
	FPendingElement(int32 InSegmentID, const FAVSAssetData& InElement): SegmentID(InSegmentID), Element(InElement){}

	bool IsSet() const
	{
		if(Element.AssetType == EAssetType::Settings)
		{
			return true; 
		}
		return Element.AssetPath.IsValid();
	}
	
	void Reset()
	{
		SegmentID = -1;
		Element ={};
	}
};

struct FPendingPathRemoval
{
	FString Path;
	double RemovalTime;
};

struct FFolderChange
{
	FString OldFolder;
	FString NewFolder;
	double ChangeTime;
	bool IfSingleFolderChange;  

	FFolderChange(): OldFolder(FString()), NewFolder(FString()), ChangeTime(0), IfSingleFolderChange(false){}
};

USTRUCT()
struct FAVSSegmentData
{
	GENERATED_BODY()

	UPROPERTY()	FText SegmentName;
	UPROPERTY() int32 SegmentID;
	UPROPERTY()	TArray<FAVSAssetData> Assets;
	UPROPERTY() bool IsCollapsed = true;
	UPROPERTY() bool IsSettingSegment = false;
	UPROPERTY() FLinearColor HeaderColor = FLinearColor::Gray;
	
	//////////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////
	FAVSSegmentData() : SegmentName(FText::GetEmpty()), SegmentID(-1){}

	FAVSSegmentData(const FText& InName, int32 InSegmentID) : SegmentName(InName), SegmentID(InSegmentID){}

	
	bool IsEmpty() const
	{
		return Assets.Num() == 0;
	}
	
	void RemoveElement(const FAVSAssetData& Element)
	{
		Assets.Remove(Element);
	}
	
	void AddElement(const FAVSAssetData& AssetData, int32 DropIndex)
	{
		
		if (DropIndex >= 0 && DropIndex <= Assets.Num())
		{
			Assets.Insert(AssetData, DropIndex);
		}
		else
		{
			Assets.Add(AssetData);
		}
	}
	
	void ReplaceElement(const FAVSAssetData& AssetData, const FString& OldPath)
	{
		for (FAVSAssetData& Element : Assets)
		{
			if(Element.AssetPath == OldPath)
			{
				Element.UpdateData(AssetData);
				return;
			}
		}
	}
	
	bool IfSegmentContainsAsset(UObject* Asset)
	{
		for (const FAVSAssetData& Element : Assets)
		{
			if(Element.Matches(Asset))
			{
				return true;
			}
		}
		return false;
	}
	
	bool IfSegmentContainsAsset(const FString& Path)
	{
		for (const FAVSAssetData& Element : Assets)
		{
			if(Element.AssetPath == Path)
			{
				return true;
			}
		}
		return false;
	}
}; 
