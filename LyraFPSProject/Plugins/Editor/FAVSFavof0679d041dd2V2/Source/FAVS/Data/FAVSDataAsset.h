// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FAVSStructs.h"
#include "FAVSDataAsset.generated.h"


USTRUCT()
struct FAVS_API FAVSDataAsset 
{
	GENERATED_BODY()

	/////////////////////////////////////////////// PROPERTIES ////////////////////////////////////////////
public:
	UPROPERTY()	bool IsArchivedVisible = false;
	UPROPERTY()	TMap<int32, FAVSSegmentData> Segments;
	UPROPERTY()	TArray<int32> SegmentsOrder;
	UPROPERTY()	TArray<int32> ArchivedSegments;
	UPROPERTY() bool HasSettingsSegment = false;
	
	//////////////////////////////////////////////// FUNCTIONS ////////////////////////////////////////////
public:
	static FAVSDataAsset& GetOrCreateData();
	int32 GetFirstAvailableSegmentID();
	void RemoveSegment(int32 SegmentID);
	void RemovePendingElement(const int32 SegmentID, const FAVSAssetData& Element);
	void RemoveFromOrder(int32 SegmentID);
	void RemoveFromArchived(int32 SegmentID);
	void ChangeSegmentOrder(int32 DraggedSegmentID, bool IfBeforeSegment, int32 DroppedOverSegmentID);
	bool AreSegmentsNeighbors(const int32 DraggedSegmentID, const bool IfDraggingOverTop, const int32 DroppedOverSegmentID, const bool ArchivedAreVisible);
	TArray<int32> GetSegmentIDContainingAsset(UObject* Asset);
	TArray<int32> GetSegmentIDContainingAsset(const FString& Path);
	bool ArchiveSegment(int32 SegmentID);
	bool UnarchiveSegment(int32 SegmentID);
	bool IsArchived(int32 SegmentID);
	bool LoadFromSavedFile();
	void TestBeforeSaveAsset();
	void SaveAsset();
private:
	int32 GetSegmentOrderIndex(int32 SegmentID);
	int32 GetNeighborSegments(int32 MySegment, bool IfDraggingOverTop);
};
