#include "SAssetCookManagerPanel.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "ContentBrowserModule.h"
#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IContentBrowserSingleton.h"
#include "IDesktopPlatform.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Misc/ScopedSlowTask.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SExpanderArrow.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "SAssetCookManagerPanel"

namespace
{
	IAssetRegistry& GetPanelAssetRegistry()
	{
		FAssetRegistryModule& Module =
			FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		return Module.Get();
	}

	/** Leaf name of a long package path, for compact display ("/Game/UI" -> "UI"). */
	FString LeafOf(const FString& PackagePath)
	{
		FString Parent;
		FString Leaf;
		if (PackagePath.Split(TEXT("/"), &Parent, &Leaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd)
			&& !Leaf.IsEmpty())
		{
			return Leaf;
		}
		return PackagePath;
	}

	FString EscapeCsvField(const FString& Field)
	{
		FString Escaped = Field;
		Escaped.ReplaceInline(TEXT("\""), TEXT("\"\""));

		if (Escaped.Contains(TEXT(",")) || Escaped.Contains(TEXT("\""))
			|| Escaped.Contains(TEXT("\r")) || Escaped.Contains(TEXT("\n")))
		{
			return FString::Printf(TEXT("\"%s\""), *Escaped);
		}

		return Escaped;
	}

	/**
	 * Deep-copy a node keeping only the subtree that matches LowerFilter (by leaf
	 * name or package path). Returns null when neither the node nor any descendant
	 * matches.
	 */
	TSharedPtr<FCookDirNode> FilterNode(const TSharedPtr<FCookDirNode>& Src, const FString& LowerFilter)
	{
		TArray<TSharedPtr<FCookDirNode>> KeptChildren;
		for (const TSharedPtr<FCookDirNode>& Child : Src->Children)
		{
			if (TSharedPtr<FCookDirNode> Filtered = FilterNode(Child, LowerFilter))
			{
				KeptChildren.Add(Filtered);
			}
		}

		const bool bSelfMatch =
			Src->DisplayName.ToLower().Contains(LowerFilter) ||
			Src->PackagePath.ToLower().Contains(LowerFilter);

		if (!bSelfMatch && KeptChildren.Num() == 0)
		{
			return nullptr;
		}

		TSharedPtr<FCookDirNode> Copy = MakeShared<FCookDirNode>();
		Copy->PackagePath = Src->PackagePath;
		Copy->DisplayName = Src->DisplayName;
		Copy->Children = MoveTemp(KeptChildren);
		return Copy;
	}
}

void SAssetCookManagerPanel::Construct(const FArguments& InArgs)
{
	RuleOptions.Add(MakeShared<ECookRuleType>(ECookRuleType::Default));
	RuleOptions.Add(MakeShared<ECookRuleType>(ECookRuleType::AlwaysCook));
	RuleOptions.Add(MakeShared<ECookRuleType>(ECookRuleType::NeverCook));

	ValidationSummary = LOCTEXT("ValidateHint", "Run validation to find packaged assets that reference NeverCook content.");

	ChildSlot
	[
		SNew(SVerticalBox)

		// Toolbar
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 8.0f, 8.0f, 4.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Refresh", "Refresh"))
				.ToolTipText(LOCTEXT("RefreshTip", "Reload the directory tree and saved rules (discards pending edits)."))
				.OnClicked(this, &SAssetCookManagerPanel::OnRefreshClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Apply", "Apply Rules"))
				.ToolTipText(LOCTEXT("ApplyTip", "Write pending rule changes to DefaultGame.ini."))
				.IsEnabled_Lambda([this]() { return HasPendingChanges(); })
				.OnClicked(this, &SAssetCookManagerPanel::OnApplyClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Validate", "Validate"))
				.ToolTipText(LOCTEXT("ValidateTip", "Find packaged assets that reference NeverCook content (hard + soft)."))
				.OnClicked(this, &SAssetCookManagerPanel::OnValidateClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 4, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Export", "Export CSV"))
				.ToolTipText(LOCTEXT("ExportTip", "Export the validation results to a CSV file."))
				.IsEnabled_Lambda([this]() { return HasResults(); })
				.OnClicked(this, &SAssetCookManagerPanel::OnExportClicked)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 12, 0)
			[
				SNew(SButton)
				.Text(LOCTEXT("Clean", "Clean"))
				.ToolTipText(LOCTEXT("CleanTip", "Remove saved rules already implied by a parent directory. Apply or refresh pending edits first."))
				.IsEnabled_Lambda([this]() { return !HasPendingChanges(); })
				.OnClicked(this, &SAssetCookManagerPanel::OnCleanClicked)
			]

			// Search filter
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SSearchBox)
				.HintText(LOCTEXT("SearchHint", "Filter directories..."))
				.OnTextChanged(this, &SAssetCookManagerPanel::OnFilterTextChanged)
			]

			// Rule stats
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(12, 0, 0, 0)
			[
				SNew(STextBlock)
				.Text_Lambda([this]() { return GetRuleStatsText(); })
			]
		]

		// Directory tree
		+ SVerticalBox::Slot()
		.FillHeight(0.62f)
		.Padding(8.0f, 4.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SAssignNew(TreeView, STreeView<TSharedPtr<FCookDirNode>>)
				.TreeItemsSource(&RootNodes)
				.OnGenerateRow(this, &SAssetCookManagerPanel::OnGenerateDirRow)
				.OnGetChildren(this, &SAssetCookManagerPanel::OnGetChildren)
				.OnContextMenuOpening(this, &SAssetCookManagerPanel::OnTreeContextMenu)
				.SelectionMode(ESelectionMode::Multi)
			]
		]

		// Validation summary
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(8.0f, 4.0f, 8.0f, 0.0f)
		[
			SNew(STextBlock)
			.Text_Lambda([this]() { return ValidationSummary; })
			.AutoWrapText(true)
		]

		// Validation results
		+ SVerticalBox::Slot()
		.FillHeight(0.38f)
		.Padding(8.0f, 4.0f, 8.0f, 8.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			[
				SAssignNew(ResultsView, SListView<TSharedPtr<FCookViolation>>)
				.ListItemsSource(&ValidationResults)
				.OnGenerateRow(this, &SAssetCookManagerPanel::OnGenerateViolationRow)
				.OnMouseButtonDoubleClick(this, &SAssetCookManagerPanel::OnViolationDoubleClick)
				.SelectionMode(ESelectionMode::Single)
			]
		]
	];

	RefreshTree();
}

void SAssetCookManagerPanel::RefreshTree()
{
	AllRootNodes = FAssetCookScanner::BuildContentTree();
	RebuildRuleCache();
	RebuildVisibleTree();
}

void SAssetCookManagerPanel::RebuildRuleCache()
{
	FAssetCookRuleManager::GetAllRuledDirectories(SavedRules);
}

void SAssetCookManagerPanel::RebuildVisibleTree()
{
	RootNodes.Reset();

	if (FilterText.IsEmpty())
	{
		RootNodes = AllRootNodes;
	}
	else
	{
		for (const TSharedPtr<FCookDirNode>& Root : AllRootNodes)
		{
			if (TSharedPtr<FCookDirNode> Filtered = FilterNode(Root, FilterText))
			{
				RootNodes.Add(Filtered);
			}
		}
	}

	if (TreeView.IsValid())
	{
		TreeView->RequestTreeRefresh();

		// When filtering, expand everything so matches deep in the tree are visible.
		if (!FilterText.IsEmpty())
		{
			TArray<TSharedPtr<FCookDirNode>> Stack = RootNodes;
			while (Stack.Num() > 0)
			{
				TSharedPtr<FCookDirNode> Node = Stack.Pop();
				TreeView->SetItemExpansion(Node, true);
				Stack.Append(Node->Children);
			}
		}
	}
}

void SAssetCookManagerPanel::OnFilterTextChanged(const FText& NewText)
{
	FilterText = NewText.ToString().ToLower();
	RebuildVisibleTree();
}

ECookRuleType SAssetCookManagerPanel::GetEffectiveRule(const FString& PackagePath) const
{
	if (const ECookRuleType* Pending = PendingRules.Find(PackagePath))
	{
		return *Pending;
	}
	if (const ECookRuleType* Saved = SavedRules.Find(PackagePath))
	{
		return *Saved;
	}
	return ECookRuleType::Default;
}

FResolvedCookRule SAssetCookManagerPanel::GetDisplayRule(const FString& PackagePath) const
{
	FResolvedCookRule Result;

	// Walk from this directory up toward /Game. At each level a pending edit
	// overrides the saved rule; the first Always/Never found wins. Mirrors the
	// cooker's recursive inheritance and reflects unsaved edits on ancestors live.
	FString Current = PackagePath;
	while (!Current.IsEmpty())
	{
		ECookRuleType RuleHere = ECookRuleType::Default;
		if (const ECookRuleType* Pending = PendingRules.Find(Current))
		{
			RuleHere = *Pending;
		}
		else if (const ECookRuleType* Saved = SavedRules.Find(Current))
		{
			RuleHere = *Saved;
		}

		if (RuleHere != ECookRuleType::Default)
		{
			Result.Rule = RuleHere;
			Result.bInherited = (Current != PackagePath);
			Result.SourceDir = Current;
			return Result;
		}

		if (Current == TEXT("/Game"))
		{
			break;
		}

		FString Parent;
		FString Leaf;
		if (Current.Split(TEXT("/"), &Parent, &Leaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd)
			&& !Parent.IsEmpty())
		{
			Current = Parent;
		}
		else
		{
			break;
		}
	}

	return Result;
}


FText SAssetCookManagerPanel::RuleToText(ECookRuleType Rule)
{
	switch (Rule)
	{
	case ECookRuleType::AlwaysCook: return LOCTEXT("RuleAlways", "Always Cook");
	case ECookRuleType::NeverCook:  return LOCTEXT("RuleNever", "Never Cook");
	case ECookRuleType::Default:
	default:                        return LOCTEXT("RuleDefault", "Default");
	}
}

FText SAssetCookManagerPanel::GetRuleStatsText() const
{
	TMap<FString, ECookRuleType> Ruled;
	FAssetCookRuleManager::GetAllRuledDirectories(Ruled);

	int32 Always = 0;
	int32 Never = 0;
	for (const TPair<FString, ECookRuleType>& Pair : Ruled)
	{
		if (Pair.Value == ECookRuleType::AlwaysCook) { ++Always; }
		else if (Pair.Value == ECookRuleType::NeverCook) { ++Never; }
	}

	return FText::Format(
		LOCTEXT("RuleStats", "Always: {0}   Never: {1}   Pending: {2}"),
		FText::AsNumber(Always),
		FText::AsNumber(Never),
		FText::AsNumber(PendingRules.Num()));
}

TSharedRef<ITableRow> SAssetCookManagerPanel::OnGenerateDirRow(TSharedPtr<FCookDirNode> Node, const TSharedRef<STableViewBase>& OwnerTable)
{
	// Pick the combo entry matching the current explicit rule.
	const ECookRuleType Effective = GetEffectiveRule(Node->PackagePath);
	TSharedPtr<ECookRuleType> InitialOption = RuleOptions[0];
	for (const TSharedPtr<ECookRuleType>& Option : RuleOptions)
	{
		if (*Option == Effective)
		{
			InitialOption = Option;
			break;
		}
	}

	TSharedRef<STableRow<TSharedPtr<FCookDirNode>>> Row =
		SNew(STableRow<TSharedPtr<FCookDirNode>>, OwnerTable);

	Row->SetContent(
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SExpanderArrow, Row)
		]
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		.Padding(2.0f, 0.0f)
		[
			SNew(STextBlock).Text(FText::FromString(Node->DisplayName))
		]

		// Inheritance hint — only visible when the rule comes from an ancestor.
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.ColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f)))
			.Visibility_Lambda([this, Node]()
			{
				return GetDisplayRule(Node->PackagePath).bInherited ? EVisibility::Visible : EVisibility::Collapsed;
			})
			.Text_Lambda([this, Node]()
			{
				const FResolvedCookRule Resolved = GetDisplayRule(Node->PackagePath);
				return FText::Format(
					LOCTEXT("InheritedFrom", "(inherited {0} from {1})"),
					RuleToText(Resolved.Rule),
					FText::FromString(LeafOf(Resolved.SourceDir)));
			})
		]

		// Redundancy hint — this dir's explicit rule duplicates an inherited one.
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 0.0f)
		[
			SNew(STextBlock)
			.ColorAndOpacity(FSlateColor(FLinearColor(0.95f, 0.85f, 0.3f)))
			.ToolTipText(LOCTEXT("RedundantTip", "Same as the rule inherited from a parent. Use Clean to remove."))
			.Text(LOCTEXT("RedundantTag", "(redundant)"))
			.Visibility_Lambda([this, Node]()
			{
				return IsRedundant(Node->PackagePath) ? EVisibility::Visible : EVisibility::Collapsed;
			})
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(4.0f, 1.0f)
		[
			SNew(SComboBox<TSharedPtr<ECookRuleType>>)
			.OptionsSource(&RuleOptions)
			.InitiallySelectedItem(InitialOption)
			.OnGenerateWidget_Lambda([](TSharedPtr<ECookRuleType> InOption)
			{
				return SNew(STextBlock).Text(SAssetCookManagerPanel::RuleToText(*InOption));
			})
			.OnSelectionChanged(this, &SAssetCookManagerPanel::OnRuleChanged, Node)
			[
				SNew(STextBlock)
				.Text_Lambda([this, Node]()
				{
					return SAssetCookManagerPanel::RuleToText(GetEffectiveRule(Node->PackagePath));
				})
			]
		]);

	return Row;
}

void SAssetCookManagerPanel::OnGetChildren(TSharedPtr<FCookDirNode> Node, TArray<TSharedPtr<FCookDirNode>>& OutChildren)
{
	if (Node.IsValid())
	{
		OutChildren = Node->Children;
	}
}

void SAssetCookManagerPanel::OnRuleChanged(TSharedPtr<ECookRuleType> NewValue, ESelectInfo::Type SelectInfo, TSharedPtr<FCookDirNode> Node)
{
	if (!NewValue.IsValid() || !Node.IsValid() || SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	SetPendingRule(Node->PackagePath, *NewValue);
}

void SAssetCookManagerPanel::SetPendingRule(const FString& PackagePath, ECookRuleType NewRule)
{
	const ECookRuleType Saved = SavedRules.FindRef(PackagePath);
	if (NewRule == Saved)
	{
		// Back to the saved value — drop the pending edit.
		PendingRules.Remove(PackagePath);
	}
	else
	{
		PendingRules.Add(PackagePath, NewRule);
	}
}

void SAssetCookManagerPanel::SetRuleForSelection(ECookRuleType NewRule)
{
	if (!TreeView.IsValid())
	{
		return;
	}

	const TArray<TSharedPtr<FCookDirNode>> Selected = TreeView->GetSelectedItems();
	for (const TSharedPtr<FCookDirNode>& Node : Selected)
	{
		if (Node.IsValid())
		{
			SetPendingRule(Node->PackagePath, NewRule);
		}
	}
}

FReply SAssetCookManagerPanel::OnApplyClicked()
{
	if (PendingRules.IsEmpty())
	{
		return FReply::Handled();
	}

	int32 Always = 0;
	int32 Never = 0;
	int32 Cleared = 0;
	for (const TPair<FString, ECookRuleType>& Pair : PendingRules)
	{
		switch (Pair.Value)
		{
		case ECookRuleType::AlwaysCook: ++Always; break;
		case ECookRuleType::NeverCook:  ++Never; break;
		default:                        ++Cleared; break;
		}
	}

	const FText Confirm = FText::Format(
		LOCTEXT("ApplyConfirm", "Apply {0} pending change(s) to DefaultGame.ini?\n\nAlways Cook: {1}\nNever Cook: {2}\nCleared: {3}"),
		FText::AsNumber(PendingRules.Num()),
		FText::AsNumber(Always),
		FText::AsNumber(Never),
		FText::AsNumber(Cleared));

	if (FMessageDialog::Open(EAppMsgType::YesNo, Confirm) != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	FAssetCookRuleManager::ApplyRules(PendingRules);
	PendingRules.Reset();
	RefreshTree();

	FMessageDialog::Open(EAppMsgType::Ok,
		LOCTEXT("Applied", "Cook rules written to DefaultGame.ini."));

	return FReply::Handled();
}

FReply SAssetCookManagerPanel::OnRefreshClicked()
{
	PendingRules.Reset();
	RefreshTree();
	return FReply::Handled();
}

FReply SAssetCookManagerPanel::OnValidateClicked()
{
	FScopedSlowTask Task(1.0f, LOCTEXT("Validating", "Validating NeverCook references..."));
	Task.MakeDialog(/*bShowCancelButton=*/true);

	float LastFrac = 0.0f;
	const TArray<FCookViolation> Found = FAssetCookScanner::ValidateNeverCookReferences(
		[&Task, &LastFrac](int32 Processed, int32 Total) -> bool
		{
			const float Frac = (Total > 0) ? static_cast<float>(Processed) / static_cast<float>(Total) : 1.0f;
			Task.EnterProgressFrame(
				Frac - LastFrac,
				FText::Format(
					LOCTEXT("ValidatingN", "Scanning packages {0} / {1}..."),
					FText::AsNumber(Processed),
					FText::AsNumber(Total)));
			LastFrac = Frac;
			return Task.ShouldCancel();
		});

	const bool bCancelled = Task.ShouldCancel();
	if (!bCancelled && LastFrac < 1.0f)
	{
		Task.EnterProgressFrame(
			1.0f - LastFrac,
			LOCTEXT("ValidatingDone", "Validation complete."));
	}

	ValidationResults.Reset();
	int32 HardCount = 0;
	int32 SoftCount = 0;
	for (const FCookViolation& V : Found)
	{
		ValidationResults.Add(MakeShared<FCookViolation>(V));
		if (V.bHardRef) { ++HardCount; } else { ++SoftCount; }
	}

	if (bCancelled)
	{
		ValidationSummary = FText::Format(
			LOCTEXT("ValidateCancelled", "Validation cancelled — {0} partial result(s) ({1} hard, {2} soft)."),
			FText::AsNumber(Found.Num()),
			FText::AsNumber(HardCount),
			FText::AsNumber(SoftCount));
	}
	else if (Found.IsEmpty())
	{
		ValidationSummary = LOCTEXT("ValidateClean", "No violations: nothing being cooked references NeverCook content.");
	}
	else
	{
		ValidationSummary = FText::Format(
			LOCTEXT("ValidateFound", "{0} violation(s): {1} hard (cook fails / broken ref), {2} soft (runtime null risk)."),
			FText::AsNumber(Found.Num()),
			FText::AsNumber(HardCount),
			FText::AsNumber(SoftCount));
	}

	if (ResultsView.IsValid())
	{
		ResultsView->RequestListRefresh();
	}

	return FReply::Handled();
}

FReply SAssetCookManagerPanel::OnExportClicked()
{
	if (!HasResults())
	{
		return FReply::Handled();
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		return FReply::Handled();
	}

	void* ParentWindowHandle = nullptr;
	if (FSlateApplication::IsInitialized())
	{
		TSharedPtr<SWindow> Window = FSlateApplication::Get().FindWidgetWindow(AsShared());
		if (Window.IsValid() && Window->GetNativeWindow().IsValid())
		{
			ParentWindowHandle = Window->GetNativeWindow()->GetOSWindowHandle();
		}
	}

	TArray<FString> OutFiles;
	const bool bPicked = DesktopPlatform->SaveFileDialog(
		ParentWindowHandle,
		LOCTEXT("ExportDialogTitle", "Export Cook Violations").ToString(),
		FPaths::ProjectSavedDir(),
		TEXT("CookViolations.csv"),
		TEXT("CSV (*.csv)|*.csv"),
		EFileDialogFlags::None,
		OutFiles);

	if (!bPicked || OutFiles.Num() == 0)
	{
		return FReply::Handled();
	}

	FString Csv = TEXT("Source,Referenced,NeverCookDir,RefType\n");
	for (const TSharedPtr<FCookViolation>& Item : ValidationResults)
	{
		if (!Item.IsValid())
		{
			continue;
		}
		Csv += FString::Printf(TEXT("%s,%s,%s,%s\n"),
			*EscapeCsvField(Item->SourcePackage),
			*EscapeCsvField(Item->ReferencedPackage),
			*EscapeCsvField(Item->NeverCookDir),
			Item->bHardRef ? TEXT("Hard") : TEXT("Soft"));
	}

	if (FFileHelper::SaveStringToFile(Csv, *OutFiles[0]))
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(LOCTEXT("ExportOk", "Exported {0} row(s) to:\n{1}"),
				FText::AsNumber(ValidationResults.Num()),
				FText::FromString(OutFiles[0])));
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("ExportFail", "Failed to write the CSV file."));
	}

	return FReply::Handled();
}

bool SAssetCookManagerPanel::IsRedundant(const FString& PackagePath) const
{
	// Explicit rule on this dir (pending edit or saved).
	const ECookRuleType Self = GetEffectiveRule(PackagePath);
	if (Self == ECookRuleType::Default)
	{
		return false;
	}

	FString Parent;
	FString Leaf;
	if (!PackagePath.Split(TEXT("/"), &Parent, &Leaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd)
		|| Parent.IsEmpty())
	{
		return false;
	}

	// Pending-aware rule inherited from the parent chain.
	const FResolvedCookRule Inherited = GetDisplayRule(Parent);
	return Inherited.Rule != ECookRuleType::Default && Inherited.Rule == Self;
}

FReply SAssetCookManagerPanel::OnCleanClicked()
{
	if (HasPendingChanges())
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("CleanPending", "Apply or refresh pending rule changes before cleaning redundant saved rules."));
		return FReply::Handled();
	}

	TArray<FString> Redundant;
	FAssetCookRuleManager::GetRedundantDirectories(Redundant);

	if (Redundant.Num() == 0)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("CleanNone", "No redundant rules — DefaultGame.ini is already lean."));
		return FReply::Handled();
	}

	const FText Confirm = FText::Format(
		LOCTEXT("CleanConfirm", "Remove {0} redundant rule(s) already implied by a parent directory?"),
		FText::AsNumber(Redundant.Num()));
	if (FMessageDialog::Open(EAppMsgType::YesNo, Confirm) != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	TMap<FString, ECookRuleType> ToClear;
	for (const FString& Dir : Redundant)
	{
		ToClear.Add(Dir, ECookRuleType::Default);
	}
	FAssetCookRuleManager::ApplyRules(ToClear);

	PendingRules.Reset();
	RefreshTree();

	FMessageDialog::Open(EAppMsgType::Ok,
		FText::Format(LOCTEXT("CleanDone", "Removed {0} redundant rule(s)."),
			FText::AsNumber(Redundant.Num())));

	return FReply::Handled();
}


TSharedRef<ITableRow> SAssetCookManagerPanel::OnGenerateViolationRow(TSharedPtr<FCookViolation> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FText RowText = FText::Format(
		LOCTEXT("ViolationRow", "[{0}]  {1}  ->  {2}   (NeverCook: {3})"),
		FText::FromString(Item->bHardRef ? TEXT("HARD") : TEXT("SOFT")),
		FText::FromString(Item->SourcePackage),
		FText::FromString(Item->ReferencedPackage),
		FText::FromString(Item->NeverCookDir));

	// Hard references in red (break the cook); soft references in amber (runtime risk).
	const FLinearColor RowColor = Item->bHardRef
		? FLinearColor(1.0f, 0.5f, 0.4f)
		: FLinearColor(1.0f, 0.75f, 0.3f);

	return SNew(STableRow<TSharedPtr<FCookViolation>>, OwnerTable)
	.ToolTipText(LOCTEXT("ViolationTip", "Double-click to show the source asset in the Content Browser."))
	[
		SNew(STextBlock)
		.Text(RowText)
		.ColorAndOpacity(FSlateColor(RowColor))
	];
}

void SAssetCookManagerPanel::OnViolationDoubleClick(TSharedPtr<FCookViolation> Item)
{
	if (!Item.IsValid())
	{
		return;
	}

	IAssetRegistry& Registry = GetPanelAssetRegistry();
	TArray<FAssetData> Assets;
	Registry.GetAssetsByPackageName(FName(*Item->SourcePackage), Assets);
	if (Assets.Num() == 0)
	{
		return;
	}

	FContentBrowserModule& ContentBrowser =
		FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	IContentBrowserSingleton& Singleton = ContentBrowser.Get();
	Singleton.SyncBrowserToAssets(Assets);
	Singleton.FocusPrimaryContentBrowser(/*bFocusSearch=*/false);
}


TSharedPtr<SWidget> SAssetCookManagerPanel::OnTreeContextMenu()
{
	if (!TreeView.IsValid() || TreeView->GetNumItemsSelected() == 0)
	{
		return nullptr;
	}

	FMenuBuilder MenuBuilder(/*bCloseAfterSelection=*/true, nullptr);

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("SetRuleSection", "Set Cook Rule"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("SetAlways", "Always Cook"),
			LOCTEXT("SetAlwaysTip", "Force selected directories into the package."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SAssetCookManagerPanel::SetRuleForSelection, ECookRuleType::AlwaysCook)));

		MenuBuilder.AddMenuEntry(
			LOCTEXT("SetNever", "Never Cook"),
			LOCTEXT("SetNeverTip", "Exclude selected directories from the package."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SAssetCookManagerPanel::SetRuleForSelection, ECookRuleType::NeverCook)));

		MenuBuilder.AddMenuEntry(
			LOCTEXT("SetDefault", "Default (clear)"),
			LOCTEXT("SetDefaultTip", "Remove any explicit rule from selected directories."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SAssetCookManagerPanel::SetRuleForSelection, ECookRuleType::Default)));
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection(NAME_None, LOCTEXT("AnalyzeSection", "Analyze"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("FindRefs", "Find Referencers"),
			LOCTEXT("FindRefsTip", "List packaged assets that reference the first selected directory."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SAssetCookManagerPanel::RunFindReferencers)));
	}
	MenuBuilder.EndSection();
	MenuBuilder.BeginSection(NAME_None, LOCTEXT("NavSection", "Navigate"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ShowInCB", "Show in Content Browser"),
			LOCTEXT("ShowInCBTip", "Select these folders in the Content Browser."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this]()
			{
				TArray<FString> Folders;
				for (const TSharedPtr<FCookDirNode>& Node : TreeView->GetSelectedItems())
				{
					if (Node.IsValid()) { Folders.Add(Node->PackagePath); }
				}
				if (Folders.Num() > 0)
				{
					FContentBrowserModule& CB =
						FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
					CB.Get().SyncBrowserToFolders(Folders);
					CB.Get().FocusPrimaryContentBrowser(false);
				}
			})));

		MenuBuilder.AddMenuEntry(
			LOCTEXT("CopyPath", "Copy Path"),
			LOCTEXT("CopyPathTip", "Copy the selected package path(s) to the clipboard."),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateLambda([this]()
			{
				TArray<FString> Paths;
				for (const TSharedPtr<FCookDirNode>& Node : TreeView->GetSelectedItems())
				{
					if (Node.IsValid()) { Paths.Add(Node->PackagePath); }
				}
				if (Paths.Num() > 0)
				{
					FPlatformApplicationMisc::ClipboardCopy(*FString::Join(Paths, TEXT("\n")));
				}
			})));
	}
	MenuBuilder.EndSection();

	return MenuBuilder.MakeWidget();
}


void SAssetCookManagerPanel::RunFindReferencers()
{
	if (!TreeView.IsValid())
	{
		return;
	}

	const TArray<TSharedPtr<FCookDirNode>> Selected = TreeView->GetSelectedItems();
	if (Selected.Num() == 0 || !Selected[0].IsValid())
	{
		return;
	}

	const FString Dir = Selected[0]->PackagePath;

	FScopedSlowTask Task(0.0f, LOCTEXT("FindingRefs", "Finding referencers..."));
	Task.MakeDialog();

	const TArray<FCookViolation> Found = FAssetCookScanner::FindReferencers(Dir);

	ValidationResults.Reset();
	for (const FCookViolation& V : Found)
	{
		ValidationResults.Add(MakeShared<FCookViolation>(V));
	}

	ValidationSummary = Found.Num() == 0
		? FText::Format(LOCTEXT("NoRefs", "Nothing references {0}."), FText::FromString(Dir))
		: FText::Format(LOCTEXT("RefsFound", "{0} reference(s) to {1} (red = hard, amber = soft)."),
			FText::AsNumber(Found.Num()), FText::FromString(Dir));

	if (ResultsView.IsValid())
	{
		ResultsView->RequestListRefresh();
	}
}

#undef LOCTEXT_NAMESPACE
