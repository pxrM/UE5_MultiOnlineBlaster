#pragma once

#include "CoreMinimal.h"
#include "AssetCookRuleManager.h"
#include "AssetCookScanner.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STreeView.h"
#include "Widgets/Views/SListView.h"

/** Editor panel: per-directory cook rules + NeverCook reference validation. */
class SAssetCookManagerPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAssetCookManagerPanel) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// --- Tree ---
	void RefreshTree();
	/** Snapshot saved rules into SavedRules so per-row lookups avoid re-reading config. */
	void RebuildRuleCache();
	/** Rebuild the visible (filtered) node list from the full tree. */
	void RebuildVisibleTree();
	void OnFilterTextChanged(const FText& NewText);
	TSharedRef<ITableRow> OnGenerateDirRow(TSharedPtr<FCookDirNode> Node, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TSharedPtr<FCookDirNode> Node, TArray<TSharedPtr<FCookDirNode>>& OutChildren);

	/** Explicit rule for a directory: pending edit if any, else the saved exact rule. */
	ECookRuleType GetEffectiveRule(const FString& PackagePath) const;
	/** Rule to display, including parent inheritance when this dir has no explicit rule. */
	FResolvedCookRule GetDisplayRule(const FString& PackagePath) const;
	void OnRuleChanged(TSharedPtr<ECookRuleType> NewValue, ESelectInfo::Type SelectInfo, TSharedPtr<FCookDirNode> Node);

	/** Set (or clear, for Default) a pending rule for one directory. */
	void SetPendingRule(const FString& PackagePath, ECookRuleType NewRule);
	/** Apply a rule to every currently selected tree row (batch edit). */
	void SetRuleForSelection(ECookRuleType NewRule);
	/** Right-click menu for the directory tree: batch rule edits + navigation. */
	TSharedPtr<SWidget> OnTreeContextMenu();
	/** Find assets referencing the first selected directory; show in results list. */
	void RunFindReferencers();

	// --- Buttons ---
	FReply OnApplyClicked();
	FReply OnValidateClicked();
	FReply OnRefreshClicked();
	FReply OnExportClicked();
	FReply OnCleanClicked();
	bool HasPendingChanges() const { return PendingRules.Num() > 0; }
	bool HasResults() const { return ValidationResults.Num() > 0; }

	/** True when this dir's explicit rule (pending or saved) equals what it would inherit. */
	bool IsRedundant(const FString& PackagePath) const;

	/** Toolbar label: counts of Always/Never rules plus pending edits. */
	FText GetRuleStatsText() const;

	// --- Validation results ---
	TSharedRef<ITableRow> OnGenerateViolationRow(TSharedPtr<FCookViolation> Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnViolationDoubleClick(TSharedPtr<FCookViolation> Item);

	static FText RuleToText(ECookRuleType Rule);

	/** Full (unfiltered) tree as built by the scanner. */
	TArray<TSharedPtr<FCookDirNode>> AllRootNodes;
	/** Nodes currently fed to the tree view (== AllRootNodes when no filter). */
	TArray<TSharedPtr<FCookDirNode>> RootNodes;
	TSharedPtr<STreeView<TSharedPtr<FCookDirNode>>> TreeView;

	/** Lower-cased substring filter from the search box; empty shows everything. */
	FString FilterText;

	/** Edits not yet written to config, keyed by long package path. */
	TMap<FString, ECookRuleType> PendingRules;

	/** Saved rules snapshot (package path -> rule), refreshed on RefreshTree. */
	TMap<FString, ECookRuleType> SavedRules;

	/** Combo box option pool (Default / AlwaysCook / NeverCook). */
	TArray<TSharedPtr<ECookRuleType>> RuleOptions;

	TArray<TSharedPtr<FCookViolation>> ValidationResults;
	TSharedPtr<SListView<TSharedPtr<FCookViolation>>> ResultsView;
	FText ValidationSummary;
};
