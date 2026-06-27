// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSDataAsset.h"
#include "JsonObjectConverter.h"

int32 FAVSDataAsset::GetFirstAvailableSegmentID()
{
	TSet<int32> UsedIDs;
	TArray<int32> Keys;
	Segments.GetKeys(Keys);
	UsedIDs.Append(Keys);

	int32 FreeID = 0;
	while (UsedIDs.Contains(FreeID))
	{
		++FreeID;
	}

	return FreeID;
}


void FAVSDataAsset::RemoveSegment(const int32 SegmentID)
{
	if(Segments.Contains(SegmentID))
	{
		Segments.Remove(SegmentID);
	}
}

void FAVSDataAsset::RemovePendingElement(const int32 SegmentID, const FAVSAssetData& Element)
{
	if(Segments.Contains(SegmentID))
	{
		Segments[SegmentID].RemoveElement(Element);
	}
}

void FAVSDataAsset::RemoveFromOrder(const int32 SegmentID)
{
	for (int32 i = 0; i < SegmentsOrder.Num(); ++i)
	{
		if (SegmentsOrder[i] == SegmentID)
		{
			SegmentsOrder.RemoveAt(i);
			return;
		}
	}
}

void FAVSDataAsset::RemoveFromArchived(const int32 SegmentID)
{
	for (int32 i = 0; i < ArchivedSegments.Num(); ++i)
	{
		if (ArchivedSegments[i] == SegmentID)
		{
			ArchivedSegments.RemoveAt(i);
			return;
		}
	}
}

int32 FAVSDataAsset::GetSegmentOrderIndex(const int32 SegmentID)
{
	if(!SegmentsOrder.Contains(SegmentID))
	{
		return -1;
	}
	
	for (int32 i = 0; i < SegmentsOrder.Num(); ++i)
	{
		if (SegmentsOrder[i] == SegmentID)
		{
			return i;
		}
	}

	return -1;
}

void FAVSDataAsset::ChangeSegmentOrder(int32 DraggedSegmentID, bool IfBeforeSegment, int32 DroppedOverSegmentID)
{
	RemoveFromOrder(DraggedSegmentID);
	int32 DropIndex = GetSegmentOrderIndex(DroppedOverSegmentID);
	if (DropIndex == -1)
	{
		return;
	}

	int32 InsertIndex = IfBeforeSegment ? DropIndex : DropIndex + 1;
	InsertIndex = FMath::Clamp(InsertIndex, 0, SegmentsOrder.Num());

	SegmentsOrder.Insert(DraggedSegmentID, InsertIndex);
}

bool FAVSDataAsset::AreSegmentsNeighbors(const int32 DraggedSegmentID, const bool IfDraggingOverTop, const int32 DroppedOverSegmentID, const bool ArchivedAreVisible)
{
	int32 NeighborSegment = INDEX_NONE;
	int32 Index = SegmentsOrder.IndexOfByKey(DroppedOverSegmentID);
	if (Index == INDEX_NONE)
	{
		return false;
	}
	if(ArchivedAreVisible)
	{
		if(IfDraggingOverTop && Index > 0)
		{
			NeighborSegment = SegmentsOrder[Index - 1];
		}
		if (!IfDraggingOverTop && Index < SegmentsOrder.Num() - 1)
		{
			NeighborSegment = SegmentsOrder[Index + 1];
		}
			
	}
	else
	{
		NeighborSegment = GetNeighborSegments(DroppedOverSegmentID, IfDraggingOverTop);
	}

	return NeighborSegment == DraggedSegmentID;
}

TArray<int32> FAVSDataAsset::GetSegmentIDContainingAsset(UObject* Asset)
{
	TArray<int32> SegmentsContainingAsset;
	
	for (auto& Element : Segments)
	{
		if (Element.Value.IfSegmentContainsAsset(Asset))
		{
			SegmentsContainingAsset.Add(Element.Key);
		}
	}
	return SegmentsContainingAsset;
}

TArray<int32> FAVSDataAsset::GetSegmentIDContainingAsset(const FString& Path)
{
	TArray<int32> SegmentsContainingAsset;
	
	for (auto& Element : Segments)
	{
		if (Element.Value.IfSegmentContainsAsset(Path))
		{
			SegmentsContainingAsset.Add(Element.Key);
		}
	}
	return SegmentsContainingAsset;
}


int32 FAVSDataAsset::GetNeighborSegments(int32 MySegment, const bool IfDraggingOverTop)
{
	int32 Index = SegmentsOrder.IndexOfByKey(MySegment);
	if (Index == INDEX_NONE)
	{
		return INDEX_NONE;
	}

	if(IfDraggingOverTop)
	{
		for (int32 i = Index - 1; i >= 0; --i)
		{
			if (!ArchivedSegments.Contains(SegmentsOrder[i]))
			{
				return SegmentsOrder[i];
			}
		}
	}
	else 
	{
		for (int32 i = Index + 1; i < SegmentsOrder.Num(); ++i)
		{
			if (!ArchivedSegments.Contains(SegmentsOrder[i]))
			{
				return SegmentsOrder[i];
			}
		}
	}
	return INDEX_NONE;
}

bool FAVSDataAsset::ArchiveSegment(const int32 SegmentID)
{
	if(Segments.Contains(SegmentID) && !IsArchived(SegmentID))
	{
		ArchivedSegments.Add(SegmentID);
		return true;
	}
	return false;
}

bool FAVSDataAsset::UnarchiveSegment(const int32 SegmentID)
{
	if(Segments.Contains(SegmentID) && IsArchived(SegmentID))
	{
		ArchivedSegments.Remove(SegmentID);
		return true;
	}
	return false;
}

bool FAVSDataAsset::IsArchived(const int32 SegmentID)
{
	return ArchivedSegments.Contains(SegmentID);
}

FAVSDataAsset& FAVSDataAsset::GetOrCreateData()
{
	static FAVSDataAsset CachedInstance;
	CachedInstance.LoadFromSavedFile();

	return CachedInstance;

}

bool FAVSDataAsset::LoadFromSavedFile()
{
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("FAVS");
	const FString FileName = TEXT("FAVSData.fav");
	const FString FullPath = Dir / FileName;

	if (!FPaths::FileExists(FullPath))
	{
		return false;
	}

	FString JsonString;
	if (!FFileHelper::LoadFileToString(JsonString, *FullPath))
	{
		return false;
	}

	if (FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, this, 0, 0))
	{
		return true;
	}

	return false;
}

void FAVSDataAsset::TestBeforeSaveAsset()
{
	for (auto& SegmentPair : Segments)
	{
		FAVSSegmentData& Data = SegmentPair.Value;
		for (FAVSAssetData& Asset : Data.Assets)
		{
			bool IsValid = Asset.IsValidAsset();
		}
	}
}

void FAVSDataAsset::SaveAsset()
{
	//TestBeforeSaveAsset();
	const FString Dir = FPaths::ProjectSavedDir() / TEXT("FAVS");
	IFileManager::Get().MakeDirectory(*Dir, true);

	const FString FileName = TEXT("FAVSData.fav");
	const FString FullPath = Dir / FileName;

	FString JsonString;
	if (FJsonObjectConverter::UStructToJsonObjectString(*this, JsonString))
	{
		if (FFileHelper::SaveStringToFile(JsonString, *FullPath))
		{
			//UE_LOG(LogTemp, Warning, TEXT("Saved! FAVS data to %s"), *FullPath);
		}
		else
		{
			//UE_LOG(LogTemp, Error, TEXT("Failed to save FAVS data to %s"), *FullPath);
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("Failed to convert FAVS data to JSON"));
	}
}
