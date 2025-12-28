#include "UMGReflectorTree.h"

#include "UMGReflectorItem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Widget.h"


void SUMGReflectorTree::Construct(const FArguments& InArgs)
{
	CreateInstanceDetailsView();

	ChildSlot[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(0.6f)
		.Padding(2.0f, 1.0f, 2.0f, 1.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(5.f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Center)
				[
					SNew(SButton)
					.Text(NSLOCTEXT("UMGReflector", "RefreshButton", "Refresh"))
					.OnClicked(this, &SUMGReflectorTree::RefreshButtonClicked)
				]
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(UMGTreeViewSlate, STreeView<TSharedPtr<FUMGReflectorItem>>)
					.TreeItemsSource(&UMGRootItems)
					.OnGenerateRow(this, &SUMGReflectorTree::OnGenerateRow)
					.OnGetChildren(this, &SUMGReflectorTree::OnGetChildren)
					.OnSelectionChanged(this, &SUMGReflectorTree::OnSelectionChanged)
					.HeaderRow(
						SNew(SHeaderRow)
						+ SHeaderRow::Column("Name")
						.DefaultLabel(NSLOCTEXT("UMGReflector", "Name", "WidgetName"))
						.FillWidth(0.6f)
						+ SHeaderRow::Column("Type")
						.DefaultLabel(NSLOCTEXT("UMGReflector", "Type", "WidgetType"))
						.FillWidth(0.4f)
					)
				]
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.4f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			[
				PropertyViewPtr->AsShared()
			]
		]
	];

	FEditorDelegates::PostPIEStarted.AddSP(this, &SUMGReflectorTree::OnPostPIEModelStarted);
	FEditorDelegates::EndPIE.AddSP(this, &SUMGReflectorTree::OnEndPIEModel);

	UpdateWidgetTree();
}

SUMGReflectorTree::~SUMGReflectorTree()
{
	FEditorDelegates::PostPIEStarted.RemoveAll(this);
	FEditorDelegates::EndPIE.RemoveAll(this);
}

TSharedRef<ITableRow> SUMGReflectorTree::OnGenerateRow(TSharedPtr<FUMGReflectorItem> InItem, const TSharedRef<STableViewBase>& InOwnerTable)
{
	//return SNew(STableRow<TSharedPtr<FUMGReflectorItem>>, InOwnerTable);
	return SNew(STableRow<TSharedPtr<FUMGReflectorItem>>, InOwnerTable)
		[
			FUMGReflectorItem::GenerateTableRowForItem(InItem)
		];
}

void SUMGReflectorTree::OnGetChildren(TSharedPtr<FUMGReflectorItem> InItem, TArray<TSharedPtr<FUMGReflectorItem>>& OutChildren)
{
	OutChildren = InItem->GetChildrenData();
}

void SUMGReflectorTree::OnSelectionChanged(TSharedPtr<FUMGReflectorItem> InItem, ESelectInfo::Type SelectionType)
{
	if (InItem.IsValid() == false) return;
	UE_LOG(LogTemp, Log, TEXT("OnSelectionChanged: %s"), *InItem->GetTypeName());


	TArray<UObject*> SelectedWidgetObjects;
	TWeakObjectPtr<const UWidget> CurItem = InItem->GetWidget();
	TSharedPtr<SWidget> Widget = CurItem->GetCachedWidget();
	if (Widget.IsValid())
	{
		TSharedPtr<FReflectionMetaData> ReflectinMetaData = Widget->GetMetaData<FReflectionMetaData>();
		if (ReflectinMetaData.IsValid())
		{
			if (UObject* SourceObject = ReflectinMetaData->SourceObject.Get())
			{
				SelectedWidgetObjects.Add(SourceObject);
			}
		}
	}
	PropertyViewPtr->SetObjects(SelectedWidgetObjects);
}

void SUMGReflectorTree::UpdateWidgetTree()
{
	UMGRootItems.Reset();

	UWorld* PIEWorld = nullptr;
	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE)
		{
			PIEWorld = Context.World();
			break;
		}
	}

	if (!IsValid(PIEWorld) || !PIEWorld->IsGameWorld())
	{
		const TSharedPtr<FUMGReflectorItem> Item = MakeShared<FUMGReflectorItem>(nullptr, TEXT("请在PIE模式下运行您的游戏！"));
		UMGRootItems.Add(Item);
		UMGTreeViewSlate->RequestTreeRefresh();
		return;
	}

	FindAllUserWidget(PIEWorld, UMGRootItems);
	UMGTreeViewSlate->RequestTreeRefresh();
}


void SUMGReflectorTree::FindAllUserWidget(const UWorld* InWorld, TArray<TSharedPtr<FUMGReflectorItem>>& OutAllUserWidget)
{
	if (!IsValid(InWorld)) return;

	for (TObjectIterator<UUserWidget> It; It; ++It)
	{
		UUserWidget* RootUserWidget = *It;
		if (IsValid(RootUserWidget) && RootUserWidget->GetWorld() == InWorld && RootUserWidget->IsInViewport())
		{
			FString WidgetName = RootUserWidget->GetName();
			TSharedPtr<SWidget> CachedWidget = RootUserWidget->GetCachedWidget();
			if (CachedWidget.IsValid())
			{
				TSharedPtr<FUMGReflectorItem> Item = MakeShared<FUMGReflectorItem>(RootUserWidget, WidgetName);
				OutAllUserWidget.Add(Item);
				BuildUMGWidgetTree(RootUserWidget, CachedWidget, Item);
			}
			else
			{
				TSharedPtr<FUMGReflectorItem> Item = MakeShared<FUMGReflectorItem>(nullptr, FString::Printf(TEXT("Widget '%s' has no CachedWidget "), *WidgetName));
				OutAllUserWidget.Add(Item);
			}
		}
	}
}

void SUMGReflectorTree::BuildUMGWidgetTree(const UUserWidget* InWBPWidget, const TSharedPtr<SWidget>& InCurrentWidget, const TSharedPtr<FUMGReflectorItem>& InParent)
{
	if (!InCurrentWidget.IsValid()) return;

	const FChildren* Children = InCurrentWidget->GetChildren();
	if (Children == nullptr) return;

	for (int32 i = 0; i < Children->Num(); ++i)
	{
		TSharedPtr<const SWidget> ConstChildWidget = Children->GetChildAt(i);
		if (!ConstChildWidget.IsValid()) continue;

		TSharedPtr<SWidget> ChildWidget = ConstCastSharedPtr<SWidget>(ConstChildWidget);
		FString WidgetName = GetUMGReflectorWidgetName(InWBPWidget, ChildWidget);
		if (WidgetName.IsEmpty())
		{
			WidgetName = ChildWidget->GetTypeAsString();
		}

		UE_LOG(LogTemp, Log, TEXT("BuildUMGWidgetTree: %s"), *InWBPWidget->WidgetTree->GetOuter()->GetName());
		UWidget* Widget = const_cast<UUserWidget*>(InWBPWidget)->GetWidgetHandle(ChildWidget.ToSharedRef());
		if (!IsValid(Widget)) continue;
		TSharedPtr<FUMGReflectorItem> ChildItem = MakeShared<FUMGReflectorItem>(Widget, WidgetName);
		InParent->AddChildNodeItem(ChildItem);

		UUserWidget* CurUserWidget = Cast<UUserWidget>(const_cast<UUserWidget*>(InWBPWidget)->GetWidgetHandle(ChildWidget.ToSharedRef()));
		if (IsValid(CurUserWidget) && InWBPWidget->WidgetTree != CurUserWidget->WidgetTree)
		{
			UE_LOG(LogTemp, Log, TEXT("GetWidgetHandle: U%s"), *CurUserWidget->GetClass()->GetName());
			BuildUMGWidgetTree(CurUserWidget, ChildWidget, ChildItem);
			continue;
		}
		BuildUMGWidgetTree(InWBPWidget, ChildWidget, ChildItem);
	}
}

FString SUMGReflectorTree::GetUMGReflectorWidgetName(const UUserWidget* InWidget, const TSharedPtr<SWidget> InSlateWidget)
{
	if (!IsValid(InWidget) || !InSlateWidget.IsValid()) return FString();

	// for (TFieldIterator<FObjectProperty> PropertyIt(InWidget->GetClass()); PropertyIt; ++PropertyIt)
	// {
	// 	const FObjectProperty* ObjectProperty = *PropertyIt;
	// 	if (ObjectProperty && ObjectProperty->PropertyClass->IsChildOf(UWidget::StaticClass()))
	// 	{
	// 		UWidget* Widget = Cast<UWidget>(ObjectProperty->GetObjectPropertyValue_InContainer(InWidget));
	// 		if (Widget && Widget->GetCachedWidget().IsValid() && Widget->GetCachedWidget() == InSlateWidget)
	// 		{
	// 			UE_LOG(LogTemp, Log, TEXT("GetUMGReflectorWidgetName ObjectProperty->GetName(): U%s"), *ObjectProperty->GetName());
	// 			return ObjectProperty->GetName();
	// 		}
	// 	}
	// }

	if (InWidget->WidgetTree)
	{
		if (UWidget* Widget = const_cast<UUserWidget*>(InWidget)->GetWidgetHandle(InSlateWidget.ToSharedRef()))
		{
			return Widget->GetName();
		}
	}


	// FName WidgetTag = InSlateWidget->GetTag();
	// if (WidgetTag != NAME_None)
	// {
	// 	return WidgetTag.ToString();
	// }

	return FString();
}

void SUMGReflectorTree::OnPostPIEModelStarted(bool bIsSimulating)
{
	UpdateWidgetTree();
}

void SUMGReflectorTree::OnEndPIEModel(bool bIsSimulating)
{
	UpdateWidgetTree();
}

FReply SUMGReflectorTree::RefreshButtonClicked()
{
	UpdateWidgetTree();
	return FReply::Handled();
}

void SUMGReflectorTree::CreateInstanceDetailsView()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.bAllowSearch = true;
		DetailsViewArgs.bShowOptions = true;
		DetailsViewArgs.bAllowMultipleTopLevelObjects = false;
		DetailsViewArgs.bAllowFavoriteSystem = true;
		DetailsViewArgs.bShowObjectLabel = false;
		DetailsViewArgs.bHideSelectionTip = true;
	}
	TSharedRef<IDetailsView> PropertyView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	PropertyViewPtr = PropertyView;
}
