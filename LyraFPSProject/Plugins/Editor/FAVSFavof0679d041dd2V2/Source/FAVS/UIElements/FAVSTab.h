// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FAVSStructs.h"
#include "FAVS/Data/FAVSDataAsset.h"
#include "Widgets/SCompoundWidget.h"

class FContentBrowserModule;
class FAVSSegment;

class FAVS_API FAVSTab : public SCompoundWidget
{
	
	SLATE_BEGIN_ARGS(FAVSTab){}
		SLATE_ARGUMENT(FString, TestString)
		SLATE_ARGUMENT(FAVSDataAsset*, FAVSData)
	SLATE_END_ARGS()
	

	/////////////////////////////////////////////// PROPERTIES ////////////////////////////////////////////
	FContentBrowserModule* CBModule = nullptr;
private:
	FAVSDataAsset* DataAsset = nullptr;
	ECheckBoxState ArchiveToggleState = ECheckBoxState::Unchecked;
	FPendingElement PendingElement;
	TSharedPtr<SScrollBox> ScrollBoxSegments;
	TMap<int32, TSharedPtr<FAVSSegment>>SegmentWidgets;
	FFolderChange LastFolderChange;
	FTimerHandle CheckTabsTimerHandle = {};
	//////////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////
private:
	void HandleAssetOpened(UObject* Object, IAssetEditorInstance* AssetEditorInstance);
	void HandleAssetClosed(UObject* Object, IAssetEditorInstance* AssetEditorInstance);
	void HandleAssetRenamed(const FAssetData&, const FString&);
	void HandleOnMapOpened(const FString& Filename, bool bAsTemplate);
	void HandleAssetDeleted(const TArray<UObject*>& DeletedAssets);
	void HandlePathsAdded(TConstArrayView<FStringView> Paths);
	void HandlePathsRemoved(TConstArrayView<FStringView> Paths);
	FString GetNewFolderName(const FString& OldFolder) const;
	void RenameFolder(const FString& OldPath, const FString& NewPath);
	void RemoveFolder(const FString& Path);
	
	void UpdateElementWidget(UObject* Object, bool IfOpened);
	TArray<TSharedPtr<FAVSSegment>> GetSegmentsForAsset(UObject* Asset);
	TArray<TSharedPtr<FAVSSegment>> GetSegmentsForAsset(const FString& Path);
public:
	void Construct(const FArguments& InArgs);
	void SetPendingElement(const FPendingElement& Element);
	void RemovePendingFromSegment() const;
	void ResetPendingElement();
	void SaveAsset();
	FAVSSegmentData* GetSegmentData(int32 SegmentID);
	void RemoveSegment(int32 SegmentID);
	void ChangeSegmentOrder(int32 DraggedSegmentID, bool IfBeforeSegment, int32 DroppedOverSegmentID);
	bool AreSegmentsNeighbors(int32 DraggedSegmentID, bool IfBeforeSegment, int32 DroppedOverSegmentID);
	void RebuildSegments();
	void ArchiveSegment(int32 SegmentID);
	void UnarchiveSegment(int32 SegmentID);
	bool IsArchived(int32 SegmentID);
private:
	void OnToggleChanged(ECheckBoxState NewState);
	void RemoveArchivedSegmentsFromList();
	void RemoveSegmentFromList(int32 SegmentID);
	void RefreshSegmentWidget(int32 SegmentID);
	TSharedRef<SScrollBox> CreateSegmentsFromData();
	void OnShowSettings();
	int32 AddSettingsSegmentData();
	FReply AddSettingSegment();
	void RemoveSettingsSegment();
	bool IsSegmentArchived(int32 SegmentID) const;
	FReply AddSegment();
	
};
