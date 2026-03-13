#include "UMGReflectorTree.h"

#include "UMGReflectorItem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Widget.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SSearchBox.h"

#if WITH_SLATE_DEBUGGING
#include "Debugging/SlateDebugging.h"
#endif

#if WITH_EDITOR
#include "Subsystems/AssetEditorSubsystem.h"
#endif

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
			// 1.0 拾取按钮
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.f, 0.f, 5.f, 0.f)
			[
				SAssignNew(PickButton, SButton)
				.Text(LOCTEXT("PickButton", "Pick"))
				.ToolTipText(LOCTEXT("PickButtonTooltip", "Click to enable widget picking mode.\nThen click on a widget in the game viewport to select it in the tree.\nPress ESC to cancel."))
				.OnClicked(this, &SUMGReflectorTree::OnPickButtonClicked)
			]
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

		// 1.5 拾取状态提示栏
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(5.f, 0.f)
		[
			SAssignNew(PickingStatusText, STextBlock)
			.ColorAndOpacity(FLinearColor::Yellow)
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
					.OnMouseButtonDoubleClick(this, &SUMGReflectorTree::OnItemDoubleClicked)
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
	if (bPickingMode)
	{
		SetPickingMode(false);
	}

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

void SUMGReflectorTree::OnItemDoubleClicked(TSharedPtr<FUMGReflectorItem> InItem)
{
#if WITH_EDITOR
	if (!InItem.IsValid() || InItem->GetWidget() == nullptr)
	{
		return;
	}

	const UWidget* Widget = InItem->GetWidget().Get();
	if (!IsValid(Widget))
	{
		return;
	}

	// 沿 Outer 链向上查找所属的 UUserWidget
	const UUserWidget* OwnerUserWidget = nullptr;
	if (const UUserWidget* AsUserWidget = Cast<UUserWidget>(Widget))
	{
		OwnerUserWidget = AsUserWidget;
	}
	else if (const UWidgetTree* WidgetTree = Cast<UWidgetTree>(Widget->GetOuter()))
	{
		OwnerUserWidget = Cast<UUserWidget>(WidgetTree->GetOuter());
	}

	if (!IsValid(OwnerUserWidget))
	{
		UE_LOG(LogTemp, Warning, TEXT("OnItemDoubleClicked: Cannot find owning UserWidget for '%s'"), *InItem->GetDisplayName());
		return;
	}

	// 通过 ClassGeneratedBy 获取 Blueprint 资产
	UBlueprint* Blueprint = Cast<UBlueprint>(OwnerUserWidget->GetClass()->ClassGeneratedBy);
	if (!IsValid(Blueprint))
	{
		UE_LOG(LogTemp, Warning, TEXT("OnItemDoubleClicked: '%s' is not a Blueprint class"), *OwnerUserWidget->GetClass()->GetName());
		return;
	}

	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetEditorSubsystem->OpenEditorForAsset(Blueprint);
		UE_LOG(LogTemp, Log, TEXT("OnItemDoubleClicked: Opened Blueprint '%s'"), *Blueprint->GetName());
	}
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

		// 递归检查子节点的后代
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

		if (!IsValid(RootUserWidget))
		{
			continue;
		}

		// 跳过 CDO 和 WidgetTree 模板实例
		if (RootUserWidget->HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
		{
			continue;
		}

		// 只收集通过 AddToViewport() 添加到视口的根 Widget
		// 编辑器 UMG 设计器预览不走 AddToViewport()，天然被排除
		// 子 Widget 由 BuildUMGWidgetTree 递归发现，不在这里重复收集
		if (!RootUserWidget->IsInViewport())
		{
			continue;
		}

		TSharedPtr<SWidget> CachedWidget = RootUserWidget->GetCachedWidget();
		if (!CachedWidget.IsValid())
		{
			continue;
		}

		FString WidgetName = RootUserWidget->GetName();
		TSharedPtr<FUMGReflectorItem> Item = MakeShared<FUMGReflectorItem>(RootUserWidget, WidgetName);
		OutAllUserWidget.Add(Item);

		BuildUMGWidgetTree(RootUserWidget, CachedWidget, Item);
		WidgetCount++;
	}
	UE_LOG(LogTemp, Log, TEXT("FindAllUserWidget: Found %d root viewport widgets"), WidgetCount);
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
		if (InWBPWidget->WidgetTree->GetOuter()->GetName().Contains("WBP_TaskPanel_C"))
		{
			UE_LOG(LogTemp, Log, TEXT("##BuildUMGWidgetTree: %s"), *ChildWidget->GetTypeAsString());
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
	if (bPickingMode)
	{
		SetPickingMode(false);
	}

	StopAutoRefreshTimer();
	if (AutoRefreshCheckBox.IsValid())
	{
		AutoRefreshCheckBox->SetIsChecked(ECheckBoxState::Unchecked);
	}
	UpdateWidgetTree();
}

// === 拾取功能实现 ===

bool SUMGReflectorTree::IsInPIE() const
{
	if (!GEngine)
	{
		return false;
	}

	for (const FWorldContext& Context : GEngine->GetWorldContexts())
	{
		if (Context.WorldType == EWorldType::PIE && IsValid(Context.World()) && Context.World()->IsGameWorld())
		{
			return true;
		}
	}
	return false;
}

FReply SUMGReflectorTree::OnPickButtonClicked()
{
	if (!IsInPIE())
	{
		return FReply::Handled();
	}

	SetPickingMode(!bPickingMode);
	return FReply::Handled();
}

void SUMGReflectorTree::SetPickingMode(bool bEnable)
{
	if (bPickingMode == bEnable)
	{
		return;
	}

	bPickingMode = bEnable;
	HoveredPickItem.Reset();
	bHasValidHoverTarget = false;
	PickingTreeRefreshTimer = 0.0f;

	if (bPickingMode)
	{
		// 强制刷新树数据 — 动态UI（如Layer系统的MountPoint子节点）
		// 可能在上次刷新后被重建，导致TWeakObjectPtr失效
		UpdateWidgetTree();

		// 注册全局输入监听
		if (FSlateApplication::IsInitialized())
		{
			GlobalMouseDownHandle = FSlateApplication::Get().OnApplicationMousePreInputButtonDownListener().AddSP(
				this, &SUMGReflectorTree::OnGlobalMouseButtonDown);
			GlobalKeyDownHandle = FSlateApplication::Get().OnApplicationPreInputKeyDownListener().AddSP(
				this, &SUMGReflectorTree::OnGlobalKeyDown);
		}

#if WITH_SLATE_DEBUGGING
		// 注册绘制高亮回调
		PaintDebugElementsHandle = FSlateDebugging::PaintDebugElements.AddSP(
			this, &SUMGReflectorTree::OnPaintDebugElements);
#endif

		// 更新UI
		if (PickButton.IsValid())
		{
			PickButton->SetContent(
				SNew(STextBlock)
				.Text(LOCTEXT("PickButtonActive", "Picking..."))
			);
		}
		if (PickingStatusText.IsValid())
		{
			PickingStatusText->SetText(LOCTEXT("PickingStatus", "Move cursor over game viewport and click to pick a widget. Press ESC to cancel."));
		}
	}
	else
	{
		// 移除全局输入监听
		if (FSlateApplication::IsInitialized())
		{
			if (GlobalMouseDownHandle.IsValid())
			{
				FSlateApplication::Get().OnApplicationMousePreInputButtonDownListener().Remove(GlobalMouseDownHandle);
				GlobalMouseDownHandle.Reset();
			}
			if (GlobalKeyDownHandle.IsValid())
			{
				FSlateApplication::Get().OnApplicationPreInputKeyDownListener().Remove(GlobalKeyDownHandle);
				GlobalKeyDownHandle.Reset();
			}
		}

#if WITH_SLATE_DEBUGGING
		// 移除绘制高亮回调
		if (PaintDebugElementsHandle.IsValid())
		{
			FSlateDebugging::PaintDebugElements.Remove(PaintDebugElementsHandle);
			PaintDebugElementsHandle.Reset();
		}
#endif

		// 更新UI
		if (PickButton.IsValid())
		{
			PickButton->SetContent(
				SNew(STextBlock)
				.Text(LOCTEXT("PickButton", "Pick"))
			);
		}
		if (PickingStatusText.IsValid())
		{
			PickingStatusText->SetText(FText::GetEmpty());
		}
	}
}

void SUMGReflectorTree::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (bPickingMode)
	{
		// 定期刷新树数据（每秒一次），防止动态UI重建导致引用失效
		PickingTreeRefreshTimer += InDeltaTime;
		if (PickingTreeRefreshTimer >= 1.0f)
		{
			PickingTreeRefreshTimer = 0.0f;
			UpdateWidgetTree();
		}

		UpdatePickingHover();
	}
}

void SUMGReflectorTree::UpdatePickingHover()
{
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	const FVector2D AbsCursorPos = FSlateApplication::Get().GetCursorPos();

	// 使用几何包含关系（而非HitTest）查找光标下最深的UMG Widget
	// HitTest 会被 SConstraintCanvas 等容器的 Visibility 设置拦截
	// 几何方式直接检查 SWidget::GetCachedGeometry().IsUnderLocation()，完全绕过 HitTest
	TSharedPtr<FUMGReflectorItem> FoundItem = FindDeepestItemUnderCursor(UMGRootItems, AbsCursorPos);

	if (FoundItem.IsValid() && FoundItem != HoveredPickItem)
	{
		HoveredPickItem = FoundItem;
		bHasValidHoverTarget = false;

		TWeakObjectPtr<const UWidget> UWidgetPtr = FoundItem->GetWidget();
		if (UWidgetPtr.IsValid())
		{
			TSharedPtr<SWidget> CachedSWidget = UWidgetPtr->GetCachedWrappedWidget();
			if (CachedSWidget.IsValid())
			{
				// 存储几何信息（desktop space）
				HoveredWidgetGeometry = CachedSWidget->GetCachedGeometry();

				// 通过 FindPathToWidget(EVisibility::All) 获取所在窗口
				// EVisibility::All 绕过 HitTest/Visibility 过滤
				FWidgetPath WidgetPath;
				if (FSlateApplication::Get().FindPathToWidget(CachedSWidget.ToSharedRef(), WidgetPath, EVisibility::All))
				{
					HoveredWidgetWindow = WidgetPath.GetWindow();
					bHasValidHoverTarget = true;
				}
			}
		}

		if (PickingStatusText.IsValid())
		{
			PickingStatusText->SetText(FText::Format(
				LOCTEXT("PickingHover", "Hovering: {0} ({1}) - Click to pick, ESC to cancel"),
				FText::FromString(HoveredPickItem->GetDisplayName()),
				FText::FromString(HoveredPickItem->GetTypeName())
			));
		}
	}
	else if (!FoundItem.IsValid() && HoveredPickItem.IsValid())
	{
		HoveredPickItem.Reset();
		bHasValidHoverTarget = false;
		if (PickingStatusText.IsValid())
		{
			PickingStatusText->SetText(LOCTEXT("PickingNoWidget", "No UMG widget under cursor - Click to pick, ESC to cancel"));
		}
	}
}

TSharedPtr<FUMGReflectorItem> SUMGReflectorTree::FindTreeItemForWidgetPath(const FWidgetPath& InWidgetPath) const
{
	// 从最深节点（叶子）向上遍历，找到第一个匹配的UWidget
	const FArrangedChildren& Widgets = InWidgetPath.Widgets;
	for (int32 i = Widgets.Num() - 1; i >= 0; --i)
	{
		TSharedRef<SWidget> CurrentSWidget = Widgets[i].Widget;

		TSharedPtr<FUMGReflectorItem> FoundItem = FindTreeItemBySWidget(UMGRootItems, CurrentSWidget);
		if (FoundItem.IsValid())
		{
			return FoundItem;
		}
	}

	return nullptr;
}

TSharedPtr<FUMGReflectorItem> SUMGReflectorTree::FindTreeItemBySWidget(
	const TArray<TSharedPtr<FUMGReflectorItem>>& InItems,
	const TSharedRef<SWidget>& InSWidget) const
{
	for (const TSharedPtr<FUMGReflectorItem>& Item : InItems)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		TWeakObjectPtr<const UWidget> UWidgetPtr = Item->GetWidget();
		if (UWidgetPtr.IsValid())
		{
			// 必须使用 GetCachedWrappedWidget 而非 GetCachedWidget
			// 与引擎 UWidgetTree::FindWidget 保持一致
			// GetCachedWrappedWidget 会优先返回 ComponentWrapperWidget/DesignWrapperWidget
			// 这些包装层在 Slate 树中实际出现，是 FWidgetPath 中会命中的节点
			TSharedPtr<SWidget> CachedSWidget = UWidgetPtr->GetCachedWrappedWidget();
			if (CachedSWidget.IsValid() && CachedSWidget == InSWidget)
			{
				return Item;
			}
		}

		// 递归搜索子节点
		TSharedPtr<FUMGReflectorItem> FoundInChildren = FindTreeItemBySWidget(Item->GetChildrenData(), InSWidget);
		if (FoundInChildren.IsValid())
		{
			return FoundInChildren;
		}
	}

	return nullptr;
}

// SWidgetReflector::SelectLiveWidget(TSharedPtr<const SWidget> InWidget)
TSharedPtr<FUMGReflectorItem> SUMGReflectorTree::FindDeepestItemUnderCursor(
	const TArray<TSharedPtr<FUMGReflectorItem>>& InItems,
	const FVector2D& AbsCursorPos) const
{
	// 反向迭代：后添加的Widget在视觉上层（后渲染），优先选中
	for (int32 i = InItems.Num() - 1; i >= 0; --i)
	{
		const TSharedPtr<FUMGReflectorItem>& Item = InItems[i];
		if (!Item.IsValid())
		{
			continue;
		}

		TWeakObjectPtr<const UWidget> UWidgetPtr = Item->GetWidget();
		if (!UWidgetPtr.IsValid())
		{
			continue;
		}

		TSharedPtr<SWidget> CachedSWidget = UWidgetPtr->GetCachedWrappedWidget();
		if (!CachedSWidget.IsValid())
		{
			continue;
		}
		
		if (CachedSWidget->GetVisibility() == EVisibility::Collapsed || CachedSWidget->GetVisibility() == EVisibility::Hidden)
		{
			continue;
		}

		const FGeometry& CachedGeometry = CachedSWidget->GetCachedGeometry();
		const FVector2D AbsPos = CachedGeometry.GetAbsolutePosition();
		const FVector2D AbsSize = CachedGeometry.GetAbsoluteSize();
		const bool bUnder = CachedGeometry.IsUnderLocation(AbsCursorPos);

		UE_LOG(LogTemp, Log, TEXT("[Pick] %s (%s) | SWidget=%s | AbsPos=(%.1f,%.1f) AbsSize=(%.1f,%.1f) | Cursor=(%.1f,%.1f) | Under=%d | Children=%d"),
			*Item->GetDisplayName(), *Item->GetTypeName(),
			*CachedSWidget->GetTypeAsString(),
			AbsPos.X, AbsPos.Y, AbsSize.X, AbsSize.Y,
			AbsCursorPos.X, AbsCursorPos.Y,
			bUnder ? 1 : 0,
			Item->GetChildrenData().Num());

		if (bUnder)
		{
			// 先检查子节点，返回最深的匹配
			TSharedPtr<FUMGReflectorItem> ChildResult = FindDeepestItemUnderCursor(Item->GetChildrenData(), AbsCursorPos);
			if (ChildResult.IsValid())
			{
				return ChildResult;
			}
			// 没有子节点匹配，返回当前节点
			UE_LOG(LogTemp, Log, TEXT("[Pick] >> Selected: %s (no child matched)"), *Item->GetDisplayName());
			return Item;
		}
	}

	return nullptr;
}

void SUMGReflectorTree::OnGlobalMouseButtonDown(const FPointerEvent& MouseEvent)
{
	if (!bPickingMode)
	{
		return;
	}

	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (HoveredPickItem.IsValid())
		{
			ConfirmPick();
		}
	}
}

void SUMGReflectorTree::OnGlobalKeyDown(const FKeyEvent& InKeyEvent)
{
	if (!bPickingMode)
	{
		return;
	}

	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		CancelPick();
	}
}

void SUMGReflectorTree::ConfirmPick()
{
	TSharedPtr<FUMGReflectorItem> PickedItem = HoveredPickItem;
	SetPickingMode(false);

	if (PickedItem.IsValid())
	{
		SelectAndExpandToItem(PickedItem);
	}
}

void SUMGReflectorTree::CancelPick()
{
	SetPickingMode(false);
}

#if WITH_SLATE_DEBUGGING
void SUMGReflectorTree::OnPaintDebugElements(const FPaintArgs& InArgs, const FGeometry& InAllottedGeometry, FSlateWindowElementList& InOutDrawElements, int32& InOutLayerId) const
{
	if (!bPickingMode || !bHasValidHoverTarget)
	{
		return;
	}

	// 检查当前绘制的窗口是否是悬停Widget所在的窗口
	TSharedPtr<SWindow> Window = HoveredWidgetWindow.Pin();
	if (!Window.IsValid())
	{
		return;
	}

	SWindow* PaintWindow = InOutDrawElements.GetPaintWindow();
	if (PaintWindow != Window.Get())
	{
		return;
	}

	// 将 desktop space 的几何信息转换为 window space
	FPaintGeometry WindowSpaceGeometry = HoveredWidgetGeometry.ToPaintGeometry();
	WindowSpaceGeometry.AppendTransform(TransformCast<FSlateLayoutTransform>(Inverse(Window->GetPositionInScreen())));

	WindowSpaceGeometry.CommitTransformsIfUsingLegacyConstructor();
	const FVector2D LocalSize = WindowSpaceGeometry.GetLocalSize();

	// 绘制绿色边框高亮
	const FLinearColor HighlightColor(0.0f, 1.0f, 0.0f, 1.0f);

	if (FMath::IsNearlyZero(LocalSize.X) || FMath::IsNearlyZero(LocalSize.Y))
	{
		// 零尺寸Widget绘制一条线
		TArray<FVector2f> LinePoints;
		LinePoints.SetNum(2);
		LinePoints[0] = FVector2f::ZeroVector;
		LinePoints[1] = FVector2f(static_cast<float>(LocalSize.X), static_cast<float>(LocalSize.Y));

		FSlateDrawElement::MakeLines(
			InOutDrawElements,
			++InOutLayerId,
			WindowSpaceGeometry,
			LinePoints,
			ESlateDrawEffect::None,
			HighlightColor,
			true,
			2.0f
		);
	}
	else
	{
		// 绘制边框
		FSlateDrawElement::MakeBox(
			InOutDrawElements,
			++InOutLayerId,
			WindowSpaceGeometry,
			FCoreStyle::Get().GetBrush(TEXT("Debug.Border")),
			ESlateDrawEffect::None,
			HighlightColor
		);
	}
}
#endif

void SUMGReflectorTree::SelectAndExpandToItem(const TSharedPtr<FUMGReflectorItem>& InItem)
{
	if (!InItem.IsValid() || !UMGTreeViewSlate.IsValid())
	{
		return;
	}

	// 清空搜索过滤，确保所有节点可见
	if (!CurrentSearchText.IsEmpty())
	{
		CurrentSearchText.Empty();
		if (SearchBox.IsValid())
		{
			SearchBox->SetText(FText::GetEmpty());
		}
		FilteredRootItems = UMGRootItems;
		UMGTreeViewSlate->RequestTreeRefresh();
	}

	// 查找从根到目标节点的路径
	TArray<TSharedPtr<FUMGReflectorItem>> Path;
	FindPathToItem(FilteredRootItems, InItem, Path);

	// 展开路径上的每个节点
	for (const TSharedPtr<FUMGReflectorItem>& PathItem : Path)
	{
		UMGTreeViewSlate->SetItemExpansion(PathItem, true);
	}

	// 选中目标节点并滚动到可见
	UMGTreeViewSlate->SetSelection(InItem);
	UMGTreeViewSlate->RequestScrollIntoView(InItem);
}

bool SUMGReflectorTree::FindPathToItem(
	const TArray<TSharedPtr<FUMGReflectorItem>>& InItems,
	const TSharedPtr<FUMGReflectorItem>& InTarget,
	TArray<TSharedPtr<FUMGReflectorItem>>& OutPath) const
{
	for (const TSharedPtr<FUMGReflectorItem>& Item : InItems)
	{
		if (!Item.IsValid())
		{
			continue;
		}

		if (Item == InTarget)
		{
			OutPath.Add(Item);
			return true;
		}

		if (FindPathToItem(Item->GetChildrenData(), InTarget, OutPath))
		{
			OutPath.Insert(Item, 0);
			return true;
		}
	}

	return false;
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