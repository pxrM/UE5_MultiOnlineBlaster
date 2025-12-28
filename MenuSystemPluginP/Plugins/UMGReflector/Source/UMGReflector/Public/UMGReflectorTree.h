#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class FUMGReflectorItem;

class SUMGReflectorTree : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SUMGReflectorTree)
		{
		}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SUMGReflectorTree() override;

	void UpdateWidgetTree();

private:
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FUMGReflectorItem> InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	void OnGetChildren(TSharedPtr<FUMGReflectorItem> InItem, TArray<TSharedPtr<FUMGReflectorItem>>& OutChildren);
	void OnSelectionChanged(TSharedPtr<FUMGReflectorItem> InItem, ESelectInfo::Type SelectionType);
	void BuildUMGWidgetTree(const UUserWidget* InWBPWidget, const TSharedPtr<SWidget>& InCurrentWidget, const TSharedPtr<FUMGReflectorItem>& InParent);
	FString GetUMGReflectorWidgetName(const UUserWidget* InWidget, const TSharedPtr<SWidget> InSlateWidget);
	void FindAllUserWidget(const UWorld* InWorld, TArray<TSharedPtr<FUMGReflectorItem>>& OutAllUserWidget);

	void OnPostPIEModelStarted(bool bIsSimulating);
	void OnEndPIEModel(bool bIsSimulating);
	FReply RefreshButtonClicked();

	void CreateInstanceDetailsView();

	
private:
	TSharedPtr<STreeView<TSharedPtr<FUMGReflectorItem>>> UMGTreeViewSlate;
	TArray<TSharedPtr<FUMGReflectorItem>> UMGRootItems;

	TSharedPtr<FTabManager> TabManager;
#if WITH_EDITOR
	TSharedPtr<IDetailsView> PropertyViewPtr;
#endif
	
};
