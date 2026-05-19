#include "SUIStateConfigPanel.h"
#include "ScopedTransaction.h"

#include "AssetRegistry/AssetData.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Framework/Application/SlateApplication.h"

#include "IDetailsView.h"
#include "InputCoreTypes.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"

#include "Styling/AppStyle.h"
#include "UMGStateConfigBlueprintExtension.h"
#include "UMGStateConfigDetailsProxy.h"
#include "UMGStateConfigPropertyRuntimeLibrary.h"
#include "UMGStateConfigValidator.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprint.h"


#include "WidgetBlueprintEditor.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "UMGStateConfigPanel"

namespace
{
FName MakeUniqueStateGroupName(const TArray<FUMGStateConfigGroup>& StateGroups)
{
	for (int32 Index = 1;; ++Index)
	{
		const FName Candidate(*FString::Printf(TEXT("StateGroup%d"), Index));
		if (!StateGroups.ContainsByPredicate([Candidate](const FUMGStateConfigGroup& Group)
		{
			return Group.GroupName == Candidate;
		}))
		{
			return Candidate;
		}
	}
}

FName MakeUniqueStateName(const FUMGStateConfigGroup& Group)
{
	for (int32 Index = 1;; ++Index)
	{
		const FName Candidate(*FString::Printf(TEXT("State%d"), Index));
		if (!Group.States.ContainsByPredicate([Candidate](const FUMGStateConfigState& State)
		{
			return State.StateName == Candidate;
		}))
		{
			return Candidate;
		}
	}
}

struct FUMGStateConfigDetailsProxyDescriptor
{
	TSubclassOf<UUMGStateConfigDetailsProxyBase> ProxyClass;
	TSubclassOf<UWidget> DefaultExpectedClass;
	bool bUseWidgetSpecificExpectedClass = false;
};

TOptional<FUMGStateConfigDetailsProxyDescriptor> GetDetailsProxyDescriptor(EUMGStateConfigPropertyType PropertyType)
{
	switch (PropertyType)
	{
	case EUMGStateConfigPropertyType::Visibility:
		return FUMGStateConfigDetailsProxyDescriptor{ UUMGStateConfigVisibilityProxy::StaticClass(), UWidget::StaticClass(), false };
	case EUMGStateConfigPropertyType::RenderOpacity:
		return FUMGStateConfigDetailsProxyDescriptor{ UUMGStateConfigRenderOpacityProxy::StaticClass(), UWidget::StaticClass(), false };
	case EUMGStateConfigPropertyType::Text:
		return FUMGStateConfigDetailsProxyDescriptor{ UUMGStateConfigTextContentProxy::StaticClass(), UWidget::StaticClass(), true };
	case EUMGStateConfigPropertyType::ImageAppearance:
		return FUMGStateConfigDetailsProxyDescriptor{ UUMGStateConfigImageAppearanceProxy::StaticClass(), UImage::StaticClass(), false };
	case EUMGStateConfigPropertyType::TextAppearance:
		return FUMGStateConfigDetailsProxyDescriptor{ UUMGStateConfigTextAppearanceProxy::StaticClass(), UTextBlock::StaticClass(), false };
	default:
		return TOptional<FUMGStateConfigDetailsProxyDescriptor>();
	}
}
}


void SUIStateConfigPanel::Construct(const FArguments& InArgs, TSharedPtr<FWidgetBlueprintEditor> InWidgetEditor)
{

	WidgetEditor = InWidgetEditor;
	RebuildWidgetRows();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(this, &SUIStateConfigPanel::GetTitleText)
					.Font(FAppStyle::GetFontStyle("HeadingExtraSmall"))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("AddParentState", "+ 父状态"))
					.OnClicked(this, &SUIStateConfigPanel::AddParentState)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("DeleteParentState", "- 父状态"))
					.ToolTipText(LOCTEXT("DeleteParentStateTip", "删除当前选中的父状态配置。"))
					.OnClicked(this, &SUIStateConfigPanel::DeleteParentState)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f)
			[
				SAssignNew(ParentTabsBox, SVerticalBox)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 6.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SAssignNew(ChildTabsBox, SVerticalBox)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("AddChildState", "+ 子状态"))
					.OnClicked(this, &SUIStateConfigPanel::AddChildState)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("DeleteChildState", "- 子状态"))
					.ToolTipText(LOCTEXT("DeleteChildStateTip", "删除当前选中的子状态配置。"))
					.OnClicked(this, &SUIStateConfigPanel::DeleteChildState)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock)
				.Text(this, &SUIStateConfigPanel::GetBreadcrumbText)
			]
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.28f)
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock).Text(LOCTEXT("AvailableWidgets", "可加入控件（右键选择属性）"))

					]
					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					[
						SAssignNew(WidgetListView, SListView<TSharedPtr<FUMGStateConfigWidgetRow>>)
						.ListItemsSource(&WidgetRows)
						.OnGenerateRow(this, &SUIStateConfigPanel::GenerateWidgetRow)
						.OnSelectionChanged(this, &SUIStateConfigPanel::OnWidgetSelected)
						.OnMouseButtonDoubleClick(this, &SUIStateConfigPanel::OnWidgetDoubleClicked)
					]

				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.72f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SAssignNew(ConfiguredWidgetsBox, SVerticalBox)
					]
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 8.0f, 0.0f, 0.0f)
			[
				SNew(SSeparator)
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 6.0f, 0.0f, 0.0f)
			[
				SAssignNew(SummaryTextBlock, STextBlock)
				.AutoWrapText(true)
				.Text(this, &SUIStateConfigPanel::GetSummaryText)
			]
		]
	];

	RefreshAll();
	ApplyPreviewState();
}

SUIStateConfigPanel::~SUIStateConfigPanel()
{
	ResetDesignerPreview();
}

UWidgetBlueprint* SUIStateConfigPanel::GetWidgetBlueprint() const
{
	TSharedPtr<FWidgetBlueprintEditor> Editor = WidgetEditor.Pin();
	return Editor.IsValid() ? Editor->GetWidgetBlueprintObj() : nullptr;
}

UUMGStateConfigBlueprintExtension* SUIStateConfigPanel::GetOrCreateExtension()
{
	return UUMGStateConfigBlueprintExtension::Request(GetWidgetBlueprint());
}

const UUMGStateConfigBlueprintExtension* SUIStateConfigPanel::GetExtension() const
{
	return UUMGStateConfigBlueprintExtension::Find(GetWidgetBlueprint());
}

FUMGStateConfigGroup* SUIStateConfigPanel::GetActiveGroup()
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	if (!Extension || Extension->ConfigData.StateGroups.Num() == 0)
	{
		return nullptr;
	}

	if (SelectedGroupName.IsNone())
	{
		SelectedGroupName = Extension->ConfigData.PreviewStateGroupName.IsNone()
			? Extension->ConfigData.StateGroups[0].GroupName
			: Extension->ConfigData.PreviewStateGroupName;
	}

	FUMGStateConfigGroup* Group = Extension->ConfigData.StateGroups.FindByPredicate([this](const FUMGStateConfigGroup& Candidate)
	{
		return Candidate.GroupName == SelectedGroupName;
	});
	return Group ? Group : &Extension->ConfigData.StateGroups[0];
}

FUMGStateConfigState* SUIStateConfigPanel::GetActiveState()
{
	FUMGStateConfigGroup* Group = GetActiveGroup();
	if (!Group || Group->States.Num() == 0)
	{
		return nullptr;
	}

	if (SelectedStateName.IsNone())
	{
		SelectedStateName = Group->DefaultStateName.IsNone() ? Group->States[0].StateName : Group->DefaultStateName;
	}

	FUMGStateConfigState* State = Group->States.FindByPredicate([this](const FUMGStateConfigState& Candidate)
	{
		return Candidate.StateName == SelectedStateName;
	});
	return State ? State : &Group->States[0];
}

UWidget* SUIStateConfigPanel::FindWidgetByName(FName WidgetName) const
{
	const UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	return WidgetBlueprint && WidgetBlueprint->WidgetTree ? WidgetBlueprint->WidgetTree->FindWidget(WidgetName) : nullptr;
}

FText SUIStateConfigPanel::GetTitleText() const
{
	const UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	return WidgetBlueprint
		? FText::Format(LOCTEXT("PanelTitle", "UI 状态配置 · {0}"), FText::FromString(WidgetBlueprint->GetName()))
		: LOCTEXT("PanelTitleNoBlueprint", "UI 状态配置");
}

FText SUIStateConfigPanel::GetSummaryText() const
{
	return CachedSummaryText;
}


FText SUIStateConfigPanel::GetBreadcrumbText() const
{
	return FText::Format(LOCTEXT("Breadcrumb", "当前：{0} → {1}"), FText::FromName(SelectedGroupName), FText::FromName(SelectedStateName));
}

FReply SUIStateConfigPanel::EnsureDefaultConfig()
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("EnsureDefaultConfig", "初始化配置"), nullptr);
	UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	if (!WidgetBlueprint)
	{
		return FReply::Handled();
	}

	WidgetBlueprint->Modify();
	UUMGStateConfigBlueprintExtension* Extension = UUMGStateConfigBlueprintExtension::Request(WidgetBlueprint);
	if (!Extension)
	{
		return FReply::Handled();
	}

	Extension->Modify();
	if (Extension->ConfigData.StateGroups.Num() == 0)
	{
		FUMGStateConfigGroup Group;
		Group.GroupName = TEXT("RewardItemState");
		Group.DisplayName = LOCTEXT("RewardItemStateDisplayName", "奖励道具状态");
		Group.DefaultStateName = TEXT("Normal");

		FUMGStateConfigState Normal;
		Normal.StateName = TEXT("Normal");
		Normal.DisplayName = LOCTEXT("NormalStateDisplayName", "普通");

		FUMGStateConfigState CanClaim;
		CanClaim.StateName = TEXT("CanClaim");
		CanClaim.DisplayName = LOCTEXT("CanClaimStateDisplayName", "可领取");

		FUMGStateConfigState Claimed;
		Claimed.StateName = TEXT("Claimed");
		Claimed.DisplayName = LOCTEXT("ClaimedStateDisplayName", "已领取");

		Group.States = { Normal, CanClaim, Claimed };
		Extension->ConfigData.StateGroups.Add(Group);
		Extension->ConfigData.PreviewStateGroupName = Group.GroupName;
		Extension->ConfigData.PreviewStateName = Group.DefaultStateName;
		SelectedGroupName = Group.GroupName;
		SelectedStateName = Group.DefaultStateName;
	}

	MarkConfigDirty(Extension);
	RefreshAll();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::ApplyPreviewState()
{
	const UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	const UUMGStateConfigBlueprintExtension* Extension = UUMGStateConfigBlueprintExtension::Find(WidgetBlueprint);
	if (!WidgetBlueprint || !Extension)
	{
		return FReply::Handled();
	}

	const FUMGStateConfigGroup* Group = Extension->ConfigData.StateGroups.FindByPredicate([Extension](const FUMGStateConfigGroup& Candidate)
	{
		return Candidate.GroupName == Extension->ConfigData.PreviewStateGroupName;
	});
	const FUMGStateConfigState* State = Group ? Group->States.FindByPredicate([Extension](const FUMGStateConfigState& Candidate)
	{
		return Candidate.StateName == Extension->ConfigData.PreviewStateName;
	}) : nullptr;
	if (!State)
	{
		ResetDesignerPreview();
		return FReply::Handled();
	}

	TSharedPtr<FWidgetBlueprintEditor> Editor = WidgetEditor.Pin();
	if (!Editor.IsValid())
	{
		return FReply::Handled();
	}

	Editor->RefreshPreview();
	UUserWidget* PreviewWidget = Editor->GetPreview();
	for (const FUMGStatePropertyChange& Change : State->PropertyChanges)
	{
		UWidget* PreviewChildWidget = PreviewWidget && PreviewWidget->WidgetTree ? PreviewWidget->WidgetTree->FindWidget(Change.TargetWidgetName) : nullptr;
		FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(PreviewChildWidget, Change.PropertyType, Change.Value);
	}

	Editor->InvalidatePreview(true);
	FSlateApplication::Get().InvalidateAllWidgets(false);
	return FReply::Handled();
}

void SUIStateConfigPanel::ResetDesignerPreview() const
{
	TSharedPtr<FWidgetBlueprintEditor> Editor = WidgetEditor.Pin();
	if (Editor.IsValid())
	{
		Editor->RefreshPreview();
		Editor->InvalidatePreview(true);
		FSlateApplication::Get().InvalidateAllWidgets(false);
	}
}

FReply SUIStateConfigPanel::AddParentState()
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("AddParentState", "添加父状态"), nullptr);
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	if (!Extension)
	{
		return FReply::Handled();
	}

	FUMGStateConfigGroup Group;
	Group.GroupName = MakeUniqueStateGroupName(Extension->ConfigData.StateGroups);
	Group.DisplayName = FText::FromName(Group.GroupName);
	Group.DefaultStateName = TEXT("Normal");

	FUMGStateConfigState State;
	State.StateName = TEXT("Normal");
	State.DisplayName = LOCTEXT("NormalStateDisplayName", "普通");
	Group.States.Add(State);

	Extension->ConfigData.StateGroups.Add(Group);
	SelectedGroupName = Group.GroupName;
	SelectedStateName = State.StateName;
	Extension->ConfigData.PreviewStateGroupName = SelectedGroupName;
	Extension->ConfigData.PreviewStateName = SelectedStateName;
	MarkConfigDirty(Extension);
	RefreshAll();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::DeleteParentState()
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("DeleteParentState", "删除父状态"), nullptr);
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	if (!Extension || SelectedGroupName.IsNone())
	{
		return FReply::Handled();
	}

	const int32 RemovedIndex = Extension->ConfigData.StateGroups.IndexOfByPredicate([this](const FUMGStateConfigGroup& Group)
	{
		return Group.GroupName == SelectedGroupName;
	});
	if (RemovedIndex == INDEX_NONE)
	{
		return FReply::Handled();
	}

	Extension->ConfigData.StateGroups.RemoveAt(RemovedIndex);
	if (Extension->ConfigData.StateGroups.Num() > 0)
	{
		const int32 NextIndex = FMath::Clamp(RemovedIndex, 0, Extension->ConfigData.StateGroups.Num() - 1);
		const FUMGStateConfigGroup& NextGroup = Extension->ConfigData.StateGroups[NextIndex];
		SelectedGroupName = NextGroup.GroupName;
		SelectedStateName = NextGroup.States.Num() > 0 ? NextGroup.States[0].StateName : NAME_None;
	}
	else
	{
		SelectedGroupName = NAME_None;
		SelectedStateName = NAME_None;
	}
	Extension->ConfigData.PreviewStateGroupName = SelectedGroupName;
	Extension->ConfigData.PreviewStateName = SelectedStateName;
	MarkConfigDirty(Extension);
	ApplyPreviewState();
	RefreshAll();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::AddChildState()
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("AddChildState", "添加子状态"), nullptr);
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigGroup* Group = GetActiveGroup();
	if (!Extension || !Group)
	{
		return FReply::Handled();
	}

	FUMGStateConfigState State;
	State.StateName = MakeUniqueStateName(*Group);
	State.DisplayName = FText::FromName(State.StateName);
	for (const FUMGStateConfigState& ExistingState : Group->States)
	{
		for (FName WidgetName : ExistingState.ConfiguredWidgetNames)
		{
			State.ConfiguredWidgetNames.AddUnique(WidgetName);
		}
	}
	Group->States.Add(State);
	SelectedStateName = State.StateName;
	Extension->ConfigData.PreviewStateGroupName = Group->GroupName;
	Extension->ConfigData.PreviewStateName = SelectedStateName;
	MarkConfigDirty(Extension);
	RefreshAll();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::DeleteChildState()
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("DeleteChildState", "删除子状态"), nullptr);
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigGroup* Group = GetActiveGroup();
	if (!Extension || !Group || SelectedStateName.IsNone())
	{
		return FReply::Handled();
	}

	const int32 RemovedIndex = Group->States.IndexOfByPredicate([this](const FUMGStateConfigState& State)
	{
		return State.StateName == SelectedStateName;
	});
	if (RemovedIndex == INDEX_NONE)
	{
		return FReply::Handled();
	}

	const bool bDeletedDefaultState = Group->DefaultStateName == SelectedStateName;
	Group->States.RemoveAt(RemovedIndex);
	if (bDeletedDefaultState)
	{
		Group->DefaultStateName = Group->States.Num() > 0 ? Group->States[0].StateName : NAME_None;
	}
	if (Group->States.Num() > 0)
	{
		const int32 NextIndex = FMath::Clamp(RemovedIndex, 0, Group->States.Num() - 1);
		SelectedStateName = Group->States[NextIndex].StateName;
	}
	else
	{
		SelectedStateName = NAME_None;
	}
	Extension->ConfigData.PreviewStateGroupName = Group->GroupName;
	Extension->ConfigData.PreviewStateName = SelectedStateName;
	MarkConfigDirty(Extension);
	ApplyPreviewState();
	RefreshAll();
	return FReply::Handled();
}


FReply SUIStateConfigPanel::ClearWidgetConfig(FName WidgetName)
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("ClearWidgetConfig", "清除控件配置"), nullptr);
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigState* State = GetActiveState();
	if (!Extension || !State)
	{
		return FReply::Handled();
	}

	State->ConfiguredWidgetNames.Remove(WidgetName);
	State->PropertyChanges.RemoveAll([WidgetName](const FUMGStatePropertyChange& Change)
	{
		return Change.TargetWidgetName == WidgetName;
	});
	MarkConfigDirty(Extension);
	ApplyPreviewState();
	RefreshAll();
	return FReply::Handled();
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildParentStateTabs()
{
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
	const UUMGStateConfigBlueprintExtension* Extension = GetExtension();
	if (!Extension)
	{
		return Row;
	}

	for (const FUMGStateConfigGroup& Group : Extension->ConfigData.StateGroups)
	{
		Row->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			BuildStateTab(Group.GroupName, Group.GroupName == SelectedGroupName, true)
		];
	}
	return Row;
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildChildStateTabs()
{
	TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
	FUMGStateConfigGroup* Group = GetActiveGroup();
	if (!Group)
	{
		return Row;
	}

	for (const FUMGStateConfigState& State : Group->States)
	{
		Row->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			BuildStateTab(State.StateName, State.StateName == SelectedStateName, false)
		];
	}
	return Row;
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildStateTab(FName StateName, bool bSelected, bool bIsParentState)
{
	return SNew(SBorder)
	.Padding(FMargin(10.0f, 3.0f))
	.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
	.BorderBackgroundColor(bSelected ? FLinearColor(0.25f, 0.45f, 0.9f, 1.0f) : FLinearColor(0.08f, 0.08f, 0.08f, 1.0f))
	.OnMouseButtonDown(this, &SUIStateConfigPanel::HandleStateTabMouseButtonDown, StateName, bIsParentState)
	[
		SNew(STextBlock)
		.Text(FText::FromName(StateName))
	];
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildRenameMenu(FName OldName, bool bIsParentState)
{
	return SNew(SBorder)
	.Padding(8.0f)
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock).Text(bIsParentState ? LOCTEXT("RenameParentState", "重命名父状态") : LOCTEXT("RenameChildState", "重命名子状态"))
		]
		+ SVerticalBox::Slot().AutoHeight()
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(180.0f)
			.Text(FText::FromName(OldName))
			.SelectAllTextWhenFocused(true)
			.OnTextCommitted_Lambda([this, OldName, bIsParentState](const FText& NewText, ETextCommit::Type CommitType)
			{
				if (CommitType == ETextCommit::OnCleared)
				{
					return;
				}
				const FString NewNameString = NewText.ToString().TrimStartAndEnd();
				if (NewNameString.IsEmpty())
				{
					return;
				}
				const FName NewName(*NewNameString);
				if (bIsParentState)
				{
					RenameParentState(OldName, NewName);
				}
				else
				{
					RenameChildState(OldName, NewName);
				}
				FSlateApplication::Get().DismissAllMenus();
			})
		]
	];
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildAvailableWidgetPropertyMenu(FName WidgetName)
{
	UWidget* Widget = FindWidgetByName(WidgetName);
	const TArray<EUMGStateConfigPropertyType> PropertyTypes = GetSupportedPropertyTypes(Widget);
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	Box->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock).Text(FText::Format(LOCTEXT("SelectWidgetProperty", "选择 {0} 要修改的属性"), FText::FromName(WidgetName)))
	];

	if (!GetActiveState())
	{
		Box->AddSlot().AutoHeight()
		[
			SNew(STextBlock).Text(LOCTEXT("NoActiveStateForPropertyMenu", "当前没有选中的子状态。"))
		];
		return SNew(SBorder).Padding(8.0f)[Box];
	}

	for (EUMGStateConfigPropertyType PropertyType : PropertyTypes)
	{
		Box->AddSlot().AutoHeight().Padding(0.0f, 2.0f)
		[
			SNew(SButton)
			.Text(GetPropertyLabel(PropertyType))
			.OnClicked_Lambda([this, WidgetName, PropertyType]()
			{
				AddWidgetPropertyToActiveState(WidgetName, PropertyType);
				FSlateApplication::Get().DismissAllMenus();
				return FReply::Handled();
			})
		];
	}

	Box->AddSlot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
	[
		SNew(SSeparator)
	];
	Box->AddSlot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("AddFromDetails", "从 Details 添加属性"))
		.ToolTipText(LOCTEXT("AddFromDetailsTip", "打开该控件的临时 Details 面板，修改任意属性后会记录该属性的序列化值。"))
		.OnClicked_Lambda([this, WidgetName]()
		{
			OpenWidgetDetailsPropertyPicker(WidgetName);
			return FReply::Handled();
		})
	];

	return SNew(SBorder).Padding(8.0f)[Box];
}


TArray<EUMGStateConfigPropertyType> SUIStateConfigPanel::GetSupportedPropertyTypes(const UWidget* Widget) const
{
	TArray<EUMGStateConfigPropertyType> PropertyTypes;
	if (!Widget)
	{
		return PropertyTypes;
	}

	PropertyTypes.Add(EUMGStateConfigPropertyType::Visibility);
	PropertyTypes.Add(EUMGStateConfigPropertyType::RenderOpacity);
	if (IsWidgetText(Widget))
	{
		PropertyTypes.Add(EUMGStateConfigPropertyType::Text);
		if (Widget->IsA<UTextBlock>())
		{
			PropertyTypes.Add(EUMGStateConfigPropertyType::TextAppearance);
		}
	}
	if (IsWidgetImage(Widget))
	{
		PropertyTypes.Add(EUMGStateConfigPropertyType::ImageAppearance);
	}
	return PropertyTypes;
}

FText SUIStateConfigPanel::GetPropertyLabel(EUMGStateConfigPropertyType PropertyType) const
{
	switch (PropertyType)
	{
	case EUMGStateConfigPropertyType::Visibility:
		return LOCTEXT("PropertyLabelVisibility", "Visibility");
	case EUMGStateConfigPropertyType::RenderOpacity:
		return LOCTEXT("PropertyLabelRenderOpacity", "Render Opacity");
	case EUMGStateConfigPropertyType::Text:
		return LOCTEXT("PropertyLabelText", "Content / Text");
	case EUMGStateConfigPropertyType::TextAppearance:
		return LOCTEXT("PropertyLabelTextAppearance", "外观（Appearance）");
	case EUMGStateConfigPropertyType::TextColor:
		return LOCTEXT("PropertyLabelTextColor", "外观 / Color and Opacity");
	case EUMGStateConfigPropertyType::BrushImage:
		return LOCTEXT("PropertyLabelBrushImage", "外观 / Brush 资源");
	case EUMGStateConfigPropertyType::BrushTint:
		return LOCTEXT("PropertyLabelBrushTint", "外观 / Color and Opacity");
	case EUMGStateConfigPropertyType::ImageAppearance:
		return LOCTEXT("PropertyLabelImageAppearance", "外观（Appearance）");
	case EUMGStateConfigPropertyType::SerializedProperty:
		return LOCTEXT("PropertyLabelSerializedProperty", "Details 属性");
	default:
		return LOCTEXT("PropertyLabelUnknown", "Unknown");
	}
}


TSubclassOf<UWidget> SUIStateConfigPanel::GetExpectedWidgetClass(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const
{
	UWidget* Widget = FindWidgetByName(WidgetName);
	switch (PropertyType)
	{
	case EUMGStateConfigPropertyType::Visibility:
	case EUMGStateConfigPropertyType::RenderOpacity:
		return UWidget::StaticClass();
	case EUMGStateConfigPropertyType::Text:
		return Widget && Widget->IsA<URichTextBlock>() ? URichTextBlock::StaticClass() : UTextBlock::StaticClass();
	case EUMGStateConfigPropertyType::TextAppearance:
	case EUMGStateConfigPropertyType::TextColor:
		return UTextBlock::StaticClass();
	case EUMGStateConfigPropertyType::BrushImage:
	case EUMGStateConfigPropertyType::BrushTint:
	case EUMGStateConfigPropertyType::ImageAppearance:
		return UImage::StaticClass();
	case EUMGStateConfigPropertyType::SerializedProperty:
		return Widget ? Widget->GetClass() : UWidget::StaticClass();
	default:
		return UWidget::StaticClass();
	}
}


void SUIStateConfigPanel::RenameParentState(FName OldName, FName NewName)

{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("RenameParentState", "重命名父状态"), nullptr);
	if (OldName == NewName)
	{
		return;
	}
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	if (!Extension || Extension->ConfigData.StateGroups.ContainsByPredicate([NewName](const FUMGStateConfigGroup& Group) { return Group.GroupName == NewName; }))
	{
		return;
	}
	if (FUMGStateConfigGroup* Group = Extension->ConfigData.StateGroups.FindByPredicate([OldName](const FUMGStateConfigGroup& Candidate) { return Candidate.GroupName == OldName; }))
	{
		Group->GroupName = NewName;
		if (Extension->ConfigData.PreviewStateGroupName == OldName)
		{
			Extension->ConfigData.PreviewStateGroupName = NewName;
		}
		SelectedGroupName = NewName;
		MarkConfigDirty(Extension);
		RefreshAll();
	}
}

void SUIStateConfigPanel::RenameChildState(FName OldName, FName NewName)
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("RenameChildState", "重命名子状态"), nullptr);
	if (OldName == NewName)
	{
		return;
	}
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigGroup* Group = GetActiveGroup();
	if (!Extension || !Group || Group->States.ContainsByPredicate([NewName](const FUMGStateConfigState& State) { return State.StateName == NewName; }))
	{
		return;
	}
	if (FUMGStateConfigState* State = Group->States.FindByPredicate([OldName](const FUMGStateConfigState& Candidate) { return Candidate.StateName == OldName; }))
	{
		State->StateName = NewName;
		if (Group->DefaultStateName == OldName)
		{
			Group->DefaultStateName = NewName;
		}
		if (Extension->ConfigData.PreviewStateName == OldName)
		{
			Extension->ConfigData.PreviewStateName = NewName;
		}
		SelectedStateName = NewName;
		MarkConfigDirty(Extension);
		RefreshAll();
	}
}

FReply SUIStateConfigPanel::HandleStateTabMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent, FName StateName, bool bIsParentState)
{
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		FSlateApplication::Get().PushMenu(
			AsShared(),
			FWidgetPath(),
			BuildRenameMenu(StateName, bIsParentState),
			MouseEvent.GetScreenSpacePosition(),
			FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
		return FReply::Handled();
	}

	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (bIsParentState)
		{
			SelectParentState(StateName);
		}
		else
		{
			SelectChildState(StateName);
		}
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildConfiguredWidgetCards()
{
	TSharedRef<SVerticalBox> Box = SNew(SVerticalBox);
	const TArray<FName> WidgetNames = GetConfiguredWidgetNames();
	if (WidgetNames.Num() == 0)
	{
		Box->AddSlot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(STextBlock).Text(LOCTEXT("NoConfiguredWidget", "当前子状态还没有控件配置。请双击左侧控件加入当前父状态的所有子状态。"))
		];
		return Box;
	}

	for (int32 Index = 0; Index < WidgetNames.Num(); ++Index)
	{
		Box->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			BuildConfiguredWidgetCard(WidgetNames[Index])
		];
		if (Index < WidgetNames.Num() - 1)
		{
			Box->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
			[
				SNew(SSeparator)
			];
		}
	}
	return Box;
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildConfiguredWidgetCard(FName WidgetName)
{
	UWidget* Widget = FindWidgetByName(WidgetName);
	TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
	Rows->AddSlot().AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("%s    %s"), *WidgetName.ToString(), Widget ? *Widget->GetClass()->GetName() : TEXT("Missing"))))
			.Font(FAppStyle::GetFontStyle("BoldFont"))
		]
		+ SHorizontalBox::Slot().AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ClearWidgetConfig", "清除"))
			.ToolTipText(LOCTEXT("ClearWidgetConfigTip", "清除当前子状态中该控件的全部配置。"))
			.OnClicked(this, &SUIStateConfigPanel::ClearWidgetConfig, WidgetName)
		]
	];

	TArray<EUMGStateConfigPropertyType> InsertedPropertyTypes;
	TArray<const FUMGStatePropertyChange*> SerializedPropertyChanges;
	if (const FUMGStateConfigState* State = GetActiveState())
	{
		const TArray<EUMGStateConfigPropertyType> SupportedPropertyTypes = GetSupportedPropertyTypes(Widget);
		for (EUMGStateConfigPropertyType PropertyType : SupportedPropertyTypes)
		{
			const bool bHasPropertyChange = State->PropertyChanges.ContainsByPredicate([WidgetName, PropertyType](const FUMGStatePropertyChange& Change)
			{
				return Change.TargetWidgetName == WidgetName && Change.PropertyType == PropertyType;
			});
			if (bHasPropertyChange)
			{
				InsertedPropertyTypes.Add(PropertyType);
			}
		}

		for (const FUMGStatePropertyChange& Change : State->PropertyChanges)
		{
			if (Change.TargetWidgetName == WidgetName && Change.PropertyType == EUMGStateConfigPropertyType::SerializedProperty)
			{
				SerializedPropertyChanges.Add(&Change);
			}
		}
	}

	int32 RowIndex = 0;
	for (EUMGStateConfigPropertyType InsertedPropertyType : InsertedPropertyTypes)
	{
		Rows->AddSlot().AutoHeight().Padding(0.0f, RowIndex++ == 0 ? 4.0f : 2.0f, 0.0f, 0.0f)
		[
			BuildPropertyRow(WidgetName, InsertedPropertyType, GetPropertyLabel(InsertedPropertyType), GetExpectedWidgetClass(WidgetName, InsertedPropertyType))
		];
	}
	for (const FUMGStatePropertyChange* SerializedChange : SerializedPropertyChanges)
	{
		if (SerializedChange)
		{
			Rows->AddSlot().AutoHeight().Padding(0.0f, RowIndex++ == 0 ? 4.0f : 2.0f, 0.0f, 0.0f)
			[
				BuildSerializedPropertyRow(*SerializedChange)
			];
		}
	}
	if (RowIndex == 0)
	{
		Rows->AddSlot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 0.0f)
		[
			SNew(STextBlock).Text(LOCTEXT("NoInsertedPropertyRows", "右键左侧控件选择属性后，会在这里插入配置行。"))
		];
	}


	return SNew(SBorder)
	.Padding(8.0f)

	.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
	[
		Rows
	];
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildPropertyRow(FName WidgetName, EUMGStateConfigPropertyType PropertyType, const FText& Label, TSubclassOf<UWidget> ExpectedClass)
{
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
	[
		SNew(SCheckBox)
		.IsChecked(this, &SUIStateConfigPanel::GetPropertyCheckState, WidgetName, PropertyType)
		.OnCheckStateChanged(this, &SUIStateConfigPanel::OnPropertyCheckChanged, WidgetName, PropertyType, ExpectedClass)
	]
	+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f).VAlign(VAlign_Center)
	[
		SNew(SBox)
		.WidthOverride(160.0f)
		[
			SNew(STextBlock).Text(Label)
		]
	]
	+ SHorizontalBox::Slot().FillWidth(1.0f)
	[
		BuildPropertyValueWidget(WidgetName, PropertyType, ExpectedClass)
	];
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildSerializedPropertyRow(const FUMGStatePropertyChange& Change)
{
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f).VAlign(VAlign_Center)
	[
		SNew(SButton)
		.Text(LOCTEXT("RemoveSerializedProperty", "移除"))
		.OnClicked(this, &SUIStateConfigPanel::RemoveSerializedPropertyChange, Change.TargetWidgetName, Change.Value.SerializedPropertyPath)
	]
	+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f).VAlign(VAlign_Center)
	[
		SNew(SBox)
		.WidthOverride(160.0f)
		[
			SNew(STextBlock).Text(FText::Format(LOCTEXT("SerializedPropertyLabel", "Details / {0}"), FText::FromString(Change.Value.SerializedPropertyPath)))
		]
	]
	+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
	[
		SNew(STextBlock)
		.AutoWrapText(true)
		.Text(FText::FromString(Change.Value.SerializedPropertyValue))
	]
	+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f, 0.0f, 0.0f).VAlign(VAlign_Center)
	[
		SNew(SButton)
		.Text(LOCTEXT("EditSerializedProperty", "编辑 Details"))
		.OnClicked_Lambda([this, WidgetName = Change.TargetWidgetName]()
		{
			OpenWidgetDetailsPropertyPicker(WidgetName);
			return FReply::Handled();
		})
	];
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildPropertyValueWidget(FName WidgetName, EUMGStateConfigPropertyType PropertyType, TSubclassOf<UWidget> ExpectedClass)

{
	(void)ExpectedClass;

	if (GetDetailsProxyDescriptor(PropertyType).IsSet())
	{
		return BuildAppearanceDetailsValueWidget(WidgetName, PropertyType);
	}


	switch (PropertyType)
	{
	case EUMGStateConfigPropertyType::TextColor:
	case EUMGStateConfigPropertyType::BrushTint:
	case EUMGStateConfigPropertyType::BrushImage:
		return SNew(STextBlock).Text(LOCTEXT("DeprecatedPropertyEditor", "已由 Appearance 覆盖，请改用外观（Appearance）。"));
	default:
		return SNew(STextBlock).Text(LOCTEXT("Unsupported", "Unsupported"));
	}
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildAppearanceDetailsValueWidget(FName WidgetName, EUMGStateConfigPropertyType PropertyType)
{
	const TOptional<FUMGStateConfigDetailsProxyDescriptor> ProxyDescriptor = GetDetailsProxyDescriptor(PropertyType);
	if (!ProxyDescriptor.IsSet())
	{
		return SNew(STextBlock).Text(LOCTEXT("UnsupportedAppearanceDetails", "Unsupported Appearance"));
	}

	const FUMGStatePropertyChange* Change = FindPropertyChange(WidgetName, PropertyType);
	const FUMGStateConfigPropertyValue SourceValue = Change ? Change->Value : MakeDefaultValueForWidget(WidgetName, PropertyType);
	UUMGStateConfigDetailsProxyBase* ProxyObject = NewObject<UUMGStateConfigDetailsProxyBase>(GetTransientPackage(), ProxyDescriptor->ProxyClass);
	if (!ProxyObject)
	{
		return SNew(STextBlock).Text(LOCTEXT("UnsupportedAppearanceDetails", "Unsupported Appearance"));
	}

	ProxyObject->SetFlags(RF_Transient);
	ProxyObject->FromValue(SourceValue);
	AppearanceDetailProxyObjects.Add(TStrongObjectPtr<UObject>(ProxyObject));


	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = false;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowOptions = false;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(ProxyObject);
	DetailsView->OnFinishedChangingProperties().AddSP(this, &SUIStateConfigPanel::OnAppearanceDetailsFinishedChanging, WidgetName, PropertyType, TWeakObjectPtr<UObject>(ProxyObject));

	return SNew(SBox)
	.MinDesiredWidth(360.0f)
	.MaxDesiredHeight(420.0f)
	[
		DetailsView
	];
}



void SUIStateConfigPanel::RefreshAll()
{
	NormalizeRedundantPropertyChanges();
	RebuildWidgetRows();
	if (WidgetListView.IsValid())
	{
		WidgetListView->RequestListRefresh();
	}
	if (ParentTabsBox.IsValid())
	{
		ParentTabsBox->ClearChildren();
		ParentTabsBox->AddSlot().AutoHeight()[BuildParentStateTabs()];
	}
	if (ChildTabsBox.IsValid())
	{
		ChildTabsBox->ClearChildren();
		ChildTabsBox->AddSlot().AutoHeight()[BuildChildStateTabs()];
	}
	if (ConfiguredWidgetsBox.IsValid())
	{
		ConfiguredWidgetsBox->ClearChildren();
		AppearanceDetailProxyObjects.Reset();
		ConfiguredWidgetsBox->AddSlot().AutoHeight()[BuildConfiguredWidgetCards()];
	}
	RefreshSummary();
}

void SUIStateConfigPanel::NormalizeRedundantPropertyChanges()
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigState* State = GetActiveState();
	if (!Extension || !State)
	{
		return;
	}

	TArray<FName> WidgetsWithImageAppearance;
	TArray<FName> WidgetsWithTextAppearance;
	for (const FUMGStatePropertyChange& Change : State->PropertyChanges)
	{
		if (Change.PropertyType == EUMGStateConfigPropertyType::ImageAppearance)
		{
			WidgetsWithImageAppearance.AddUnique(Change.TargetWidgetName);
		}
		else if (Change.PropertyType == EUMGStateConfigPropertyType::TextAppearance)
		{
			WidgetsWithTextAppearance.AddUnique(Change.TargetWidgetName);
		}
	}

	const int32 RemovedCount = State->PropertyChanges.RemoveAll([&WidgetsWithImageAppearance, &WidgetsWithTextAppearance](const FUMGStatePropertyChange& Change)
	{
		if (WidgetsWithImageAppearance.Contains(Change.TargetWidgetName))
		{
			return Change.PropertyType == EUMGStateConfigPropertyType::BrushImage
				|| Change.PropertyType == EUMGStateConfigPropertyType::BrushTint;
		}
		if (WidgetsWithTextAppearance.Contains(Change.TargetWidgetName))
		{
			return Change.PropertyType == EUMGStateConfigPropertyType::TextColor;
		}
		return false;
	});

	if (RemovedCount > 0)
	{
		MarkConfigDirty(Extension);
	}
}

void SUIStateConfigPanel::RefreshSummary()
{
	const UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	const UUMGStateConfigBlueprintExtension* Extension = GetExtension();

	int32 ChangeCount = 0;
	if (Extension)
	{
		for (const FUMGStateConfigGroup& Group : Extension->ConfigData.StateGroups)
		{
			for (const FUMGStateConfigState& State : Group.States)
			{
				ChangeCount += State.PropertyChanges.Num();
			}
		}
	}

	TArray<FText> Errors;
	TArray<FText> Warnings;
	TArray<FText> Hints;
	FUMGStateConfigValidator::Validate(WidgetBlueprint, Errors, Warnings, Hints);

	CachedSummaryText = FText::Format(
		LOCTEXT("Summary", "校验提示：{0} 个错误 · {1} 个警告 · {2} 个提示 · 当前共 {3} 条属性修改"),
		FText::AsNumber(Errors.Num()),
		FText::AsNumber(Warnings.Num()),
		FText::AsNumber(Hints.Num()),
		FText::AsNumber(ChangeCount));

	if (SummaryTextBlock.IsValid())
	{
		SummaryTextBlock->SetText(CachedSummaryText);
	}
}

void SUIStateConfigPanel::RebuildWidgetRows()
{
	WidgetRows.Reset();
	const UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
	{
		return;
	}

	TArray<UWidget*> Widgets;
	WidgetBlueprint->WidgetTree->GetAllWidgets(Widgets);
	for (UWidget* Widget : Widgets)
	{
		if (!Widget || (Widget->IsGeneratedName() && !Widget->bIsVariable))
		{
			continue;
		}
		TSharedPtr<FUMGStateConfigWidgetRow> Row = MakeShared<FUMGStateConfigWidgetRow>();
		Row->Widget = Widget;
		WidgetRows.Add(Row);
	}
}

void SUIStateConfigPanel::SelectParentState(FName GroupName)
{
	SelectedGroupName = GroupName;
	FUMGStateConfigGroup* Group = GetActiveGroup();
	SelectedStateName = Group && Group->States.Num() > 0 ? (Group->DefaultStateName.IsNone() ? Group->States[0].StateName : Group->DefaultStateName) : NAME_None;
	if (UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension())
	{
		Extension->ConfigData.PreviewStateGroupName = SelectedGroupName;
		Extension->ConfigData.PreviewStateName = SelectedStateName;
		MarkConfigDirty(Extension);
	}
	ApplyPreviewState();
	RefreshAll();
}

void SUIStateConfigPanel::SelectChildState(FName StateName)
{
	SelectedStateName = StateName;
	if (UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension())
	{
		Extension->ConfigData.PreviewStateGroupName = SelectedGroupName;
		Extension->ConfigData.PreviewStateName = SelectedStateName;
		MarkConfigDirty(Extension);
	}
	ApplyPreviewState();
	RefreshAll();
}

TSharedRef<ITableRow> SUIStateConfigPanel::GenerateWidgetRow(TSharedPtr<FUMGStateConfigWidgetRow> RowItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	FString WidgetName = TEXT("Invalid");
	FString WidgetType = TEXT("-");
	if (RowItem.IsValid())
	{
		if (UWidget* Widget = RowItem->Widget.Get())
		{
			WidgetName = Widget->GetFName().ToString();
			WidgetType = Widget->GetClass()->GetName();
		}
	}
	return SNew(STableRow<TSharedPtr<FUMGStateConfigWidgetRow>>, OwnerTable)
	[
		SNew(SBorder)
		.Padding(0.0f)
		.BorderImage(FAppStyle::GetBrush("NoBorder"))
		.OnMouseButtonDown(this, &SUIStateConfigPanel::HandleAvailableWidgetRowMouseButtonDown, RowItem)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(180.0f)
				[
					SNew(STextBlock).Text(FText::FromString(WidgetName))
				]
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(120.0f)
				[
					SNew(STextBlock).Text(FText::FromString(WidgetType))
				]
			]
		]
	];
}

FReply SUIStateConfigPanel::HandleAvailableWidgetRowMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& MouseEvent, TSharedPtr<FUMGStateConfigWidgetRow> RowItem)
{
	if (MouseEvent.GetEffectingButton() != EKeys::RightMouseButton || !RowItem.IsValid())
	{
		return FReply::Unhandled();
	}

	UWidget* Widget = RowItem->Widget.Get();
	if (!Widget)
	{
		return FReply::Unhandled();
	}

	SelectedWidgetName = Widget->GetFName();
	FSlateApplication::Get().PushMenu(
		AsShared(),
		FWidgetPath(),
		BuildAvailableWidgetPropertyMenu(SelectedWidgetName),
		MouseEvent.GetScreenSpacePosition(),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
	return FReply::Handled();
}

void SUIStateConfigPanel::OnWidgetSelected(TSharedPtr<FUMGStateConfigWidgetRow> RowItem, ESelectInfo::Type SelectInfo)
{
	SelectedWidgetName = NAME_None;
	if (RowItem.IsValid())
	{
		if (UWidget* Widget = RowItem->Widget.Get())
		{
			SelectedWidgetName = Widget->GetFName();
		}
	}
	RefreshSummary();
}

void SUIStateConfigPanel::OnWidgetDoubleClicked(TSharedPtr<FUMGStateConfigWidgetRow> RowItem)
{
	if (!RowItem.IsValid())
	{
		return;
	}
	if (UWidget* Widget = RowItem->Widget.Get())
	{
		SelectedWidgetName = Widget->GetFName();
		AddWidgetToActiveGroupStates(SelectedWidgetName);
	}
}

void SUIStateConfigPanel::AddWidgetToActiveGroupStates(FName WidgetName)
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("AddWidgetToActiveGroupStates", "添加控件到状态"), nullptr);
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigGroup* Group = GetActiveGroup();
	if (!Extension || !Group || WidgetName.IsNone())
	{
		return;
	}

	for (FUMGStateConfigState& State : Group->States)
	{
		State.ConfiguredWidgetNames.AddUnique(WidgetName);
	}
	MarkConfigDirty(Extension);
	RefreshAll();
}

void SUIStateConfigPanel::AddWidgetPropertyToActiveState(FName WidgetName, EUMGStateConfigPropertyType PropertyType)
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("AddWidgetPropertyToActiveState", "添加控件属性"), nullptr);
	UWidget* Widget = FindWidgetByName(WidgetName);
	if (!Widget || !GetSupportedPropertyTypes(Widget).Contains(PropertyType))
	{
		return;
	}

	AddOrUpdatePropertyChange(
		WidgetName,
		PropertyType,
		MakeDefaultValueForWidget(WidgetName, PropertyType),
		GetExpectedWidgetClass(WidgetName, PropertyType));
	RefreshAll();
}

void SUIStateConfigPanel::OpenWidgetDetailsPropertyPicker(FName WidgetName)
{
	UWidget* Widget = FindWidgetByName(WidgetName);
	if (!Widget)
	{
		return;
	}

	UWidget* ProxyWidget = DuplicateObject<UWidget>(Widget, GetTransientPackage());
	if (!ProxyWidget)
	{
		return;
	}
	ProxyWidget->SetFlags(RF_Transient);

	if (const FUMGStateConfigState* State = GetActiveState())
	{
		for (const FUMGStatePropertyChange& Change : State->PropertyChanges)
		{
			if (Change.TargetWidgetName == WidgetName)
			{
				FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(ProxyWidget, Change.PropertyType, Change.Value);
			}
		}
	}

	AppearanceDetailProxyObjects.Add(TStrongObjectPtr<UObject>(ProxyWidget));

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowOptions = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(ProxyWidget);
	DetailsView->OnFinishedChangingProperties().AddSP(this, &SUIStateConfigPanel::OnWidgetDetailsPropertyFinishedChanging, WidgetName, TWeakObjectPtr<UObject>(ProxyWidget));

	FSlateApplication::Get().PushMenu(
		AsShared(),
		FWidgetPath(),
		SNew(SBorder)
		.Padding(8.0f)
		[
			SNew(SBox)
			.WidthOverride(520.0f)
			.MaxDesiredHeight(640.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.Text(LOCTEXT("DetailsPropertyPickerHint", "修改下方 Details 中的属性后，会把变更属性作为 Details 序列化属性加入当前状态。嵌套字段会按顶层属性保存。"))
				]
				+ SVerticalBox::Slot().FillHeight(1.0f)
				[
					DetailsView
				]
			]
		],
		FSlateApplication::Get().GetCursorPos(),
		FPopupTransitionEffect(FPopupTransitionEffect::ContextMenu));
}

void SUIStateConfigPanel::OnWidgetDetailsPropertyFinishedChanging(const FPropertyChangedEvent& PropertyChangedEvent, FName WidgetName, TWeakObjectPtr<UObject> ProxyObject)
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("OnWidgetDetailsPropertyFinishedChanging", "编辑属性"), nullptr);
	UWidget* ProxyWidget = Cast<UWidget>(ProxyObject.Get());
	if (!ProxyWidget)
	{
		return;
	}

	FProperty* ChangedProperty = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty : PropertyChangedEvent.Property;
	if (!ChangedProperty)
	{
		return;
	}

	FUMGStateConfigPropertyValue Value;
	Value.SerializedPropertyPath = ChangedProperty->GetName();
	if (!FUMGStateConfigPropertyRuntimeLibrary::CaptureCurrentValue(ProxyWidget, EUMGStateConfigPropertyType::SerializedProperty, Value))
	{
		return;
	}

	AddOrUpdateSerializedPropertyChange(WidgetName, Value, ProxyWidget->GetClass());
	RefreshAll();
}

FUMGStatePropertyChange* SUIStateConfigPanel::FindPropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType)


{
	FUMGStateConfigState* State = GetActiveState();
	return State ? State->PropertyChanges.FindByPredicate([WidgetName, PropertyType](const FUMGStatePropertyChange& Change)
	{
		return Change.TargetWidgetName == WidgetName && Change.PropertyType == PropertyType;
	}) : nullptr;
}

const FUMGStatePropertyChange* SUIStateConfigPanel::FindPropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const
{
	const UUMGStateConfigBlueprintExtension* Extension = GetExtension();
	if (!Extension)
	{
		return nullptr;
	}
	const FUMGStateConfigGroup* Group = Extension->ConfigData.StateGroups.FindByPredicate([this](const FUMGStateConfigGroup& Candidate)
	{
		return Candidate.GroupName == SelectedGroupName;
	});
	const FUMGStateConfigState* State = Group ? Group->States.FindByPredicate([this](const FUMGStateConfigState& Candidate)
	{
		return Candidate.StateName == SelectedStateName;
	}) : nullptr;
	return State ? State->PropertyChanges.FindByPredicate([WidgetName, PropertyType](const FUMGStatePropertyChange& Change)
	{
		return Change.TargetWidgetName == WidgetName && Change.PropertyType == PropertyType;
	}) : nullptr;
}

bool SUIStateConfigPanel::HasPropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const
{
	return FindPropertyChange(WidgetName, PropertyType) != nullptr;
}

ECheckBoxState SUIStateConfigPanel::GetPropertyCheckState(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const
{
	return HasPropertyChange(WidgetName, PropertyType) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SUIStateConfigPanel::OnPropertyCheckChanged(ECheckBoxState NewState, FName WidgetName, EUMGStateConfigPropertyType PropertyType, TSubclassOf<UWidget> ExpectedClass)
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("OnPropertyCheckChanged", "切换属性"), nullptr);
	if (NewState == ECheckBoxState::Checked)
	{
		AddOrUpdatePropertyChange(WidgetName, PropertyType, MakeDefaultValueForWidget(WidgetName, PropertyType), ExpectedClass);
	}
	else
	{
		RemovePropertyChange(WidgetName, PropertyType);
	}
	RefreshAll();
}

void SUIStateConfigPanel::AddOrUpdatePropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& Value, TSubclassOf<UWidget> ExpectedClass)
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigState* State = GetActiveState();
	if (!Extension || !State || WidgetName.IsNone())
	{
		return;
	}

	State->ConfiguredWidgetNames.AddUnique(WidgetName);
	if (PropertyType == EUMGStateConfigPropertyType::ImageAppearance)
	{
		State->PropertyChanges.RemoveAll([WidgetName](const FUMGStatePropertyChange& Change)
		{
			return Change.TargetWidgetName == WidgetName
				&& (Change.PropertyType == EUMGStateConfigPropertyType::BrushImage || Change.PropertyType == EUMGStateConfigPropertyType::BrushTint);
		});
	}
	else if (PropertyType == EUMGStateConfigPropertyType::TextAppearance)
	{
		State->PropertyChanges.RemoveAll([WidgetName](const FUMGStatePropertyChange& Change)
		{
			return Change.TargetWidgetName == WidgetName && Change.PropertyType == EUMGStateConfigPropertyType::TextColor;
		});
	}

	if (FUMGStatePropertyChange* Existing = FindPropertyChange(WidgetName, PropertyType))
	{
		Existing->Value = Value;
		Existing->ExpectedWidgetClass = ExpectedClass;
	}
	else
	{
		FUMGStatePropertyChange Change;
		Change.TargetWidgetName = WidgetName;
		Change.ExpectedWidgetClass = ExpectedClass;
		Change.EditorPath = WidgetName.ToString();
		Change.PropertyType = PropertyType;
		Change.Value = Value;
		State->PropertyChanges.Add(Change);
	}

	MarkConfigDirty(Extension);
	ApplyPreviewState();
}

void SUIStateConfigPanel::AddOrUpdateSerializedPropertyChange(FName WidgetName, const FUMGStateConfigPropertyValue& Value, TSubclassOf<UWidget> ExpectedClass)
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigState* State = GetActiveState();
	if (!Extension || !State || WidgetName.IsNone() || Value.SerializedPropertyPath.IsEmpty())
	{
		return;
	}

	State->ConfiguredWidgetNames.AddUnique(WidgetName);
	FUMGStatePropertyChange* Existing = State->PropertyChanges.FindByPredicate([WidgetName, &Value](const FUMGStatePropertyChange& Change)
	{
		return Change.TargetWidgetName == WidgetName
			&& Change.PropertyType == EUMGStateConfigPropertyType::SerializedProperty
			&& Change.Value.SerializedPropertyPath == Value.SerializedPropertyPath;
	});

	if (Existing)
	{
		Existing->Value = Value;
		Existing->ExpectedWidgetClass = ExpectedClass;
	}
	else
	{
		FUMGStatePropertyChange Change;
		Change.TargetWidgetName = WidgetName;
		Change.ExpectedWidgetClass = ExpectedClass;
		Change.EditorPath = FString::Printf(TEXT("%s.%s"), *WidgetName.ToString(), *Value.SerializedPropertyPath);
		Change.PropertyType = EUMGStateConfigPropertyType::SerializedProperty;
		Change.Value = Value;
		State->PropertyChanges.Add(Change);
	}

	MarkConfigDirty(Extension);
	ApplyPreviewState();
}

void SUIStateConfigPanel::RemovePropertyChange(FName WidgetName, EUMGStateConfigPropertyType PropertyType)
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigState* State = GetActiveState();
	if (!Extension || !State)
	{
		return;
	}
	State->PropertyChanges.RemoveAll([WidgetName, PropertyType](const FUMGStatePropertyChange& Change)
	{
		return Change.TargetWidgetName == WidgetName && Change.PropertyType == PropertyType;
	});
	MarkConfigDirty(Extension);
	ApplyPreviewState();
}

FReply SUIStateConfigPanel::RemoveSerializedPropertyChange(FName WidgetName, FString SerializedPropertyPath)
{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("RemoveSerializedPropertyChange", "移除序列化属性"), nullptr);
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigState* State = GetActiveState();
	if (!Extension || !State)
	{
		return FReply::Handled();
	}

	State->PropertyChanges.RemoveAll([WidgetName, &SerializedPropertyPath](const FUMGStatePropertyChange& Change)
	{
		return Change.TargetWidgetName == WidgetName
			&& Change.PropertyType == EUMGStateConfigPropertyType::SerializedProperty
			&& Change.Value.SerializedPropertyPath == SerializedPropertyPath;
	});
	MarkConfigDirty(Extension);
	ApplyPreviewState();
	RefreshAll();
	return FReply::Handled();
}

void SUIStateConfigPanel::OnAppearanceDetailsFinishedChanging(const FPropertyChangedEvent& PropertyChangedEvent, FName WidgetName, EUMGStateConfigPropertyType PropertyType, TWeakObjectPtr<UObject> ProxyObject)

{
	TUniquePtr<FScopedTransaction> Transaction = BeginConfigEdit(LOCTEXT("OnAppearanceDetailsFinishedChanging", "编辑外观"), nullptr);
	(void)PropertyChangedEvent;

	if (!ProxyObject.IsValid())
	{
		return;
	}


	const TOptional<FUMGStateConfigDetailsProxyDescriptor> ProxyDescriptor = GetDetailsProxyDescriptor(PropertyType);
	const UUMGStateConfigDetailsProxyBase* DetailsProxy = Cast<UUMGStateConfigDetailsProxyBase>(ProxyObject.Get());
	if (!ProxyDescriptor.IsSet() || !DetailsProxy)
	{
		return;
	}

	FUMGStateConfigPropertyValue Value = FindPropertyChange(WidgetName, PropertyType)
		? FindPropertyChange(WidgetName, PropertyType)->Value
		: MakeDefaultValueForWidget(WidgetName, PropertyType);
	DetailsProxy->ToValue(Value);

	const TSubclassOf<UWidget> ExpectedClass = ProxyDescriptor->bUseWidgetSpecificExpectedClass
		? GetExpectedWidgetClass(WidgetName, PropertyType)
		: ProxyDescriptor->DefaultExpectedClass;
	AddOrUpdatePropertyChange(WidgetName, PropertyType, Value, ExpectedClass);
	RefreshSummary();
}


FUMGStateConfigPropertyValue SUIStateConfigPanel::MakeDefaultValueForWidget(FName WidgetName, EUMGStateConfigPropertyType PropertyType) const
{
	FUMGStateConfigPropertyValue Value;
	UWidget* Widget = FindWidgetByName(WidgetName);
	switch (PropertyType)
	{
	case EUMGStateConfigPropertyType::Visibility:
		Value.VisibilityValue = Widget ? Widget->GetVisibility() : ESlateVisibility::Visible;
		break;
	case EUMGStateConfigPropertyType::RenderOpacity:
		Value.FloatValue = Widget ? Widget->GetRenderOpacity() : 1.0f;
		break;
	case EUMGStateConfigPropertyType::Text:
		if (const UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			Value.TextValue = TextBlock->GetText();
		}
		else if (const URichTextBlock* RichTextBlock = Cast<URichTextBlock>(Widget))
		{
			Value.TextValue = RichTextBlock->GetText();
		}
		else
		{
			Value.TextValue = LOCTEXT("DefaultText", "示例文本");
		}
		break;
	case EUMGStateConfigPropertyType::TextAppearance:
		if (const UTextBlock* TextBlock = Cast<UTextBlock>(Widget))
		{
			Value.ColorValue = TextBlock->GetColorAndOpacity().GetSpecifiedColor();
			Value.FontValue = TextBlock->GetFont();
			Value.VectorValue = TextBlock->GetShadowOffset();
			Value.SecondaryColorValue = TextBlock->GetShadowColorAndOpacity();
		}
		break;
	case EUMGStateConfigPropertyType::TextColor:
		Value.ColorValue = Cast<UTextBlock>(Widget) ? Cast<UTextBlock>(Widget)->GetColorAndOpacity().GetSpecifiedColor() : FLinearColor::Yellow;
		break;
	case EUMGStateConfigPropertyType::BrushImage:
		if (const UImage* Image = Cast<UImage>(Widget))
		{
			Value.ObjectValue = Image->GetBrush().GetResourceObject();
		}
		break;
	case EUMGStateConfigPropertyType::BrushTint:
		Value.ColorValue = Cast<UImage>(Widget) ? Cast<UImage>(Widget)->GetColorAndOpacity() : FLinearColor::White;
		break;
	case EUMGStateConfigPropertyType::ImageAppearance:
		if (const UImage* Image = Cast<UImage>(Widget))
		{
			Value.BrushValue = Image->GetBrush();
			Value.ColorValue = Image->GetColorAndOpacity();
			Value.ObjectValue = Image->GetBrush().GetResourceObject();
		}
		break;
	default:
		break;
	}
	return Value;
}

TArray<FName> SUIStateConfigPanel::GetConfiguredWidgetNames() const
{
	TArray<FName> Names;
	const FUMGStateConfigState* State = nullptr;
	const UUMGStateConfigBlueprintExtension* Extension = GetExtension();
	const FUMGStateConfigGroup* Group = Extension ? Extension->ConfigData.StateGroups.FindByPredicate([this](const FUMGStateConfigGroup& Candidate)
	{
		return Candidate.GroupName == SelectedGroupName;
	}) : nullptr;
	State = Group ? Group->States.FindByPredicate([this](const FUMGStateConfigState& Candidate)
	{
		return Candidate.StateName == SelectedStateName;
	}) : nullptr;
	if (!State)
	{
		return Names;
	}
	for (FName WidgetName : State->ConfiguredWidgetNames)
	{
		Names.AddUnique(WidgetName);
	}
	for (const FUMGStatePropertyChange& Change : State->PropertyChanges)
	{
		Names.AddUnique(Change.TargetWidgetName);
	}
	return Names;
}



bool SUIStateConfigPanel::IsWidgetText(const UWidget* Widget) const
{
	return Widget && (Widget->IsA<UTextBlock>() || Widget->IsA<URichTextBlock>());
}

bool SUIStateConfigPanel::IsWidgetImage(const UWidget* Widget) const
{
	return Widget && Widget->IsA<UImage>();
}

FString SUIStateConfigPanel::FormatPropertyChange(const FUMGStatePropertyChange& Change) const
{
	const UEnum* PropertyEnum = StaticEnum<EUMGStateConfigPropertyType>();
	const FString PropertyName = PropertyEnum ? PropertyEnum->GetNameStringByValue(static_cast<int64>(Change.PropertyType)) : TEXT("Unknown");
	return FString::Printf(TEXT("%s.%s"), *Change.TargetWidgetName.ToString(), *PropertyName);
}


TUniquePtr<FScopedTransaction> SUIStateConfigPanel::BeginConfigEdit(const FText& Description, UUMGStateConfigBlueprintExtension* Extension)
{
	TUniquePtr<FScopedTransaction> Transaction = MakeUnique<FScopedTransaction>(Description);
	if (UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint())
	{
		WidgetBlueprint->Modify();
	}
	if (Extension)
	{
		Extension->Modify();
	}
	return Transaction;
}
void SUIStateConfigPanel::MarkConfigDirty(UUMGStateConfigBlueprintExtension* Extension)
{
	UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	if (WidgetBlueprint)
	{
		WidgetBlueprint->Modify();
		WidgetBlueprint->MarkPackageDirty();
	}
	if (Extension)
	{
		Extension->Modify();
	}
}

#undef LOCTEXT_NAMESPACE
