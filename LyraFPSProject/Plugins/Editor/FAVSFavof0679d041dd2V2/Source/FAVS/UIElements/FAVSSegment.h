// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FAVSTab.h"
#include "../Data/FAVSDataAsset.h"
#include "Widgets/SCompoundWidget.h"

class FAVSHeader;
class FAVSElement;

class FAVS_API FAVSSegment : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(FAVSSegment){}
		SLATE_ARGUMENT(int32, SegmentID)
		SLATE_ARGUMENT(FAVSTab*, Parent)
	SLATE_END_ARGS()
	
	/////////////////////////////////////////////// PROPERTIES ////////////////////////////////////////////
public:
	int32 SegmentID;
private:
	TSharedPtr<FAVSHeader> Header;
	int32 RowCreated = 0;
	FAVSTab* Parent = nullptr;
	TArray<TSharedPtr<FAVSAssetData>> AssetItems;
	TSharedPtr<SListView<TSharedPtr<FAVSAssetData>>> ListViewWidget;
	FText SegmentName;
	bool IsHoveredByDrag = false;
	bool IsCollapsed = false;
	int32 CurrentDropIndex = false;
	bool IfInsertBefore = false;
	TSharedPtr<SVerticalBox> VerticalBox;
	TSharedPtr<SBox> SegmentBorder;
	TMap<TSharedPtr<FAVSAssetData>, TWeakPtr<FAVSElement>> ElementWidgets;
	TMap<FSoftObjectPath, TWeakPtr<FAVSElement>> AssetWidgets;


	//////////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////
public:
	void OnAssetAdded(const FAVSAssetData& AssetData, int32 _SegmentID);
	void Construct(const FArguments& InArgs);
	void Destruct();
	void CreateListView();
	void TurnOffBtns();
	void UpdateBtns();
	FAVSTab* GetParent();
	void RecreateListView();
	void UpdateOpenAssetBtnStyle(const FSoftObjectPath& AssetPath, bool IfOpened);
	void RemoveElement(const FSoftObjectPath& Asset);
	void SetListVisibility(bool IfVisible);
	void PreReplaceAsset(const FAssetData& Asset, const FString& OldPath, const FString& NewFolderPath, bool IsFolder);
	void ReplaceAsset(const FAVSAssetData& AssetData, const FString& OldPath);
	FAVSSegmentData* GetSegmentData(); 
private:
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	int32 FindDropIndexFromListView(const FDragDropEvent& DragDropEvent);
	bool CanAddElement(const FAVSAssetData& Asset);
	void CreateHeader();
	void CreateListItems();
	EVisibility GetListViewVisibility() const;
	void ResetPaddings();
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FAVSAssetData> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	
};
