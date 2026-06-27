// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSSegment.h"
#include "ClassIconFinder.h"
#include "FAVSElement.h"
#include "FAVSElementDragDropOp.h"
#include "FAVSElementFolder.h"
#include "FAVSElementAsset.h"
#include "FAVSElementSetting.h"
#include "FAVSEvents.h"
#include "FAVSHeader.h"
#include "FAVSHelper.h"
#include "FAVSSegmentDragDropOp.h"
#include "SlateOptMacros.h"
#include "DragAndDrop/AssetDragDropOp.h"

class FAssetDragDropOp;
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION



void FAVSSegment::Construct(const FArguments& InArgs)
{
	SegmentID = InArgs._SegmentID;
	Parent = InArgs._Parent;
	
	FAVSSegmentData* TempSegmentData = GetSegmentData();
	SegmentName = TempSegmentData->SegmentName;
	IsCollapsed = TempSegmentData->IsCollapsed;
	
	VerticalBox = SNew(SVerticalBox);
	CreateHeader();
	CreateListItems();
	CreateListView();
		
	SegmentBorder = SNew(SBox)
	
	.Padding(0)
	[
		VerticalBox.ToSharedRef()
	];
	
	ChildSlot
	[
		SegmentBorder.ToSharedRef()
	];
	FAVSEvents::OnAssetAdded.AddSP(this, &FAVSSegment::OnAssetAdded);
}

void FAVSSegment::Destruct()
{
}

void FAVSSegment::OnAssetAdded(const FAVSAssetData& AssetData, int32 _SegmentID)
{
	FAVSSegmentData* SegmentData = GetSegmentData();
	if (!SegmentData)
	{
		return;
	}
	
	if(SegmentID == _SegmentID && CanAddElement(AssetData))
	{
		SegmentData->AddElement(AssetData, -1);

		if(IsCollapsed)
		{
			SetListVisibility(true);
			Header->SetToggleBtnStyle();
		}
		RecreateListView();

		GetParent()->SaveAsset();
	}
}

void FAVSSegment::CreateListView()
{
	
	VerticalBox->AddSlot()
	.AutoHeight()
	[
		SAssignNew(ListViewWidget, SListView<TSharedPtr<FAVSAssetData>>)
		.Visibility(this, &FAVSSegment::GetListViewVisibility)
		.ListItemsSource(&AssetItems)
		.SelectionMode(ESelectionMode::None)
		.OnGenerateRow(this, &FAVSSegment::OnGenerateRow)
	];
}

void FAVSSegment::TurnOffBtns()
{
	TArray<TWeakPtr<FAVSElement>> Elements;
	ElementWidgets.GenerateValueArray(Elements);
	
	for (TWeakPtr<FAVSElement>& WeakElement : Elements)
	{
		if (TSharedPtr<FAVSElement> FAVSElement = WeakElement.Pin())
		{
			if(FAVSElement->IsLevel()) FAVSElement->UpdateButtonStyle(false);
		}
	}
}

void FAVSSegment::UpdateBtns()
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	FString FullSoftPath;
	if (EditorWorld && EditorWorld->PersistentLevel)
	{
		FString OpenLevelPath = EditorWorld->PersistentLevel->GetOutermost()->GetName();
		FString AssetName = FPaths::GetBaseFilename(OpenLevelPath);
		FullSoftPath = OpenLevelPath + TEXT(".") + AssetName;
		
	}
	FSoftObjectPath LevelPath(FullSoftPath);
   	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem)
	{
		return;
	}

	TArray<UObject*> OpenAssets = AssetEditorSubsystem->GetAllEditedAssets();
    
	TSet<FSoftObjectPath> OpenAssetPaths;
	OpenAssetPaths.Reserve(OpenAssets.Num());
    
	for (UObject* OpenAsset : OpenAssets)
	{
		if (OpenAsset)
		{
			OpenAssetPaths.Add(FSoftObjectPath(OpenAsset));
		}
	}
	OpenAssetPaths.Add(LevelPath);
	
	for (const TSharedPtr<FAVSAssetData>& Element : AssetItems)
	{
		if (!Element.IsValid() || Element->AssetType == EAssetType::Folder) continue;
		
		bool IsOpen = OpenAssetPaths.Contains(Element->AssetPath);
        if (IsOpen)
		{
			UpdateOpenAssetBtnStyle(Element->AssetPath, true);
		}
	}
}

TSharedRef<ITableRow> FAVSSegment::OnGenerateRow(TSharedPtr<FAVSAssetData> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<FAVSElement> ElementWidget;
	switch (InItem->AssetType)
	{
		case EAssetType::Settings:
			ElementWidget = StaticCastSharedRef<FAVSElement>(SNew(FAVSElementSetting).Asset(*InItem).SegmentID(SegmentID).Parent(this));
			break;

		case EAssetType::Folder:
			ElementWidget = StaticCastSharedRef<FAVSElement>(SNew(FAVSElementFolder).Asset(*InItem).SegmentID(SegmentID).Parent(this));
			break;

		default:
			ElementWidget = StaticCastSharedRef<FAVSElement>(SNew(FAVSElementAsset).Asset(*InItem).SegmentID(SegmentID).Parent(this));
			break;
	}
		
	TSharedRef<STableRow<TSharedPtr<FAVSAssetData>>> Row =	SNew(STableRow<TSharedPtr<FAVSAssetData>>, OwnerTable)
	[
		ElementWidget.ToSharedRef()
	];
	
	ElementWidgets.Add(InItem, ElementWidget);
	
	FSoftObjectPath AssetPath = InItem.Get()->AssetPath;
	AssetWidgets.Add(AssetPath ,ElementWidget);

	RowCreated++;
	if(RowCreated == AssetItems.Num())
	{
		UpdateBtns();
	}
	return Row;
}

void FAVSSegment::CreateHeader()
{
	VerticalBox->AddSlot()
	.AutoHeight()
	[
		 SAssignNew(Header, FAVSHeader)
		.SegmentID(SegmentID)
		.Parent(this)
	];
}

void FAVSSegment::CreateListItems()  
{
	AssetItems.Reset();

	FAVSSegmentData* SegmentData = GetSegmentData();
	
	for (FAVSAssetData& Asset : SegmentData->Assets)
	{
		if (Asset.AssetType == EAssetType::Settings)
		{
			Asset.IsValid = true;
		}
		else if (Asset.AssetType == EAssetType::Folder)
		{
			FString FullPath;
			if (Asset.AssetPath.ToString().StartsWith(TEXT("/Game")))
			{
				FPackageName::TryConvertLongPackageNameToFilename(Asset.AssetPath.ToString(), FullPath);
			}
			else
			{
				FullPath = Asset.AssetPath.ToString();
			}
			
			Asset.IsValid = IFileManager::Get().DirectoryExists(*FullPath);
		}
		else
		{
			Asset.IsValid = FPackageName::DoesPackageExist(Asset.AssetPath.GetLongPackageName());
			
		}

		AssetItems.Add(MakeShared<FAVSAssetData>(Asset));
	}
}

EVisibility FAVSSegment::GetListViewVisibility() const
{
	return IsCollapsed ? EVisibility::Collapsed : EVisibility::Visible;
}

void FAVSSegment::SetListVisibility(const bool IfVisible)
{
	IsCollapsed = !IfVisible;
	ListViewWidget->SetVisibility(GetListViewVisibility());
	FAVSSegmentData* SegmentData = GetSegmentData();
	SegmentData->IsCollapsed = IsCollapsed;
}

void FAVSSegment::RemoveElement(const FSoftObjectPath& AssetPath)
{
	AssetWidgets.Remove(AssetPath);

	TSharedPtr<FAVSAssetData> FoundPtr = nullptr;
	for (const TSharedPtr<FAVSAssetData>& Item : AssetItems)
	{
		if (Item.IsValid() && Item->AssetPath == AssetPath)
		{
			FoundPtr = Item;
			ElementWidgets.Remove(Item);
			break;
		}
	}

	if (FoundPtr.IsValid())
	{
		FAVSSegmentData* SegmentData = GetSegmentData();
		SegmentData->RemoveElement(*FoundPtr.Get()); //todo tu mialem blad, dlaczego 2x jest wywolywane przy Ondrop w tym samym segmencie ??
		AssetItems.RemoveSingle(FoundPtr); 
		GetParent()->RemovePendingFromSegment();
		GetParent()->ResetPendingElement();
	}
}

void FAVSSegment::RecreateListView()
{
	RowCreated = 0;
	ElementWidgets.Empty();
	AssetWidgets.Empty();
	
	CreateListItems();
	if (ListViewWidget.IsValid())
	{
		ListViewWidget->RequestListRefresh();
	}
		
}

void FAVSSegment::UpdateOpenAssetBtnStyle(const FSoftObjectPath& AssetPath, bool IfOpened)
{
	if (TWeakPtr<FAVSElement>* WidgetPtr = AssetWidgets.Find(AssetPath))
	{
		if (TSharedPtr<FAVSElement> Widget = WidgetPtr->Pin())
		{
			Widget->UpdateButtonStyle(IfOpened);
		}
	}
}

void FAVSSegment::PreReplaceAsset(const FAssetData& Asset, const FString& OldPath, const FString& NewFolderPath, bool IsFolder)
{
	FAVSAssetData AssetData;
	
	if(IsFolder)
	{
		AssetData = { FPaths::GetCleanFilename(NewFolderPath), FSoftObjectPath(NewFolderPath), EAssetType::Folder, nullptr, FColor(200,157, 78,255)}; //todo<-ten kolor musze poprawic
	}else
	{
		const FSoftObjectPath& AssetPath = Asset.ToSoftObjectPath();
		EAssetType AssetType = FAVSHelper::GetAssetTypeFromAssetData(Asset);
		const UClass* AssetClass = FClassIconFinder::GetIconClassForAssetData(Asset);
		const FColor IconColor = FAVSHelper::GetIconColor(Asset.GetClass());

		AssetData = {Asset.AssetName.ToString(), AssetPath, AssetType, AssetClass, IconColor};

	}

	ReplaceAsset(AssetData,OldPath);
	
}

void FAVSSegment::ReplaceAsset(const FAVSAssetData& AssetData, const FString& OldPath)
{
	FAVSSegmentData* SegmentData = GetSegmentData();
	SegmentData->ReplaceElement(AssetData, OldPath);
	if (TWeakPtr<FAVSElement>* Found = AssetWidgets.Find(OldPath))
	{
		if (Found->IsValid())
		{
			TSharedPtr<FAVSElement> Widget = Found->Pin();

			AssetWidgets.Remove(OldPath);
			AssetWidgets.Add(AssetData.AssetPath, Widget);

			Widget->UpdateWidget(AssetData);
			
			FSoftObjectPath OldSoftPath(OldPath);
			for (const TSharedPtr<FAVSAssetData>& ElementPtr : AssetItems)
			{
				if (!ElementPtr.IsValid()) continue;
				
				if(ElementPtr->AssetPath == OldSoftPath)
				{
					ElementPtr->DisplayName = AssetData.DisplayName;
					ElementPtr->AssetPath   = AssetData.AssetPath;
				}
			}
		}
	}
}

FAVSSegmentData* FAVSSegment::GetSegmentData()
{
	return GetParent()->GetSegmentData(SegmentID);
}

FReply FAVSSegment::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if(!Operation.IsValid()) return FReply::Unhandled();
	FAVSSegmentData* SegmentData = GetSegmentData();
	if (Operation->IsOfType<FAVSElementDragDropOp>())
	{
		auto ElementOperation = StaticCastSharedPtr<FAVSElementDragDropOp>(Operation);
		
		if(CanAddElement(ElementOperation->Asset))
		{
			SegmentData->AddElement(ElementOperation->Asset, CurrentDropIndex);
			
			if(IsCollapsed)
			{
				SetListVisibility(true);
				Header->SetToggleBtnStyle();
			}
			RecreateListView();
			GetParent()->SaveAsset();
		}
		else
		{
			GetParent()->ResetPendingElement();
		}
		OnDragLeave(DragDropEvent); 
		return FReply::Handled();
	}

	if (Operation->IsOfType<FAssetDragDropOp>())
	{
		const TSharedPtr<FAssetDragDropOp> AssetOperation = StaticCastSharedPtr<FAssetDragDropOp>(Operation);
		const TArray<FString>& FolderPaths = AssetOperation->GetAssetPaths();
		
		int32 i = 0;
		if (FolderPaths.Num() > 0)
		{
			for (const FString& FolderPath : FolderPaths)
			{
				FAVSAssetData AssetData = { FPaths::GetCleanFilename(FolderPath), FSoftObjectPath(FolderPath), EAssetType::Folder, nullptr,FColor(200,157, 78,255)}; // FColor(160,70, 20,210)};
				if(CanAddElement(AssetData))
				{
					SegmentData->AddElement(AssetData, CurrentDropIndex + i);
					i++;
				}else
				{
				}
			}
			
		}
		for (const FAssetData& Asset : AssetOperation->GetAssets())
		{
			const FSoftObjectPath& AssetPath = Asset.ToSoftObjectPath();
			EAssetType AssetType = FAVSHelper::GetAssetTypeFromAssetData(Asset);
			const UClass* AssetClass = FClassIconFinder::GetIconClassForAssetData(Asset);
			const FColor IconColor = FAVSHelper::GetIconColor(Asset.GetClass());
			FAVSAssetData AssetData = {Asset.AssetName.ToString(), AssetPath, AssetType, AssetClass, IconColor};
			if(CanAddElement(AssetData))
			{
				SegmentData->AddElement(AssetData, CurrentDropIndex + i);
				i++;
			}
			
		}
		
		if(IsCollapsed)
		{
			SetListVisibility(true);
			Header->SetToggleBtnStyle();
		}
		RecreateListView();
		GetParent()->SaveAsset();
		OnDragLeave(DragDropEvent);
		return FReply::Handled();
	}
	if (Operation->IsOfType<FAVSSegmentDragDropOp>()){
		const TSharedPtr<FAVSSegmentDragDropOp> SegmentOperation = StaticCastSharedPtr<FAVSSegmentDragDropOp>(Operation);
		
		if(SegmentOperation->SegmentID != SegmentID)
		{
			GetParent()->ChangeSegmentOrder(SegmentOperation->SegmentID, IfInsertBefore, SegmentID);
			return FReply::Handled();
		}
	}
	return FReply::Unhandled();
}

void FAVSSegment::OnDragLeave(const FDragDropEvent& DragDropEvent)
{
	IsHoveredByDrag = false;
	CurrentDropIndex = -1;
	
	ResetPaddings();
	if (SegmentBorder.IsValid())
	{
		SegmentBorder->SetPadding(0);
	}
	SCompoundWidget::OnDragLeave(DragDropEvent);
}

FReply FAVSSegment::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent)
{
	TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
	if(!Operation.IsValid()) return FReply::Unhandled();

	if (Operation->IsOfType<FAVSSegmentDragDropOp>())
	{
		
		const TSharedPtr<FAVSSegmentDragDropOp> SegmentOperation = StaticCastSharedPtr<FAVSSegmentDragDropOp>(Operation);

		if(SegmentID == SegmentOperation->SegmentID)
		{
			return FReply::Handled();
		}

		IsHoveredByDrag = true;

		const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(DragDropEvent.GetScreenSpacePosition());
		const float HalfHeight = MyGeometry.GetLocalSize().Y / 2.0f;
		
		IfInsertBefore = LocalPos.Y < HalfHeight;
		if (SegmentBorder.IsValid()) 
		{
			bool AreSegmentsNeighbors = GetParent()->AreSegmentsNeighbors(SegmentOperation->SegmentID, IfInsertBefore, SegmentID);
			if(!AreSegmentsNeighbors)
			{
				if(IfInsertBefore)	SegmentBorder->SetPadding(FMargin(0, 15, 0, 0));
				else 	SegmentBorder->SetPadding(FMargin(0, 0, 0, 15));
			}
		}
		return FReply::Handled();
	}

	int32 NewDropIndex = FindDropIndexFromListView(DragDropEvent);
	
	if (NewDropIndex != CurrentDropIndex)
	{
		CurrentDropIndex = NewDropIndex;
		
		ResetPaddings();

		if (ListViewWidget.IsValid())
		{
			TSharedPtr<FAVSElement> ElementWidget_1;
			TSharedPtr<FAVSElement> ElementWidget_2;

			if (AssetItems.IsValidIndex(CurrentDropIndex))
			{
				TSharedPtr<FAVSAssetData>& Item = AssetItems[CurrentDropIndex];
				TWeakPtr<FAVSElement> WeakRow = ElementWidgets.FindRef(Item);
				if(WeakRow.IsValid())
				{
					ElementWidget_1 = WeakRow.Pin();
					ElementWidget_1->SetPaddingWhenDragOver(10,0);
				}
			}
			else if (AssetItems.IsValidIndex(CurrentDropIndex - 1))
			{
				TSharedPtr<FAVSAssetData>& Item = AssetItems[CurrentDropIndex - 1];
				TWeakPtr<FAVSElement> WeakRow = ElementWidgets.FindRef(Item);
				if(WeakRow.IsValid())
				{
					ElementWidget_2 = WeakRow.Pin();
					ElementWidget_2->SetPaddingWhenDragOver(0, 10);
				}
			}
		
			ListViewWidget->RequestListRefresh();
		}
	}
	return FReply::Handled();
}

void FAVSSegment::ResetPaddings()
{
	for (const TPair<TSharedPtr<FAVSAssetData>, TWeakPtr<FAVSElement>>& Pair : ElementWidgets)
	{
		if (TSharedPtr<FAVSElement> Element = Pair.Value.Pin())
		{
			Element->SetPaddingWhenDragOver(0,0);
		}
	}
}

FAVSTab* FAVSSegment::GetParent()
{
	return Parent;
}

int32 FAVSSegment::FindDropIndexFromListView(const FDragDropEvent& DragDropEvent)
{
	int32 DropIndex = AssetItems.Num();
	const FVector2D ScreenDropPos = DragDropEvent.GetScreenSpacePosition();

	for (int32 i = 0; i < AssetItems.Num(); ++i)
	{
		const TSharedPtr<FAVSAssetData>& Item = AssetItems[i];
		TWeakPtr<FAVSElement> WeakRow = ElementWidgets.FindRef(Item);

		if (TSharedPtr<FAVSElement> Row = WeakRow.Pin())
		{
			const TSharedRef<SWidget> RowWidget = Row.ToSharedRef();
			FGeometry RowGeo = RowWidget->GetCachedGeometry();

			if (RowGeo.IsUnderLocation(ScreenDropPos))
			{
				const FVector2D LocalPos = RowGeo.AbsoluteToLocal(ScreenDropPos);

				if (LocalPos.Y < RowGeo.GetLocalSize().Y / 2)
				{
					return i;
				}
				else
				{
					return i + 1;
				}
			}
		}
	}

	return DropIndex;
}

bool FAVSSegment::CanAddElement(const FAVSAssetData& Asset)
{
	FAVSSegmentData* SegmentData = GetSegmentData();
	
	if (!SegmentData)
	{
		return false;
	}
	return !SegmentData->Assets.Contains(Asset);
}


END_SLATE_FUNCTION_BUILD_OPTIMIZATION
