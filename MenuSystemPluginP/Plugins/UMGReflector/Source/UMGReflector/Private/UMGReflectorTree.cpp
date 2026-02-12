#include "UMGReflectorTree.h"

#include "UMGReflectorItem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "Widgets/Input/SSearchBox.h"

#define LOCTEXT_NAMESPACE "UMGReflector"

void SUMGReflectorTree::Construct(const FArguments& InArgs)
{
	CreateInstanceDetailsView();

	ChildSlot[
		SNew(SVerticalBox)

		// 1. === 顶部工具栏 ===
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.f)
		[
			SNew(SHorizontalBox)
			// 1.1 刷新按钮
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 5.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("RefreshButton", "Refresh"))
				.ToolTipText(LOCTEXT("RefreshButtonTooltip", "Manually refresh the widget tree"))
				.OnClicked(this, &SUMGReflectorTree::OnRefreshButtonClicked)
			]
			// 1.2 自动刷新复选框
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 10.f, 0.f)
			[
				SAssignNew(AutoRefreshCheckBox, SCheckBox)
				.IsChecked(ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &SUMGReflectorTree::OnAutoRefreshChanged)
				.ToolTipText(LOCTEXT("AutoRefreshTooltip", "Automatically refresh widget tree every second"))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AutoRefresh", "Auto Refresh"))
				]
			]
			// 1.3 搜索框
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SAssignNew(SearchBox, SSearchBox)
				.HintText(LOCTEXT("SearchHint", "Search widgets..."))
				.ToolTipText(LOCTEXT("SearchTooltip", "Filter widgets by name or type"))
				.OnTextChanged(this, &SUMGReflectorTree::OnSearchTextChanged)
			]
			// 1.4 清空搜索按钮
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(5.f, 0.f, 0.f, 0.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ClearSearch", "X"))
				.ToolTipText(LOCTEXT("ClearSearchTooltip", "Clear search filter"))
				.OnClicked(this, &SUMGReflectorTree::OnClearSearchClicked)
			]
		]

		// 2. === 主内容区域 ===
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SNew(SHorizontalBox)
			// 2.1 左侧Widget树
			+ SHorizontalBox::Slot()
			.FillWidth(0.6f)
			.Padding(2.0f, 1.0f, 2.0f, 1.0f)
			[
				SNew(SScrollBox)
				+ SScrollBox::Slot()
				[
					SAssignNew(UMGTreeViewSlate, STreeView<TSharedPtr<FUMGReflectorItem>>)
					.TreeItemsSource(&FilteredRootItems)
					.OnGenerateRow(this, &SUMGReflectorTree::OnGenerateRow)
					.OnGetChildren(this, &SUMGReflectorTree::OnGetChildren)
					.OnSelectionChanged(this, &SUMGReflectorTree::OnSelectionChanged)
					.HeaderRow(
						SNew(SHeaderRow)
						// Widget名称列
						+ SHeaderRow::Column("Name")
						.DefaultLabel(LOCTEXT("ColumnName", "Widget Name"))
						.FillWidth(0.35f)

						// 类型列
						+ SHeaderRow::Column("Type")
						.DefaultLabel(LOCTEXT("ColumnType", "Type"))
						.FillWidth(0.25f)

						// 可见性列
						+ SHeaderRow::Column("Visibility")
						.DefaultLabel(LOCTEXT("ColumnVisibility", "Visibility"))
						.FillWidth(0.15f)

						// 位置列
						+ SHeaderRow::Column("Position")
						.DefaultLabel(LOCTEXT("ColumnPosition", "Position"))
						.FillWidth(0.15f)

						// 大小列
						+ SHeaderRow::Column("Size")
						.DefaultLabel(LOCTEXT("ColumnSize", "Size"))
						.FillWidth(0.1f)
					)
				]
			]
			// 2.2 右侧属性面板
			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			[
#if WITH_EDITOR
				PropertyViewPtr.ToSharedRef()
#else
				SNew(STextBlock)
				.Text(LOCTEXT("NoPropertyView", "Property view not available"))
#endif
			]
		]
	];

	FEditorDelegates::PostPIEStarted.AddSP(this, &SUMGReflectorTree::OnPostPIEStarted);
	FEditorDelegates::EndPIE.AddSP(this, &SUMGReflectorTree::OnEndPIE);

	UpdateWidgetTree();
}

SUMGReflectorTree::~SUMGReflectorTree()
{
	StopAutoRefreshTimer();
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
	if (InItem.IsValid())
	{
		OutChildren = InItem->GetChildrenData();
	}
}

void SUMGReflectorTree::OnSelectionChanged(TSharedPtr<FUMGReflectorItem> InItem, ESelectInfo::Type SelectionType)
{
	if (!InItem.IsValid() || InItem->GetWidget() == nullptr) return;
	UE_LOG(LogTemp, Log, TEXT("OnSelectionChanged: %s"), *InItem->GetTypeName());
	
#if WITH_EDITOR
	TArray<UObject*> SelectedWidgetObjects;
	TWeakObjectPtr<const UWidget> CurItem = InItem->GetWidget();
	TSharedPtr<SWidget> Widget = CurItem->GetCachedWidget();
	if (Widget.IsValid())
	{
		// 尝试从ReflectionMetaData获取SourceObject
		TSharedPtr<FReflectionMetaData> ReflectinMetaData = Widget->GetMetaData<FReflectionMetaData>();
		if (ReflectinMetaData.IsValid())
		{
			if (UObject* SourceObject = ReflectinMetaData->SourceObject.Get())
			{
				SelectedWidgetObjects.Add(SourceObject);
			}
		}
		// 如果没有找到，尝试直接使用UWidget
		if (SelectedWidgetObjects.Num() == 0)
		{
			if (UObject* WidgetObject = const_cast<UWidget*>(CurItem.Get()))
			{
				SelectedWidgetObjects.Add(WidgetObject);
			}
		}
	}
	PropertyViewPtr->SetObjects(SelectedWidgetObjects);
	UE_LOG(LogTemp, Log, TEXT("Selected Widget: %s (%s)"), *InItem->GetDisplayName(), *InItem->GetTypeName());
#endif
}

FReply SUMGReflectorTree::OnRefreshButtonClicked()
{
	UpdateWidgetTree();
	return FReply::Handled();
}

void SUMGReflectorTree::OnSearchTextChanged(const FText& InText)
{
	CurrentSearchText = InText.ToString();

	FilteredRootItems.Reset();
	
	if (CurrentSearchText.IsEmpty())
	{
		FilteredRootItems = UMGRootItems;
		for (auto& Item : FilteredRootItems)
		{
			CollapseItemRecursively(Item);  // 折叠所有
		}
		FilteredRootItems = UMGRootItems;	// 没有搜索条件，显示所有项
	}
	else
	{
		FilterTreeItems(UMGRootItems, FilteredRootItems, CurrentSearchText);		// 应用过滤
		if (UMGTreeViewSlate.IsValid())
		{
			for (auto& Item : FilteredRootItems)
			{
				ExpandMatchingItems(Item, CurrentSearchText);	// 递归展开所有包含匹配项的节点
			}
		}
	}

	if (UMGTreeViewSlate.IsValid())
	{
		UMGTreeViewSlate->RequestTreeRefresh();
		// 如果有搜索结果，自动展开所有
		if (!CurrentSearchText.IsEmpty() && FilteredRootItems.Num() > 0)
		{
			UMGTreeViewSlate->SetItemExpansion(FilteredRootItems[0], true);
		}
	}
}

void SUMGReflectorTree::OnAutoRefreshChanged(ECheckBoxState NewState)
{
	bAutoRefreshEnabled = (NewState == ECheckBoxState::Checked);

	if (bAutoRefreshEnabled)
	{
		StartAutoRefreshTimer();
	}
	else
	{
		StopAutoRefreshTimer();
	}
}

FReply SUMGReflectorTree::OnClearSearchClicked()
{
	if (SearchBox.IsValid())
	{
		SearchBox->SetText(FText::GetEmpty());
	}
	return FReply::Handled();
}

void SUMGReflectorTree::ExpandMatchingItems(const TSharedPtr<FUMGReflectorItem>& Item, const FString& SearchString)
{
	if (!Item.IsValid() || !UMGTreeViewSlate.IsValid())
		return;

	bool bCurrentMatches = FilterWidgetItem(Item, SearchString);
	bool bHasMatchingChildren = false;
	for (const TSharedPtr<FUMGReflectorItem>& Child : Item->GetChildrenData())
	{
		if (!Child.IsValid()) continue;

		if (FilterWidgetItem(Child, SearchString) || HasMatchingDescendants(Child, SearchString))
		{
			bHasMatchingChildren = true;
			// 递归展开子项
			ExpandMatchingItems(Child, SearchString);
		}
	}
	if (bCurrentMatches || bHasMatchingChildren)
	{
		UMGTreeViewSlate->SetItemExpansion(Item, true);
	}
}

void SUMGReflectorTree::CollapseItemRecursively(const TSharedPtr<FUMGReflectorItem>& Item)
{
	if (!Item.IsValid() || !UMGTreeViewSlate.IsValid())
		return;

	// 折叠当前项
	UMGTreeViewSlate->SetItemExpansion(Item, false);
    
	// 递归折叠所有子项
	for (const TSharedPtr<FUMGReflectorItem>& Child : Item->GetChildrenData())
	{
		CollapseItemRecursively(Child);
	}
}

bool SUMGReflectorTree::FilterWidgetItem(const TSharedPtr<FUMGReflectorItem>& InItem, const FString& SearchString) const
{
	if (!InItem.IsValid() || SearchString.IsEmpty())
	{
		return true;
	}

	// 检查名称匹配
	if (InItem->GetDisplayName().Contains(SearchString, ESearchCase::IgnoreCase))
	{
		return true;
	}
	
	// 检查类型匹配
	if (InItem->GetTypeName().Contains(SearchString, ESearchCase::IgnoreCase))
	{
		return true;
	}
	
	return false;
}

void SUMGReflectorTree::FilterTreeItems(const TArray<TSharedPtr<FUMGReflectorItem>>& SourceItems, TArray<TSharedPtr<FUMGReflectorItem>>& FilteredItems, const FString& SearchString) const
{
	for (auto& Item : SourceItems)
	{
		if (!Item.IsValid()) continue;
		
		bool bMatches = FilterWidgetItem(Item, SearchString);
		bool bHasMatchingChildren = HasMatchingDescendants(Item, SearchString);
		
		// 如果当前项或子项匹配，添加到过滤结果
		if (bMatches || bHasMatchingChildren)
		{
			FilteredItems.Add(Item);
		}
	}
}

bool SUMGReflectorTree::HasMatchingDescendants(const TSharedPtr<FUMGReflectorItem>& Item, const FString& SearchString) const
{
	if (!Item.IsValid()) return false;

	for (const TSharedPtr<FUMGReflectorItem>& Child : Item->GetChildrenData())
	{
		if (!Child.IsValid()) continue;

		// 检查子节点
		if (FilterWidgetItem(Child, SearchString))
			return true;

		// 🔑 递归检查子节点的后代
		if (HasMatchingDescendants(Child, SearchString))
			return true;
	}

	return false;
}

void SUMGReflectorTree::StartAutoRefreshTimer()
{
	if (GEditor)
	{
		GEditor->GetTimerManager()->SetTimer(
		AutoRefreshTimerHandle,
		FTimerDelegate::CreateSP(this, &SUMGReflectorTree::OnAutoRefreshTimer),
		AutoRefreshInterval,
		true
		);
	}
}

void SUMGReflectorTree::StopAutoRefreshTimer()
{
	if (GEditor && AutoRefreshTimerHandle.IsValid())
	{
		GEditor->GetTimerManager()->ClearTimer(AutoRefreshTimerHandle);
		AutoRefreshTimerHandle.Invalidate();
	}
}

void SUMGReflectorTree::OnAutoRefreshTimer()
{
	UpdateWidgetTree();
}

void SUMGReflectorTree::UpdateWidgetTree()
{
	UMGRootItems.Reset();
	FilteredRootItems.Reset();

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
		const TSharedPtr<FUMGReflectorItem> WarningItem = MakeShared<FUMGReflectorItem>(nullptr, LOCTEXT("NoPIEWarning", "Please run your game in PIE mode!").ToString());
		UMGRootItems.Add(WarningItem);
		FilteredRootItems = UMGRootItems;
		if (UMGTreeViewSlate.IsValid())
		{
			UMGTreeViewSlate->RequestTreeRefresh();
		}
		return;
	}

	FindAllUserWidget(PIEWorld, UMGRootItems);

	// 应用搜索过滤
	if (CurrentSearchText.IsEmpty())
	{
		FilteredRootItems = UMGRootItems;
	}
	else
	{
		FilterTreeItems(UMGRootItems, FilteredRootItems, CurrentSearchText);
	}

	// 刷新树视图
	if (UMGTreeViewSlate.IsValid())
	{
		UMGTreeViewSlate->RequestTreeRefresh();
	}
}


void SUMGReflectorTree::FindAllUserWidget(const UWorld* InWorld, TArray<TSharedPtr<FUMGReflectorItem>>& OutAllUserWidget)
{
	if (!IsValid(InWorld))
	{
		UE_LOG(LogTemp, Warning, TEXT("FindAllUserWidget: Invalid World"));
		return;
	}

	int32 WidgetCount = 0;
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
				WidgetCount++;
			}
			else
			{
				TSharedPtr<FUMGReflectorItem> Item = MakeShared<FUMGReflectorItem>(nullptr, FString::Printf(TEXT("Widget '%s' has no CachedWidget "), *WidgetName));
				OutAllUserWidget.Add(Item);
			}
		}
	}
	UE_LOG(LogTemp, Log, TEXT("FindAllUserWidget: Found %d widgets in viewport"), WidgetCount);
}

void SUMGReflectorTree::BuildUMGWidgetTree(const UUserWidget* InWBPWidget, const TSharedPtr<SWidget>& InCurrentWidget, const TSharedPtr<FUMGReflectorItem>& InParent)
{
	if (!InCurrentWidget.IsValid() || !InParent.IsValid())
	{
		return;
	}

	const FChildren* Children = InCurrentWidget->GetChildren();
	if (Children == nullptr) return;

	for (int32 i = 0; i < Children->Num(); ++i)
	{
		TSharedPtr<const SWidget> ConstChildWidget = Children->GetChildAt(i);
		if (!ConstChildWidget.IsValid()) continue;

		TSharedPtr<SWidget> ChildWidget = ConstCastSharedPtr<SWidget>(ConstChildWidget);
		// 获取Widget名称
		FString WidgetName = GetUMGWidgetName(InWBPWidget, ChildWidget);
		if (WidgetName.IsEmpty())
		{
			WidgetName = ChildWidget->GetTypeAsString();
		}

		UE_LOG(LogTemp, Log, TEXT("BuildUMGWidgetTree: %s"), *InWBPWidget->WidgetTree->GetOuter()->GetName());
		// 获取UWidget实例
		UWidget* Widget = const_cast<UUserWidget*>(InWBPWidget)->GetWidgetHandle(ChildWidget.ToSharedRef());
		if (!IsValid(Widget)) continue;

		// 创建节点
		TSharedPtr<FUMGReflectorItem> ChildItem = MakeShared<FUMGReflectorItem>(Widget, WidgetName);
		InParent->AddChildNodeItem(ChildItem);

		// 检查是否是嵌套的UserWidget
		UUserWidget* CurUserWidget = Cast<UUserWidget>(const_cast<UUserWidget*>(InWBPWidget)->GetWidgetHandle(ChildWidget.ToSharedRef()));
		if (IsValid(CurUserWidget) && InWBPWidget->WidgetTree != CurUserWidget->WidgetTree)
		{
			// 递归处理嵌套UserWidget
			UE_LOG(LogTemp, Log, TEXT("GetWidgetHandle: U%s"), *CurUserWidget->GetClass()->GetName());
			BuildUMGWidgetTree(CurUserWidget, ChildWidget, ChildItem);
			
		}
		else
		{
			// 继续处理当前层级的子Widget
			BuildUMGWidgetTree(InWBPWidget, ChildWidget, ChildItem);
		}
	}
}

FString SUMGReflectorTree::GetUMGWidgetName(const UUserWidget* InWidget, const TSharedPtr<SWidget> InSlateWidget)
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

	// 从WidgetTree获取名称
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

void SUMGReflectorTree::OnPostPIEStarted(bool bIsSimulating)
{
	UpdateWidgetTree();
}

void SUMGReflectorTree::OnEndPIE(bool bIsSimulating)
{
	StopAutoRefreshTimer();
	if (AutoRefreshCheckBox.IsValid())
	{
		AutoRefreshCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
	}
	UpdateWidgetTree();
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
		DetailsViewArgs.bShowObjectLabel = true;
		DetailsViewArgs.bHideSelectionTip = true;
		DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	}
	TSharedRef<IDetailsView> PropertyView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	PropertyViewPtr = PropertyView;
}

#undef LOCTEXT_NAMESPACE