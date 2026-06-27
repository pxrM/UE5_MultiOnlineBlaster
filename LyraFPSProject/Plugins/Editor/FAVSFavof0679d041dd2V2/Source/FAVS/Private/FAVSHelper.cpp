// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#include "FAVSHelper.h"
#include "FAVS/Data/FAVSDataAsset.h"
#include "FAVSStructs.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetTypeActions_Base.h"
#include "ClassIconFinder.h"
#include "FAVSEvents.h"
#include "FAVSStyle.h"
#include "UObject/Class.h"

#define LOCTEXT_NAMESPACE "FAVS"
TMap<const UClass*, FColor> FAVSHelper::CachedColors;
static const FText EntryTextFormatted = LOCTEXT("AddTo", "Add to {0}");
static const FText TooltipText_BP = LOCTEXT("AddToTooltip_BP", "Adds the currently open blueprint to FAVS");
static const FText TooltipText_Level = LOCTEXT("AddToTooltip_Level", "Adds the currently open level to FAVS");
static const FText TooltipText_Anim = LOCTEXT("AddToTooltip_Anim", "Adds the currently open asset to FAVS");


FColor FAVSHelper::GetIconColor(const UClass* AssetClass)
{
	if (!AssetClass)
	{
		return FColor::White;
	}
	if (const FColor* FoundColor = CachedColors.Find(AssetClass))
	{
		return *FoundColor;
	}
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	IAssetTools& AssetTools = AssetToolsModule.Get();
	TWeakPtr<IAssetTypeActions> WeakAction = AssetTools.GetAssetTypeActionsForClass(AssetClass);
	if (TSharedPtr<IAssetTypeActions> AssetAction = WeakAction.Pin())
	{
		FColor Kolor = AssetAction->GetTypeColor();
		Kolor.A = ColorOpacity;
		CachedColors.Add(AssetClass, Kolor);
		return Kolor;
	}

	return FColor::White;
}

void FAVSHelper::AddToolbarButton_Asset(FAVSDataAsset* DataAsset, const FName& Toolbar)
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu(Toolbar);
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("FAVS"); 
	
	Section.AddEntry(FToolMenuEntry::InitComboButton(
		"FAVS.DropdownButton",
		FUIAction(),
		FOnGetContent::CreateLambda([DataAsset]() -> TSharedRef<SWidget>
		{
			FMenuBuilder MenuBuilder(true, nullptr);
					
			for (int i = 0; i < DataAsset->SegmentsOrder.Num(); ++i)
			{
				int32 SegmentID = DataAsset->SegmentsOrder[i];

				if(DataAsset->ArchivedSegments.Contains(SegmentID) && !DataAsset->IsArchivedVisible) continue;

				FAVSSegmentData* SegmentPtr = DataAsset->Segments.Find(SegmentID);
				if (!SegmentPtr)
				{
					continue;
				}
				FText EntryText = FText::Format(EntryTextFormatted, SegmentPtr->SegmentName);
				
				
				MenuBuilder.AddMenuEntry(
					EntryText,
					TooltipText_BP,
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([SegmentID, DataAsset]()
					{
						UObject* EditedAsset = GetEditedAsset();
						const FSoftObjectPath SoftPath(EditedAsset);
						const FString DisplayName = EditedAsset->GetName();
						EAssetType AssetType = GetAssetTypeFromAssetData(EditedAsset);
													
						const FAssetData TempAssetData(EditedAsset);
						const UClass* AssetClassForIcon = FClassIconFinder::GetIconClassForAssetData(TempAssetData);
						const FColor IconColor = GetIconColor(EditedAsset->GetClass());
						FAVSAssetData AssetData(DisplayName, SoftPath, AssetType, AssetClassForIcon, IconColor);
						
						FAVSEvents::OnAssetAdded.Broadcast(AssetData, SegmentID);
					}))
				);
			}
			return MenuBuilder.MakeWidget();
		}),
		FText::FromString(TEXT("FAVS")),
		 FText::GetEmpty(),
		FSlateIcon(FAVSStyle::Get().GetStyleSetName(), "FAVS.StarTint"),
		false
	));
}


void FAVSHelper::AddToolbarButton_Level(FAVSDataAsset* DataAsset)
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("FAVS");
	Section.AddEntry(FToolMenuEntry::InitComboButton(
		"FAVS.DropdownButton",
		FUIAction(),
		FOnGetContent::CreateLambda([DataAsset]() -> TSharedRef<SWidget>
		{
			FMenuBuilder MenuBuilder(true, nullptr);
					
			for (int i = 0; i < DataAsset->SegmentsOrder.Num(); ++i)
			{
				int32 SegmentID = DataAsset->SegmentsOrder[i];
				
				if(DataAsset->ArchivedSegments.Contains(SegmentID) && !DataAsset->IsArchivedVisible) continue;

				FAVSSegmentData* SegmentPtr = DataAsset->Segments.Find(SegmentID);
				if (!SegmentPtr)
				{
					continue;
				}
				FText EntryText = FText::Format(EntryTextFormatted, SegmentPtr->SegmentName);
				
				MenuBuilder.AddMenuEntry(
					EntryText, 
					TooltipText_Level,
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([SegmentID, DataAsset]()
					{
						UWorld* CurrentWorld = GEditor->GetEditorWorldContext().World();
						if (!CurrentWorld) return;
						
						const FString MapName = CurrentWorld->GetOutermost()->GetName();
						
						if (MapName.StartsWith(TEXT("/Temp/Untitled")))
						{
							return;
						}
						const FString LevelPath = CurrentWorld->GetOutermost()->GetName();
						const FString DisplayName = FPackageName::GetShortName(LevelPath);
						FString FullSoftPath = LevelPath + TEXT(".") + DisplayName;
						FSoftObjectPath AssetSoftPath(FullSoftPath);
						UClass* AssetClass = UWorld::StaticClass();
						const FColor IconColor = GetIconColor(AssetClass);
						FAVSAssetData AssetData(DisplayName, AssetSoftPath, EAssetType::Level, AssetClass, IconColor);

						FAVSEvents::OnAssetAdded.Broadcast(AssetData, SegmentID);
					}))
				);
			}
			return MenuBuilder.MakeWidget();
		}),
		FText::FromString(TEXT("FAVS")),
		FText::GetEmpty(),
		FSlateIcon(FAVSStyle::Get().GetStyleSetName(), "FAVS.StarTint"),
		false
	));
	
}

void FAVSHelper::AddToolbarButton_Sequencer(FToolBarBuilder& ToolbarBuilder, FAVSDataAsset* DataAsset)
{
	ToolbarBuilder.AddSeparator();

	ToolbarBuilder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateLambda([DataAsset]() -> TSharedRef<SWidget>
		{
			FMenuBuilder MenuBuilder(true, nullptr);
			for (int i = 0; i < DataAsset->SegmentsOrder.Num(); ++i)
			{
				int32 SegmentID = DataAsset->SegmentsOrder[i];
				
				if(DataAsset->ArchivedSegments.Contains(SegmentID) && !DataAsset->IsArchivedVisible) continue;

				FAVSSegmentData* SegmentPtr = DataAsset->Segments.Find(SegmentID);
				if (!SegmentPtr)
				{
					continue;
				}
				FText EntryText = FText::Format(EntryTextFormatted, SegmentPtr->SegmentName);

				MenuBuilder.AddMenuEntry(
				
				EntryText, 
					TooltipText_Anim,
					FSlateIcon(),
					FUIAction(FExecuteAction::CreateLambda([SegmentID, DataAsset]()
					{
						UObject* EditedAsset = GetEditedAsset();
						const FSoftObjectPath SoftPath(EditedAsset);
						const FString DisplayName = EditedAsset->GetName();
						EAssetType AssetType = GetAssetTypeFromAssetData(EditedAsset);
						UClass* AssetClass = EditedAsset->GetClass();
						const FColor IconColor = GetIconColor(AssetClass);
						FAVSAssetData AssetData(DisplayName, SoftPath, AssetType, AssetClass, IconColor);

						FAVSEvents::OnAssetAdded.Broadcast(AssetData, SegmentID);
					}))
				);
			}

			return MenuBuilder.MakeWidget();
		}),
		FText::FromString(TEXT("FAVS")),
		FText::GetEmpty(),
		FSlateIcon(FAVSStyle::Get().GetStyleSetName(), "FAVS.StarTint"),
		false
	);
}

UObject* FAVSHelper::GetEditedAsset()
{
	if (!GEditor) return nullptr;

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem) return nullptr;

	TSharedPtr<SWindow> ActiveWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	bool IsSequencer = ActiveWindow->GetTitle().ToString().Contains(TEXT("Sequencer"));
	
	
	for (UObject* EditedAsset : AssetEditorSubsystem->GetAllEditedAssets())
	{
		if (!EditedAsset) continue;
	
		IAssetEditorInstance* EditorInstance = AssetEditorSubsystem->FindEditorForAsset(EditedAsset, false);
		if (!EditorInstance) continue;

		if(IsSequencer && EditorInstance->GetEditorName().ToString().Contains(TEXT("SequenceEditor")))
		{
			return EditedAsset;
		}
				
		FAssetEditorToolkit* Toolkit = static_cast<FAssetEditorToolkit*>(EditorInstance);
		
		if (Toolkit && Toolkit->GetTabManager().IsValid())
		{
			TSharedPtr<SDockTab> Tab = Toolkit->GetTabManager()->GetOwnerTab();
			if (Tab.IsValid() && Tab->IsForeground())
			{
				return EditedAsset;
			}
		}
	}

	return nullptr;
}

EAssetType FAVSHelper::GetAssetTypeFromAssetData(const FAssetData& AssetData)  
{
	const FName AssetClassName = AssetData.AssetClassPath.GetAssetName();
	FString ParentClass;
	AssetData.GetTagValue("ParentClass", ParentClass);
	UClass* AssetClass = AssetData.GetClass();

	if (!AssetClass)
	{
		return EAssetType::Unknown;
	}
	if (AssetClassName == "World")
	{
		return EAssetType::Level;
	}
	if (!AssetData.IsUAsset() && !AssetData.IsValid() && AssetData.PackageName.ToString().Contains("/"))
	{
		return EAssetType::Folder;
	}
		
	return EAssetType::Unknown;
}

bool FAVSHelper::ExtractMovedFolder(const FString& OldAssetPath, const FString& NewAssetPath, FString& OldFolder, FString& NewFolder)
{
	TArray<FString> OldSegments;
	TArray<FString> NewSegments;
	OldAssetPath.ParseIntoArray(OldSegments, TEXT("/"), true);
	NewAssetPath.ParseIntoArray(NewSegments, TEXT("/"), true);

	while (OldSegments.Num() > 0 && NewSegments.Num() > 0 && OldSegments.Last() == NewSegments.Last())
	{
		OldSegments.Pop();
		NewSegments.Pop();
	}

	OldFolder = FString::Join(OldSegments, TEXT("/"));
	NewFolder = FString::Join(NewSegments, TEXT("/"));

	if (!OldFolder.StartsWith(TEXT("/"))) OldFolder = TEXT("/") + OldFolder;
	if (!NewFolder.StartsWith(TEXT("/"))) NewFolder = TEXT("/") + NewFolder;

	return !(OldFolder == NewFolder);
}




EAssetType FAVSHelper::GetAssetTypeFromAssetData(const UObject* Asset)
{
	if (!Asset)
	{
		return EAssetType::Unknown;
	}
	if (const UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
	{

		
		switch (Blueprint->BlueprintType)
		{
			case BPTYPE_FunctionLibrary:
				return EAssetType::BlueprintFunctionLibrary;

			case BPTYPE_Interface:
				return EAssetType::BlueprintInterface;

			default:
				return EAssetType::Blueprint;
		}
	}
	if (Asset->IsA<UDataAsset>())
	{
		return EAssetType::DataAsset;
	}
		
	if (Asset->IsA<UStaticMesh>())
	{
		return EAssetType::StaticMesh;
	}
	if (Asset->IsA<UWorld>())
	{
		return EAssetType::Level;
	}
	
	if (Asset->IsA<UTexture>())
	{
		return EAssetType::Texture;
	}

	return EAssetType::Unknown;
	
}

#undef LOCTEXT_NAMESPACE