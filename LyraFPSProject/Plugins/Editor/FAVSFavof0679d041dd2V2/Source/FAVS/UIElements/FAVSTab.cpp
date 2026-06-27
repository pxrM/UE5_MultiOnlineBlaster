// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSTab.h"
#include "SlateOptMacros.h"
#include "../UIElements/FAVSSegment.h"
#include "ContentBrowserModule.h"
#include "FAVSHelper.h"
#include "FAVSSettings.h"
#include "FAVSStyle.h"
#include "FAVSToggleArchive.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IPluginManager.h"

class ULevelEditorSubsystem;
BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
#define LOCTEXT_NAMESPACE "FAVS"


void FAVSTab::Construct(const FArguments& InArgs)
{
	DataAsset = InArgs._FAVSData;
	bCanSupportFocus = true; 
	
	const FString Header = InArgs._TestString;

	if(DataAsset->IsArchivedVisible)
	{
		ArchiveToggleState = ECheckBoxState::Checked;
	}
	
	FString PluginContentDir = IPluginManager::Get().FindPlugin(TEXT("FAVS"))->GetContentDir();

	TSharedPtr<FSlateImageBrush> ToggleOnBrush = MakeShareable(new FSlateImageBrush(
		PluginContentDir / TEXT("Resources/ToggleOn"), FVector2D(64, 64)));

	TSharedPtr<FSlateImageBrush> ToggleOffBrush = MakeShareable(new FSlateImageBrush(
		PluginContentDir / TEXT("Resources/ToggleOff"), FVector2D(64, 64)));

	TSharedRef<FCheckBoxStyle> CustomToggleStyle = MakeShared<FCheckBoxStyle>();
	CustomToggleStyle->SetCheckedImage(*ToggleOnBrush)
		.SetCheckedHoveredImage(*ToggleOnBrush)
		.SetCheckedPressedImage(*ToggleOnBrush)
		.SetUncheckedImage(*ToggleOffBrush)
		.SetUncheckedHoveredImage(*ToggleOffBrush)
		.SetUncheckedPressedImage(*ToggleOffBrush);

	ChildSlot
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5,10,5,5)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			   .FillWidth(1.0f)
			   [
				   SNullWidget::NullWidget
			   ]
				
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(FAVSToggleArchive)
					.IsChecked(ArchiveToggleState == ECheckBoxState::Checked)
					.ToolTipText(LOCTEXT("ToggleArchiveTooltip", "Toggle archive on/off"))
					.OnToggleChanged(this, &FAVSTab::OnToggleChanged)
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(5,0,0,0)
				[
					SNew(SButton)
					.ButtonStyle(&FAVSStyle::Get().GetWidgetStyle<FButtonStyle>("ButtonAdd"))
					.ToolTipText(LOCTEXT("AddSegmentTooltip", "Add new segment"))
					.OnClicked(this, &FAVSTab::AddSegment)
				]
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(0,5,0,5)
			[
				CreateSegmentsFromData()
			]
		];


	CBModule = &FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();

	TSharedRef<FAVSTab> MyTab = SharedThis(this);
	AssetEditorSubsystem->OnAssetOpenedInEditor().AddSP(MyTab, &FAVSTab::HandleAssetOpened);
	
	AssetEditorSubsystem->OnAssetClosedInEditor().AddSP(MyTab, &FAVSTab::HandleAssetClosed);

	FEditorDelegates::OnAssetsPreDelete.AddSP(MyTab, &FAVSTab::HandleAssetDeleted);
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	AssetRegistryModule.Get().OnAssetRenamed().AddSP(MyTab, &FAVSTab::HandleAssetRenamed);

	FEditorDelegates::OnMapOpened.AddSP(MyTab, &FAVSTab::HandleOnMapOpened); 
	
	AssetRegistryModule.Get().OnPathsAdded().AddSP(MyTab, &FAVSTab::HandlePathsAdded);
	AssetRegistryModule.Get().OnPathsRemoved().AddSP(MyTab, &FAVSTab::HandlePathsRemoved);

	FAVSEvents::OnSettingsToggleChanged.AddSP(MyTab, &FAVSTab::OnShowSettings);
		
}

void FAVSTab::HandleAssetOpened(UObject* Object, IAssetEditorInstance* AssetEditorInstance)
{
	UpdateElementWidget(Object, true);
}

void FAVSTab::HandleAssetClosed(UObject* Object, IAssetEditorInstance* AssetEditorInstance)
{
	UpdateElementWidget(Object, false);
}

void FAVSTab::HandleAssetRenamed(const FAssetData& Asset, const FString& OldPath)
{
		
	FString OldAssetFolder = FPaths::GetPath(OldPath);        
	FString NewAssetFolder = FPaths::GetPath(Asset.ToSoftObjectPath().ToString());

	FString OldFolder;
	FString NewFolder;
	bool IfChanged = FAVSHelper::ExtractMovedFolder(OldAssetFolder, NewAssetFolder, OldFolder, NewFolder);
	if(IfChanged)
	{
		if(LastFolderChange.NewFolder != NewFolder || LastFolderChange.OldFolder != OldFolder)
		{
			LastFolderChange.ChangeTime = FPlatformTime::Seconds();
			LastFolderChange.OldFolder = OldFolder;
			LastFolderChange.NewFolder = NewFolder;
		}
	}
	
	TArray<TSharedPtr<FAVSSegment>> Segmenty = GetSegmentsForAsset(OldPath);
	for (auto& Segment : Segmenty)
	{
		Segment->PreReplaceAsset(Asset, OldPath, FString(), false);
	}

	SaveAsset();
}

void FAVSTab::HandleOnMapOpened(const FString& Filename, bool bAsTemplate)
{
	TArray<int32> SegmentIDs;
	DataAsset->Segments.GetKeys(SegmentIDs);

	for (const int32 SegmentID : SegmentIDs)
	{
		if (TSharedPtr<FAVSSegment>* SegmentPtr = SegmentWidgets.Find(SegmentID))
		{
			if (SegmentPtr->IsValid())
			{
				(*SegmentPtr)->TurnOffBtns();
			}
		}
	}

	FString PackageName;
	if (FPackageName::TryConvertFilenameToLongPackageName(Filename, PackageName))
	{
		FString AssetName = FPaths::GetBaseFilename(Filename);
		FString FullSoftPath = PackageName + TEXT(".") + AssetName;

		TArray<TSharedPtr<FAVSSegment>> Segmenty = GetSegmentsForAsset(FullSoftPath);

		for (auto& Segment : Segmenty)
		{
			Segment->UpdateOpenAssetBtnStyle(FullSoftPath, true);
		}
	}
	else
	{
	}
}

void FAVSTab::UpdateElementWidget(UObject* Object, const bool IfOpened)
{
	
	FSoftObjectPath Path(Object);
	TArray<TSharedPtr<FAVSSegment>> Segmenty = GetSegmentsForAsset(Object);
	
	for (auto& Segment : Segmenty)
	{
		Segment->UpdateOpenAssetBtnStyle(Path, IfOpened);
	}
}

void FAVSTab::HandleAssetDeleted(const TArray<UObject*>& DeletedAssets)
{
	for (UObject* Object : DeletedAssets)
	{
		if (!Object) continue;

		const FSoftObjectPath Path(Object);

		for (auto& Segment : GetSegmentsForAsset(Object))
		{
			Segment->RemoveElement(Path);
		}
		
	}

	SaveAsset();
}

void FAVSTab::HandlePathsAdded(TConstArrayView<FStringView> Paths)
{
	
	double CurrentTime = FPlatformTime::Seconds();
	auto TimePassed = CurrentTime - LastFolderChange.ChangeTime;

	if(Paths.Num() == 1 && TimePassed > .3)
	{
		LastFolderChange.IfSingleFolderChange = true;
		LastFolderChange.NewFolder = FString(Paths[0]);
	}
	else
	{
		LastFolderChange.IfSingleFolderChange = false;
	}
	
	
	LastFolderChange.ChangeTime = CurrentTime;
}

void FAVSTab::HandlePathsRemoved(TConstArrayView<FStringView> Paths)
{
	double CurrentTime = FPlatformTime::Seconds();
	auto TimePassed = CurrentTime - LastFolderChange.ChangeTime;
	if (TimePassed >= 1)
	{
		for (FStringView PathView : Paths)
		{
			RemoveFolder(FString(PathView));
		}
				
	}
	else if(LastFolderChange.IfSingleFolderChange)
	{
		LastFolderChange.OldFolder = FString(Paths.Last());
		RenameFolder(LastFolderChange.OldFolder, LastFolderChange.NewFolder);
	}
	else
	{
		for (FStringView PathView : Paths)
		{
			FString OldFolder(PathView);
			FString NewFolder = GetNewFolderName(OldFolder);
						
			RenameFolder(OldFolder,NewFolder);
		}
	}
	
	LastFolderChange.IfSingleFolderChange = false;
	LastFolderChange.ChangeTime = CurrentTime;
	SaveAsset();
}

FString FAVSTab::GetNewFolderName(const FString& OldFolder) const
{
	
	FString NewFolder = OldFolder;
	if (OldFolder.StartsWith(LastFolderChange.OldFolder))
	{
		NewFolder = LastFolderChange.NewFolder + OldFolder.Mid(LastFolderChange.OldFolder.Len());
	}
	return NewFolder;
}

void FAVSTab::RenameFolder(const FString& OldPath, const FString& NewPath)
{
	TArray<TSharedPtr<FAVSSegment>> Segmenty = GetSegmentsForAsset(OldPath);
	for (auto& Segment : Segmenty)
	{
		Segment->PreReplaceAsset(FAssetData(), OldPath, NewPath, true);  //todo to wlaczyc
	}

	
}

void FAVSTab::RemoveFolder(const FString& Path)
{
	const FSoftObjectPath SoftPath(Path);

	for (auto& Segment : GetSegmentsForAsset(Path))
	{
		Segment->RemoveElement(SoftPath);
	}
	
}

TArray<TSharedPtr<FAVSSegment>> FAVSTab::GetSegmentsForAsset(UObject* Asset)
{
	TArray<TSharedPtr<FAVSSegment>> Segments;
	
    TArray<int32> SegmentIDs = DataAsset->GetSegmentIDContainingAsset(Asset);
		
    for (int32 SegmentID : SegmentIDs)
    {
	    if (TSharedPtr<FAVSSegment>* Segment = SegmentWidgets.Find(SegmentID))
        {
            if (Segment->IsValid())
            {
                Segments.Add(*Segment);
            }
        }
    }
    return Segments;
}

TArray<TSharedPtr<FAVSSegment>> FAVSTab::GetSegmentsForAsset(const FString& Path)
{
	TArray<TSharedPtr<FAVSSegment>> Segments;
	
	TArray<int32> SegmentIDs = DataAsset->GetSegmentIDContainingAsset(Path);
	
	for (int32 SegmentID : SegmentIDs)
	{
		if (TSharedPtr<FAVSSegment>* Segment = SegmentWidgets.Find(SegmentID))
		{
			if (Segment->IsValid())
			{
				Segments.Add(*Segment);
			}
		}
	}
	return Segments;
}

void FAVSTab::OnToggleChanged(const ECheckBoxState NewState)
{
	ArchiveToggleState = NewState;
	if(!DataAsset->ArchivedSegments.IsEmpty())
	{
		if (NewState == ECheckBoxState::Checked)
		{
			RebuildSegments();
		}
		else
		{
			RemoveArchivedSegmentsFromList();
		}
	}
	
	DataAsset->IsArchivedVisible = ArchiveToggleState==ECheckBoxState::Checked;
	SaveAsset();
}

void FAVSTab::RemoveArchivedSegmentsFromList()
{
	for (auto SegmentID : DataAsset->ArchivedSegments)
	{
		RemoveSegmentFromList(SegmentID);
	}
}

void FAVSTab::RemoveSegmentFromList(const int32 SegmentID)
{
	if (TSharedPtr<FAVSSegment>* SegmentWidgetPtr = SegmentWidgets.Find(SegmentID))
	{
		if (SegmentWidgetPtr->IsValid())
		{
			ScrollBoxSegments->RemoveSlot(SegmentWidgetPtr->ToSharedRef());
		}
	}
}

void FAVSTab::SetPendingElement(const FPendingElement& Element)
{
	PendingElement = Element;
}

FReply FAVSTab::AddSegment()
{
	int32 SegmentID = DataAsset->GetFirstAvailableSegmentID();
	FAVSSegmentData Segment = FAVSSegmentData(LOCTEXT("SegmentDefaultName", "Enter name"), SegmentID);
	DataAsset->Segments.Add(SegmentID, Segment); 
	DataAsset->SegmentsOrder.Add(SegmentID);

	TSharedRef<FAVSSegment> SegmentWidget = SNew(FAVSSegment).SegmentID(SegmentID).Parent(this);

	ScrollBoxSegments->AddSlot()
	[
		SegmentWidget
	];
	SegmentWidgets.Add(SegmentID, SegmentWidget);
	
	
	SaveAsset();
	return FReply::Handled();
}

TSharedRef<SScrollBox> FAVSTab::CreateSegmentsFromData()
{
	ScrollBoxSegments = SNew(SScrollBox);
	SegmentWidgets.Empty();
	
	if (!DataAsset)
	{
		return ScrollBoxSegments.ToSharedRef();
	}

	AddSettingsSegmentData();
	
	for (int i = 0; i < DataAsset->SegmentsOrder.Num(); ++i)
	{
		int32 SegmentID = DataAsset->SegmentsOrder[i];
		if(ArchiveToggleState == ECheckBoxState::Unchecked && IsSegmentArchived(SegmentID)) continue; //ECheckBoxState::Unchecked = czyli ukryte

		
		TSharedRef<FAVSSegment> Segment = SNew(FAVSSegment).SegmentID(SegmentID).Parent(this);
			
		SegmentWidgets.Add(SegmentID, Segment);

		ScrollBoxSegments->AddSlot()
		[
			Segment
		];
	}
	return ScrollBoxSegments.ToSharedRef();
}

void FAVSTab::OnShowSettings()
{
	bool IfDisplaySettings = GetDefault<UFAVSSettings>()->DisplaySettings;

	if(IfDisplaySettings)
	{
		AddSettingSegment();
	}else
	{
		RemoveSettingsSegment();
	}
	
}

int32 FAVSTab::AddSettingsSegmentData()
{
	if(DataAsset->HasSettingsSegment) return -1;
	
	bool IfDisplaySettings = GetDefault<UFAVSSettings>()->DisplaySettings;
	if(!IfDisplaySettings) return -1;

	
	int32 SegmentID = DataAsset->GetFirstAvailableSegmentID();
	FAVSSegmentData SettingsSegment = FAVSSegmentData(LOCTEXT("SettingSegment", "Settings"), SegmentID);
	SettingsSegment.IsSettingSegment = true;
	
	FAVSAssetData Setting1{LOCTEXT("EditorPreferences", "Editor Preferences").ToString(), FSoftObjectPath(TEXT("/Virtual/EditorPreferences")), EAssetType::Settings, nullptr, FColor::White };
	FAVSAssetData Setting2{LOCTEXT("ProjectSettings", "Project Settings").ToString(),  FSoftObjectPath(TEXT("/Virtual/ProjectSettings")), EAssetType::Settings, nullptr, FColor::White };
	Setting1.SettingType = ESettingsWindow::EditorPreferences;
	Setting2.SettingType = ESettingsWindow::ProjectSettings;
		
	SettingsSegment.Assets = {Setting1, Setting2};
	
	DataAsset->Segments.Add(SegmentID, SettingsSegment);
	DataAsset->SegmentsOrder.Add(SegmentID);
	DataAsset->HasSettingsSegment = true;
	SaveAsset();

	return SegmentID;
}

FReply FAVSTab::AddSettingSegment()
{
	int32 SegmentID = AddSettingsSegmentData();
	if(SegmentID == -1) return FReply::Handled();
	
	
	TSharedRef<FAVSSegment> SegmentWidget = SNew(FAVSSegment).SegmentID(SegmentID).Parent(this);

	ScrollBoxSegments->AddSlot()
	[
		SegmentWidget
	];
	SegmentWidgets.Add(SegmentID, SegmentWidget);
		
	return FReply::Handled();
}

void FAVSTab::RemoveSettingsSegment()
{
	for (auto& Segment : DataAsset->Segments)
	{
		if (Segment.Value.IsSettingSegment)
		{
			RemoveSegment(Segment.Key);
			break;
		}
	}
}

void FAVSTab::RebuildSegments()
{
	if (!ScrollBoxSegments.IsValid()) return;

	ScrollBoxSegments->ClearChildren();
	SegmentWidgets.Empty();
	for (int i = 0; i < DataAsset->SegmentsOrder.Num(); ++i)
	{
		int32 SegmentID = DataAsset->SegmentsOrder[i];
		if(ArchiveToggleState == ECheckBoxState::Unchecked && IsSegmentArchived(SegmentID)) continue;
				
		
		TSharedRef<FAVSSegment> SegmentWidget =	SNew(FAVSSegment).SegmentID(SegmentID).Parent(this);
			
		ScrollBoxSegments->AddSlot()
		[
			SegmentWidget
		];
		SegmentWidgets.Add(SegmentID, SegmentWidget);
		
	}
}

bool FAVSTab::IsSegmentArchived(const int32 SegmentID) const
{
	return DataAsset->ArchivedSegments.Contains(SegmentID);
}

void FAVSTab::SaveAsset()
{
	DataAsset->SaveAsset();
}

FAVSSegmentData* FAVSTab::GetSegmentData(int32 SegmentID)
{
	return DataAsset->Segments.Find(SegmentID);
}

void FAVSTab::RemoveSegment(const int32 SegmentID)
{
	
	FAVSSegmentData* SegmentPtr = DataAsset->Segments.Find(SegmentID);
	if (!SegmentPtr)
	{
		return;
	}
	
	if(SegmentPtr->IsSettingSegment)
	{		
		DataAsset->HasSettingsSegment = false;
		UFAVSSettings::SetSettingsForSegment(false);
	}
	
	DataAsset->RemoveSegment(SegmentID);
	DataAsset->RemoveFromOrder(SegmentID);
	DataAsset->RemoveFromArchived(SegmentID);

	if (TSharedPtr<FAVSSegment>* SegmentWidgetPtr = SegmentWidgets.Find(SegmentID))
	{
		if (SegmentWidgetPtr->IsValid())
		{
			ScrollBoxSegments->RemoveSlot(SegmentWidgetPtr->ToSharedRef());
		}
	}
	SegmentWidgets.Remove(SegmentID);
	SaveAsset();
	
}

void FAVSTab::ChangeSegmentOrder(const int32 DraggedSegmentID, const bool IfBeforeSegment, const int32 DroppedOverSegmentID)
{
	DataAsset->ChangeSegmentOrder(DraggedSegmentID, IfBeforeSegment, DroppedOverSegmentID);
	RebuildSegments();
	SaveAsset();
}

bool FAVSTab::AreSegmentsNeighbors(const int32 DraggedSegmentID, const bool IfDraggingOverTop, const int32 DroppedOverSegmentID)
{
	bool ArchivedVisible = ArchiveToggleState == ECheckBoxState::Checked;
	return DataAsset->AreSegmentsNeighbors(DraggedSegmentID, IfDraggingOverTop, DroppedOverSegmentID, ArchivedVisible);
	
}

void FAVSTab::RemovePendingFromSegment() const
{
	DataAsset->RemovePendingElement(PendingElement.SegmentID, PendingElement.Element);
}

void FAVSTab::ResetPendingElement()
{
	PendingElement.Reset();
}

void FAVSTab::RefreshSegmentWidget(int32 SegmentID)
{
	TSharedPtr<FAVSSegment>* Segment = SegmentWidgets.Find(SegmentID);
	if(Segment->IsValid())
	{
		Segment->Get()->RecreateListView();
	}
}

void FAVSTab::ArchiveSegment(const int32 SegmentID)
{
	if(DataAsset->ArchiveSegment(SegmentID))
	{
		if(DataAsset->IsArchivedVisible)
		{
		}else
		{
			if (TSharedPtr<FAVSSegment>* SegmentWidgetPtr = SegmentWidgets.Find(SegmentID))
			{
				if (SegmentWidgetPtr->IsValid())
				{
					ScrollBoxSegments->RemoveSlot(SegmentWidgetPtr->ToSharedRef());
				}
			}
		}
		
		SaveAsset();
	}
}

void FAVSTab::UnarchiveSegment(const int32 SegmentID)
{
	if(DataAsset->UnarchiveSegment(SegmentID))
	{
		SaveAsset();
	}
}

bool FAVSTab::IsArchived(const int32 SegmentID)
{
	return DataAsset->IsArchived(SegmentID);
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION
#undef LOCTEXT_NAMESPACE