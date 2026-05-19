#pragma once

#include "CoreMinimal.h"
#include "UObject/StrongObjectPtr.h"
#include "UMGStateConfigData.h"
#include "Widgets/SCompoundWidget.h"

struct FAssetData;
struct FPropertyChangedEvent;
class FWidgetBlueprintEditor;
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
	FReply DeleteParentState();
	FReply AddChildState();
	FReply DeleteChildState();
	FReply ClearWidgetConfig(FName WidgetName);

	TSharedRef<SWidget> BuildParentStateTabs();
	TSharedRef<SWidget> BuildChildStateTabs();
	TSharedRef<SWidget> BuildStateTab(FName StateName, bool bSelected, bool bIsParentState);
	TSharedRef<SWidget> BuildRenameMenu(FName OldName, bool bIsParentState);
	TSharedRef<SWidget> BuildAvailableWidgetPropertyMenu(FName WidgetName);
	TArray<EUMGStateConfigPropertyType> GetSupportedPropertyTypes(const UWidget* Widget) const;
	FText GetPropertyLabel(EUMGStateConfigPropertyType PropertyType) const;
	TSubclassOf<UWidget> GetExpectedWidgetClass(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const;
	void RenameParentState(FName OldName, FName NewName);
	void RenameChildState(FName OldName, FName NewName);
	FReply HandleStateTabMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent, FName StateName, bool bIsParentState);
	FReply HandleAvailableWidgetRowMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent, TSharedPtr<FUMGStateConfigWidgetRow> RowItem);

	TSharedRef<SWidget> BuildConfiguredWidgetCards();
	TSharedRef<SWidget> BuildConfiguredWidgetCard(FName WidgetName);
	TSharedRef<SWidget> BuildPropertyRow(FName WidgetName, EUMGStateConfigPropertyType PropertyType, const FText& Label, TSubclassOf<UWidget> ExpectedClass);
	TSharedRef<SWidget> BuildSerializedPropertyRow(const FUMGStatePropertyChange& Change);
	TSharedRef<SWidget> BuildPropertyValueWidget(FName WidgetName, EUMGStateConfigPropertyType PropertyType, TSubclassOf<UWidget> ExpectedClass);
	TSharedRef<SWidget> BuildAppearanceDetailsValueWidget(FName WidgetName, EUMGStateConfigPropertyType PropertyType);



	void RefreshAll();
	void NormalizeRedundantPropertyChanges();
	void RefreshSummary();
	void RebuildWidgetRows();
	void SelectParentState(FName GroupName);
	void SelectChildState(FName StateName);

	TSharedRef<ITableRow> GenerateWidgetRow(TSharedPtr<FUMGStateConfigWidgetRow> RowItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnWidgetSelected(TSharedPtr<FUMGStateConfigWidgetRow> RowItem, ESelectInfo::Type SelectInfo);
	void OnWidgetDoubleClicked(TSharedPtr<FUMGStateConfigWidgetRow> RowItem);
	void AddWidgetToActiveGroupStates(FName WidgetName);
	void AddWidgetPropertyToActiveState(FName WidgetName, EUMGStateConfigPropertyType PropertyType);
	void OpenWidgetDetailsPropertyPicker(FName WidgetName);
	void OnWidgetDetailsPropertyFinishedChanging(const FPropertyChangedEvent& PropertyChangedEvent, FName WidgetName, TWeakObjectPtr<UObject> ProxyObject);

	FUMGStatePropertyChange* FindPropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType);

	const FUMGStatePropertyChange* FindPropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const;
	bool HasPropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const;
	ECheckBoxState GetPropertyCheckState(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const;
	void OnPropertyCheckChanged(ECheckBoxState NewState, FName WidgetName, EUMGStateConfigPropertyType PropertyType, TSubclassOf<UWidget> ExpectedClass);
	void AddOrUpdatePropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& Value, TSubclassOf<UWidget> ExpectedClass);
	void AddOrUpdateSerializedPropertyChange(FName WidgetName, const FUMGStateConfigPropertyValue& Value, TSubclassOf<UWidget> ExpectedClass);
	void RemovePropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType);
	FReply RemoveSerializedPropertyChange(FName WidgetName, FString SerializedPropertyPath);
	void OnAppearanceDetailsFinishedChanging(const FPropertyChangedEvent& PropertyChangedEvent, FName WidgetName, EUMGStateConfigPropertyType PropertyType, TWeakObjectPtr<UObject> ProxyObject);


	FUMGStateConfigPropertyValue MakeDefaultValueForWidget(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const;

	TArray<FName> GetConfiguredWidgetNames() const;
	bool IsWidgetText(const UWidget* Widget) const;

	bool IsWidgetImage(const UWidget* Widget) const;
	FString FormatPropertyChange(const FUMGStatePropertyChange& Change) const;
	void MarkConfigDirty(UUMGStateConfigBlueprintExtension* Extension);
	TUniquePtr<class FScopedTransaction> BeginConfigEdit(const FText& Description, UUMGStateConfigBlueprintExtension* Extension);

private:
	TWeakPtr<FWidgetBlueprintEditor> WidgetEditor;
	TSharedPtr<class STextBlock> SummaryTextBlock;
	TSharedPtr<class SVerticalBox> ParentTabsBox;
	TSharedPtr<class SVerticalBox> ChildTabsBox;
	TSharedPtr<class SVerticalBox> ConfiguredWidgetsBox;
	TSharedPtr<SListView<TSharedPtr<FUMGStateConfigWidgetRow>>> WidgetListView;

	TArray<TSharedPtr<FUMGStateConfigWidgetRow>> WidgetRows;
	TArray<TStrongObjectPtr<UObject>> AppearanceDetailProxyObjects;

	FName SelectedGroupName;
	FName SelectedStateName;
	FName SelectedWidgetName;
	FText CachedSummaryText;
};
