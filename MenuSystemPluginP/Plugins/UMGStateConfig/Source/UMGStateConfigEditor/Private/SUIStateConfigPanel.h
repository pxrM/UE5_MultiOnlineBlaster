#pragma once

#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "UMGStateConfigData.h"
#include "Widgets/SCompoundWidget.h"

struct FPropertyChangedEvent;

class FWidgetBlueprintEditor;
class ITableRow;
class SListViewBase;
class STableViewBase;
class UUMGStateConfigBlueprintExtension;
class UWidget;
class UWidgetBlueprint;

template <typename ItemType>
class SListView;

template <typename ItemType>
class STableRow;

struct FUMGStateConfigWidgetRow
{
	TWeakObjectPtr<UWidget> Widget;
};

struct FUMGStateConfigDetailsProxyContext
{
	TWeakObjectPtr<UObject> ProxyObject;
	TMap<FString, FUMGStateConfigPropertyValue> Snapshot;
	bool bCommonOnly = true;
};


class SUIStateConfigPanel : public SCompoundWidget

{
public:
	SLATE_BEGIN_ARGS(SUIStateConfigPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<FWidgetBlueprintEditor> InWidgetEditor);
	virtual ~SUIStateConfigPanel() override;

private:
	UWidgetBlueprint* GetWidgetBlueprint() const;
	UUMGStateConfigBlueprintExtension* GetOrCreateExtension();
	const UUMGStateConfigBlueprintExtension* GetExtension() const;
	FUMGStateConfigGroup* GetActiveGroup();
	FUMGStateConfigState* GetActiveState();
	UWidget* FindWidgetByName(FName WidgetName) const;

	FText GetTitleText() const;
	FText GetSummaryText() const;
	FText GetBreadcrumbText() const;
	FReply EnsureDefaultConfig();
	FReply ApplyPreviewState();
	void ResetDesignerPreview() const;
	FReply AddParentState();
	FReply DuplicateParentState();
	FReply DeleteParentState();

	FReply AddChildState();
	FReply DuplicateChildState();
	FReply DeleteChildState();
	FReply RemoveInvalidChanges();
	FReply ClearWidgetConfig(FName WidgetName);


	TSharedRef<SWidget> BuildParentStateTabs();
	TSharedRef<SWidget> BuildChildStateTabs();
	TSharedRef<SWidget> BuildStateTab(FName StateName, bool bSelected, bool bIsParentState);
	TSharedRef<SWidget> BuildRenameMenu(FName OldName, bool bIsParentState);
	TSharedRef<SWidget> BuildAvailableWidgetPropertyMenu(FName WidgetName);
	TSubclassOf<UWidget> GetExpectedWidgetClass(FName WidgetName) const;
	void RenameParentState(FName OldName, FName NewName);

	void RenameChildState(FName OldName, FName NewName);
	FReply HandleStateTabMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent, FName StateName, bool bIsParentState);
	FReply HandleAvailableWidgetRowMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent, TSharedPtr<FUMGStateConfigWidgetRow> RowItem);

	TSharedRef<SWidget> BuildConfiguredWidgetCards();
	TSharedRef<SWidget> BuildConfiguredWidgetCard(FName WidgetName);
	TSharedRef<SWidget> BuildSerializedPropertyRow(const FUMGStatePropertyChange& Change, TOptional<FText> LabelOverride = TOptional<FText>());




	void RefreshAll();
	void RefreshStatesAndConfig();
	void RefreshConfigOnly();
	void RequestPreviewRefresh();
	void RequestConfigRefresh();
	void EnsureDeferredTimer();
	EActiveTimerReturnType HandleDeferredRefresh(double InCurrentTime, float InDeltaTime);
	void RefreshStateTabs();
	void RefreshWidgetList();
	void RefreshConfiguredWidgets();
	void NormalizeRedundantPropertyChanges();
	void RefreshSummary();
	void RebuildWidgetRows();
	void OnWidgetFilterChanged(const FText& InText);

	void SelectParentState(FName GroupName);
	void SelectChildState(FName StateName);

	TSharedRef<ITableRow> GenerateWidgetRow(TSharedPtr<FUMGStateConfigWidgetRow> RowItem, const TSharedRef<STableViewBase>& OwnerTable);
	TSharedRef<ITableRow> GenerateConfiguredWidgetRow(TSharedPtr<FName> Item, const TSharedRef<STableViewBase>& OwnerTable);
	EVisibility GetConfiguredEmptyVisibility() const;
	void OnWidgetSelected(TSharedPtr<FUMGStateConfigWidgetRow> RowItem, ESelectInfo::Type SelectInfo);
	void OnWidgetDoubleClicked(TSharedPtr<FUMGStateConfigWidgetRow> RowItem);
	void AddWidgetToActiveGroupStates(FName WidgetName);
	void OpenWidgetDetailsPropertyPicker(FName WidgetName, bool bCommonOnly);
	void OnWidgetDetailsPropertyFinishedChanging(const FPropertyChangedEvent& PropertyChangedEvent, FName WidgetName, TWeakObjectPtr<UObject> ProxyObject);
	void CaptureEditablePropertySnapshot(UWidget* Widget, bool bCommonOnly, TMap<FString, FUMGStateConfigPropertyValue>& OutSnapshot) const;
	void CaptureEditablePropertySnapshotRecursive(UWidget* Widget, UStruct* CurrentStruct, void* CurrentContainer, const FString& PathPrefix, bool bCommonOnly, TMap<FString, FUMGStateConfigPropertyValue>& OutSnapshot) const;
	void DiffPropertySnapshot(const TMap<FString, FUMGStateConfigPropertyValue>& BeforeSnapshot, const TMap<FString, FUMGStateConfigPropertyValue>& AfterSnapshot, TArray<FUMGStateConfigPropertyValue>& OutChangedValues) const;
	void FilterNoisyPropertyChanges(TArray<FUMGStateConfigPropertyValue>& InOutChangedValues, TArray<FString>& OutFilteredPaths) const;

	FUMGStateConfigDetailsProxyContext* FindDetailsProxyContext(UObject* ProxyObject);

	void AddOrUpdateSerializedPropertyChange(FName WidgetName, const FUMGStateConfigPropertyValue& Value, TSubclassOf<UWidget> ExpectedClass);

	FReply RemoveSerializedPropertyChange(FName WidgetName, FString SerializedPropertyPath);
	FReply RemoveSerializedPropertyGroup(FName WidgetName, FString TopLevelSegment);

	TArray<FName> GetConfiguredWidgetNames() const;

	FText GetSerializedPropertyDisplayName(const FString& SerializedPropertyPath) const;
	FString FormatPropertyChange(const FUMGStatePropertyChange& Change) const;

	void MarkConfigDirty(UUMGStateConfigBlueprintExtension* Extension);

private:
	TWeakPtr<FWidgetBlueprintEditor> WidgetEditor;
	TSharedPtr<class STextBlock> SummaryTextBlock;
	TSharedPtr<class SVerticalBox> ParentTabsBox;
	TSharedPtr<class SVerticalBox> ChildTabsBox;
	TSharedPtr<SListView<TSharedPtr<FName>>> ConfiguredWidgetsListView;
	TArray<TSharedPtr<FName>> ConfiguredWidgetItems;
	TSharedPtr<SListView<TSharedPtr<FUMGStateConfigWidgetRow>>> WidgetListView;

	TArray<TSharedPtr<FUMGStateConfigWidgetRow>> WidgetRows;
	TArray<TStrongObjectPtr<UObject>> AppearanceDetailProxyObjects;
	TArray<FUMGStateConfigDetailsProxyContext> DetailsProxyContexts;

	FName SelectedGroupName;
	FText LastDetailsCaptureMessage;

	FName SelectedStateName;

	FName SelectedWidgetName;
	FString WidgetFilterText;

	bool bPreviewRefreshPending = false;
	bool bConfigRefreshPending = false;
	bool bDeferredTimerRegistered = false;
};
