#include "SUIStateConfigPanel.h"

#include "Blueprint/UserWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"

#include "Framework/Application/SlateApplication.h"

#include "IDetailsView.h"
#include "InputCoreTypes.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Modules/ModuleManager.h"

#include "PropertyEditorModule.h"

#include "Styling/AppStyle.h"
#include "Styling/SlateIconFinder.h"
#include "UMGStateConfigBlueprintExtension.h"
#include "UMGStateConfigPropertyRuntimeLibrary.h"

#include "UMGStateConfigValidator.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprint.h"


#include "WidgetBlueprintEditor.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"

#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Images/SImage.h"
#include "Misc/MessageDialog.h"

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

bool IsSnapshotPropertySupported(const FProperty* Property)
{
	if (!Property)
	{
		return false;
	}

	const EPropertyFlags UnsupportedFlags = CPF_Transient | CPF_EditConst | CPF_Deprecated | CPF_DisableEditOnInstance;
	if (Property->HasAnyPropertyFlags(UnsupportedFlags))
	{
		return false;
	}

	return !CastField<FDelegateProperty>(Property)
		&& !CastField<FMulticastDelegateProperty>(Property)
		&& !CastField<FMapProperty>(Property)
		&& !CastField<FSetProperty>(Property)
		&& !CastField<FArrayProperty>(Property);
}

bool IsCommonSerializedPropertyPath(const FString& PropertyPath)
{
	return PropertyPath == TEXT("Visibility")
		|| PropertyPath == TEXT("RenderOpacity")
		|| PropertyPath == TEXT("Text")
		|| PropertyPath == TEXT("HintText")
		|| PropertyPath == TEXT("Brush.ResourceObject")
		|| PropertyPath.StartsWith(TEXT("ColorAndOpacity"))
		|| PropertyPath.StartsWith(TEXT("Brush.TintColor"))
		|| PropertyPath.StartsWith(TEXT("Brush.ImageSize"))
		|| PropertyPath == TEXT("Font.Size")
		|| PropertyPath.StartsWith(TEXT("RenderTransform"))
		|| PropertyPath.StartsWith(TEXT("Style"))
		|| PropertyPath == TEXT("Percent")
		|| PropertyPath == TEXT("Value")
		|| PropertyPath.StartsWith(TEXT("FillColorAndOpacity"))
		|| PropertyPath.StartsWith(TEXT("SliderBarColor"))
		|| PropertyPath.StartsWith(TEXT("SliderHandleColor"))
		|| PropertyPath.StartsWith(TEXT("BackgroundColor"))
		|| PropertyPath.StartsWith(TEXT("ContentColorAndOpacity"))
		|| PropertyPath.StartsWith(TEXT("Padding"))
		|| PropertyPath.StartsWith(TEXT("ForegroundColor"));
}

FString BuildDetailsPropertyPath(const FPropertyAndParent& PropertyAndParent)
{
	FString PropertyPath;
	for (int32 Index = PropertyAndParent.ParentProperties.Num() - 1; Index >= 0; --Index)
	{
		if (const FProperty* ParentProperty = PropertyAndParent.ParentProperties[Index])
		{
			if (!PropertyPath.IsEmpty())
			{
				PropertyPath += TEXT(".");
			}
			PropertyPath += ParentProperty->GetName();
		}
	}

	if (!PropertyPath.IsEmpty())
	{
		PropertyPath += TEXT(".");
	}
	PropertyPath += PropertyAndParent.Property.GetName();
	return PropertyPath;
}

bool IsCommonSerializedPropertyPathOrParent(const FString& PropertyPath)
{
	static const TArray<FString> CommonLeafPaths = {
		TEXT("Visibility"),
		TEXT("RenderOpacity"),
		TEXT("Text"),
		TEXT("HintText"),
		TEXT("Brush.ResourceObject"),
		TEXT("ColorAndOpacity"),
		TEXT("Brush.TintColor"),
		TEXT("Brush.ImageSize"),
		TEXT("Font.Size"),
		TEXT("RenderTransform"),
		TEXT("Style"),
		TEXT("Percent"),
		TEXT("Value"),
		TEXT("FillColorAndOpacity"),
		TEXT("SliderBarColor"),
		TEXT("SliderHandleColor"),
		TEXT("BackgroundColor"),
		TEXT("ContentColorAndOpacity"),
		TEXT("Padding"),
		TEXT("ForegroundColor")
	};

	if (IsCommonSerializedPropertyPath(PropertyPath))
	{
		return true;
	}

	for (const FString& CommonLeafPath : CommonLeafPaths)
	{
		if (CommonLeafPath.StartsWith(PropertyPath + TEXT(".")))
		{
			return true;
		}
	}
	return false;
}

bool IsNoisyPropertySuppressedByPrimary(const FString& CandidatePath, const FString& PrimaryPath)

{
	FString PrimaryRoot;

	FString PrimaryLeaf;
	if (!PrimaryPath.Split(TEXT("."), &PrimaryRoot, &PrimaryLeaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
	{
		return false;
	}

	FString CandidateRoot;
	FString CandidateLeaf;
	if (!CandidatePath.Split(TEXT("."), &CandidateRoot, &CandidateLeaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd) || CandidateRoot != PrimaryRoot)
	{
		return false;
	}

	if (PrimaryLeaf == TEXT("ResourceObject"))
	{
		return CandidateLeaf == TEXT("DrawAs") || CandidateLeaf == TEXT("ImageType") || CandidateLeaf == TEXT("ResourceName");
	}
	if (PrimaryLeaf == TEXT("FontObject"))
	{
		return CandidateLeaf == TEXT("TypefaceFontName");
	}
	if (PrimaryLeaf == TEXT("SpecifiedColor"))
	{
		return CandidateLeaf == TEXT("ColorUseRule");
	}
	return false;
}

TSharedRef<SWidget> MakeIconTextButtonContent(const FName& IconName, const FText& Label)
{
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 4.0f, 0.0f)
	[
		SNew(SImage).Image(FAppStyle::GetBrush(IconName))
	]
	+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
	[
		SNew(STextBlock).Text(Label)
	];
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
						[
							MakeIconTextButtonContent("Icons.Plus", LOCTEXT("AddParentStateBtn", "父状态"))
						]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("DuplicateParentState", "复制父状态"))
					.ToolTipText(LOCTEXT("DuplicateParentStateTip", "复制当前父状态组及其全部子状态和控件属性配置。"))
					.OnClicked(this, &SUIStateConfigPanel::DuplicateParentState)
						[
							MakeIconTextButtonContent("Icons.Duplicate", LOCTEXT("DuplicateParentStateBtn", "复制父状态"))
						]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("DeleteParentState", "- 父状态"))
					.ToolTipText(LOCTEXT("DeleteParentStateTip", "删除当前选中的父状态配置。"))
					.OnClicked(this, &SUIStateConfigPanel::DeleteParentState)
						[
							MakeIconTextButtonContent("Icons.Delete", LOCTEXT("DeleteParentStateBtn", "删除父状态"))
						]
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
						[
							MakeIconTextButtonContent("Icons.Plus", LOCTEXT("AddChildStateBtn", "子状态"))
						]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("DuplicateChildState", "复制子状态"))
					.ToolTipText(LOCTEXT("DuplicateChildStateTip", "复制当前子状态及其全部控件属性配置。"))
					.OnClicked(this, &SUIStateConfigPanel::DuplicateChildState)
						[
							MakeIconTextButtonContent("Icons.Duplicate", LOCTEXT("DuplicateChildStateBtn", "复制子状态"))
						]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("RemoveInvalidChanges", "清理无效"))
					.ToolTipText(LOCTEXT("RemoveInvalidChangesTip", "移除当前子状态中目标控件已不存在的配置。"))
					.OnClicked(this, &SUIStateConfigPanel::RemoveInvalidChanges)
						[
							MakeIconTextButtonContent("Icons.Refresh", LOCTEXT("RemoveInvalidChangesBtn", "清理无效"))
						]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("DeleteChildState", "- 子状态"))
					.ToolTipText(LOCTEXT("DeleteChildStateTip", "删除当前选中的子状态配置。"))
					.OnClicked(this, &SUIStateConfigPanel::DeleteChildState)
						[
							MakeIconTextButtonContent("Icons.Delete", LOCTEXT("DeleteChildStateBtn", "删除子状态"))
						]
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
				SNew(SSplitter)
				.Orientation(Orient_Horizontal)
				.PhysicalSplitterHandleSize(4.0f)
				+ SSplitter::Slot()
				.Value(0.28f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock).Text(LOCTEXT("AvailableWidgets", "可加入控件（右键从 Details 添加属性）"))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 2.0f, 0.0f, 2.0f)
					[
						SNew(SSearchBox)
						.HintText(LOCTEXT("FilterWidgetsHint", "筛选控件…"))
						.OnTextChanged(this, &SUIStateConfigPanel::OnWidgetFilterChanged)
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
				+ SSplitter::Slot()
				.Value(0.72f)
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SAssignNew(ConfiguredWidgetsListView, SListView<TSharedPtr<FName>>)
						.ListItemsSource(&ConfiguredWidgetItems)
						.OnGenerateRow(this, &SUIStateConfigPanel::GenerateConfiguredWidgetRow)
						.SelectionMode(ESelectionMode::None)
					]
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill).VAlign(VAlign_Top)
					.Padding(0.0f, 4.0f)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Text(LOCTEXT("NoConfiguredWidget", "当前子状态还没有控件配置。请双击左侧控件加入当前父状态的所有子状态。"))
						.Visibility(this, &SUIStateConfigPanel::GetConfiguredEmptyVisibility)
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
				.ColorAndOpacity_Lambda([this]() -> FSlateColor
				{
					TArray<FText> Errors;
					TArray<FText> Warnings;
					TArray<FText> Hints;
					FUMGStateConfigValidator::Validate(GetWidgetBlueprint(), Errors, Warnings, Hints);
					if (Errors.Num() > 0)
					{
						return FSlateColor(FLinearColor(0.90f, 0.25f, 0.25f));
					}
					if (Warnings.Num() > 0)
					{
						return FSlateColor(FLinearColor(0.95f, 0.75f, 0.20f));
					}
					return FSlateColor::UseForeground();
				})
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

	const FText BaseSummary = FText::Format(
		LOCTEXT("Summary", "校验提示：{0} 个错误 · {1} 个警告 · {2} 个提示 · 当前共 {3} 条属性修改 · 修改配置后请编译 Widget Blueprint 使运行时生效"),
		FText::AsNumber(Errors.Num()),
		FText::AsNumber(Warnings.Num()),
		FText::AsNumber(Hints.Num()),
		FText::AsNumber(ChangeCount));
	return LastDetailsCaptureMessage.IsEmpty()
		? BaseSummary
		: FText::Format(LOCTEXT("SummaryWithDetailsCapture", "{0}\n{1}"), BaseSummary, LastDetailsCaptureMessage);


}


FText SUIStateConfigPanel::GetBreadcrumbText() const
{
	return FText::Format(LOCTEXT("Breadcrumb", "当前：{0} → {1}"), FText::FromName(SelectedGroupName), FText::FromName(SelectedStateName));
}

FReply SUIStateConfigPanel::EnsureDefaultConfig()
{
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

	const FName PreviewGroupName = SelectedGroupName.IsNone() ? Extension->ConfigData.PreviewStateGroupName : SelectedGroupName;
	const FName PreviewStateName = SelectedStateName.IsNone() ? Extension->ConfigData.PreviewStateName : SelectedStateName;
	const FUMGStateConfigGroup* Group = Extension->ConfigData.StateGroups.FindByPredicate([PreviewGroupName](const FUMGStateConfigGroup& Candidate)
	{
		return Candidate.GroupName == PreviewGroupName;
	});
	const FUMGStateConfigState* State = Group ? Group->States.FindByPredicate([PreviewStateName](const FUMGStateConfigState& Candidate)
	{
		return Candidate.StateName == PreviewStateName;
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
	return FReply::Handled();
}

void SUIStateConfigPanel::ResetDesignerPreview() const
{
	TSharedPtr<FWidgetBlueprintEditor> Editor = WidgetEditor.Pin();
	if (Editor.IsValid())
	{
		Editor->RefreshPreview();
		Editor->InvalidatePreview(true);
	}
}

FReply SUIStateConfigPanel::AddParentState()
{
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
	RefreshStatesAndConfig();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::DuplicateParentState()
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigGroup* Group = GetActiveGroup();
	if (!Extension || !Group)
	{
		return FReply::Handled();
	}

	FUMGStateConfigGroup NewGroup = *Group;
	NewGroup.GroupName = MakeUniqueStateGroupName(Extension->ConfigData.StateGroups);
	NewGroup.DisplayName = FText::FromName(NewGroup.GroupName);
	Extension->ConfigData.StateGroups.Add(NewGroup);
	SelectedGroupName = NewGroup.GroupName;
	SelectedStateName = NewGroup.States.Num() > 0
		? (NewGroup.DefaultStateName.IsNone() ? NewGroup.States[0].StateName : NewGroup.DefaultStateName)
		: NAME_None;
	Extension->ConfigData.PreviewStateGroupName = SelectedGroupName;
	Extension->ConfigData.PreviewStateName = SelectedStateName;
	MarkConfigDirty(Extension);
	ApplyPreviewState();
	RefreshStatesAndConfig();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::DeleteParentState()
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();

	if (!Extension || SelectedGroupName.IsNone())
	{
		return FReply::Handled();
	}

	if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("ConfirmDeleteParentState", "确定删除当前父状态组及其全部子状态和属性配置？")) != EAppReturnType::Yes)
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
	RefreshStatesAndConfig();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::AddChildState()
{
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
	RefreshStatesAndConfig();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::DuplicateChildState()
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigGroup* Group = GetActiveGroup();
	FUMGStateConfigState* State = GetActiveState();
	if (!Extension || !Group || !State)
	{
		return FReply::Handled();
	}

	FUMGStateConfigState NewState = *State;
	NewState.StateName = MakeUniqueStateName(*Group);
	NewState.DisplayName = FText::FromName(NewState.StateName);
	Group->States.Add(NewState);
	SelectedStateName = NewState.StateName;
	Extension->ConfigData.PreviewStateGroupName = Group->GroupName;
	Extension->ConfigData.PreviewStateName = SelectedStateName;
	MarkConfigDirty(Extension);
	ApplyPreviewState();
	RefreshStatesAndConfig();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::DeleteChildState()
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigGroup* Group = GetActiveGroup();

	if (!Extension || !Group || SelectedStateName.IsNone())
	{
		return FReply::Handled();
	}

	if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("ConfirmDeleteChildState", "确定删除当前子状态及其全部属性配置？")) != EAppReturnType::Yes)
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
	RefreshStatesAndConfig();
	return FReply::Handled();
}


FReply SUIStateConfigPanel::RemoveInvalidChanges()
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigState* State = GetActiveState();
	if (!Extension || !State)
	{
		return FReply::Handled();
	}

	State->ConfiguredWidgetNames.RemoveAll([this](FName WidgetName)
	{
		return WidgetName.IsNone() || !FindWidgetByName(WidgetName);
	});
	State->PropertyChanges.RemoveAll([this](const FUMGStatePropertyChange& Change)
	{
		return Change.TargetWidgetName.IsNone() || !FindWidgetByName(Change.TargetWidgetName);
	});
	MarkConfigDirty(Extension);
	ApplyPreviewState();
	RefreshConfigOnly();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::ClearWidgetConfig(FName WidgetName)
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigState* State = GetActiveState();

	if (!Extension || !State)
	{
		return FReply::Handled();
	}

	if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("ConfirmClearWidgetConfig", "确定清除该控件在当前子状态的全部属性配置？")) != EAppReturnType::Yes)
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
	RefreshConfigOnly();
	return FReply::Handled();
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildParentStateTabs()
{
	TSharedRef<SWrapBox> Row = SNew(SWrapBox).UseAllottedSize(true);
	const UUMGStateConfigBlueprintExtension* Extension = GetExtension();
	if (!Extension)
	{
		return Row;
	}

	for (const FUMGStateConfigGroup& Group : Extension->ConfigData.StateGroups)
	{
		Row->AddSlot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
		[
			BuildStateTab(Group.GroupName, Group.GroupName == SelectedGroupName, true)
		];
	}
	return Row;
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildChildStateTabs()
{
	TSharedRef<SWrapBox> Row = SNew(SWrapBox).UseAllottedSize(true);
	FUMGStateConfigGroup* Group = GetActiveGroup();
	if (!Group)
	{
		return Row;
	}

	for (const FUMGStateConfigState& State : Group->States)
	{
		Row->AddSlot().Padding(0.0f, 0.0f, 4.0f, 4.0f)
		[
			BuildStateTab(State.StateName, State.StateName == SelectedStateName, false)
		];
	}
	return Row;
}

TSharedRef<SWidget> SUIStateConfigPanel::BuildStateTab(FName StateName, bool bSelected, bool bIsParentState)
{
	return SNew(SBorder)
	.Padding(FMargin(12.0f, 4.0f))
	.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
	.BorderBackgroundColor(bSelected
		? FAppStyle::Get().GetSlateColor("SelectionColor").GetSpecifiedColor()
		: FLinearColor(0.10f, 0.10f, 0.10f, 1.0f))
	.OnMouseButtonDown(this, &SUIStateConfigPanel::HandleStateTabMouseButtonDown, StateName, bIsParentState)
	.ToolTipText(bIsParentState
		? LOCTEXT("StateTabParentTip", "左键切换父状态 · 右键重命名")
		: LOCTEXT("StateTabChildTip", "左键切换子状态 · 右键重命名"))
	[
		SNew(STextBlock)
		.Text(FText::FromName(StateName))
		.ColorAndOpacity(bSelected ? FLinearColor::White : FLinearColor(0.72f, 0.72f, 0.72f, 1.0f))
		.Font(bSelected ? FAppStyle::GetFontStyle("BoldFont") : FAppStyle::GetFontStyle("NormalFont"))
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

	Box->AddSlot().AutoHeight().Padding(0.0f, 2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("AddCommonFromDetails", "从常用 Details 添加属性"))
		.ToolTipText(LOCTEXT("AddCommonFromDetailsTip", "打开临时 Details 面板，只捕获常用 UI 外观属性。"))
		.OnClicked_Lambda([this, WidgetName]()
		{
			FSlateApplication::Get().DismissAllMenus();
			OpenWidgetDetailsPropertyPicker(WidgetName, true);
			return FReply::Handled();
		})
	];
	Box->AddSlot().AutoHeight().Padding(0.0f, 2.0f)
	[
		SNew(SButton)
		.Text(LOCTEXT("AddAdvancedFromDetails", "从全部 Details 添加属性"))
		.ToolTipText(LOCTEXT("AddAdvancedFromDetailsTip", "打开临时 Details 面板，捕获白名单内的全部可支持属性。"))
		.OnClicked_Lambda([this, WidgetName]()
		{
			FSlateApplication::Get().DismissAllMenus();
			OpenWidgetDetailsPropertyPicker(WidgetName, false);
			return FReply::Handled();
		})
	];


	return SNew(SBorder).Padding(8.0f)[Box];
}




TSubclassOf<UWidget> SUIStateConfigPanel::GetExpectedWidgetClass(FName WidgetName) const
{
	UWidget* Widget = FindWidgetByName(WidgetName);
	return Widget ? Widget->GetClass() : UWidget::StaticClass();
}



void SUIStateConfigPanel::RenameParentState(FName OldName, FName NewName)

{
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
		RefreshStatesAndConfig();
	}
}

void SUIStateConfigPanel::RenameChildState(FName OldName, FName NewName)
{
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
		RefreshStatesAndConfig();
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

	TArray<const FUMGStatePropertyChange*> SerializedPropertyChanges;
	if (const FUMGStateConfigState* State = GetActiveState())
	{
		for (const FUMGStatePropertyChange& Change : State->PropertyChanges)
		{
			if (Change.TargetWidgetName == WidgetName && Change.PropertyType == EUMGStateConfigPropertyType::SerializedProperty)
			{
				SerializedPropertyChanges.Add(&Change);
			}
		}
	}

	TArray<FString> GroupOrder;
	TMap<FString, TArray<const FUMGStatePropertyChange*>> GroupedChanges;
	for (const FUMGStatePropertyChange* SerializedChange : SerializedPropertyChanges)
	{
		if (!SerializedChange)
		{
			continue;
		}
		FString TopSegment = SerializedChange->Value.SerializedPropertyPath;
		int32 DotIndex = INDEX_NONE;
		if (TopSegment.FindChar(TEXT('.'), DotIndex))
		{
			TopSegment = TopSegment.Left(DotIndex);
		}
		if (!GroupedChanges.Contains(TopSegment))
		{
			GroupOrder.Add(TopSegment);
		}
		GroupedChanges.FindOrAdd(TopSegment).Add(SerializedChange);
	}

	int32 RowIndex = 0;
	for (const FString& TopSegment : GroupOrder)
	{
		const TArray<const FUMGStatePropertyChange*>& GroupChanges = GroupedChanges[TopSegment];
		const bool bSingleLeaf = GroupChanges.Num() == 1 && !GroupChanges[0]->Value.SerializedPropertyPath.Contains(TEXT("."));
		if (bSingleLeaf)
		{
			Rows->AddSlot().AutoHeight().Padding(0.0f, RowIndex++ == 0 ? 4.0f : 2.0f, 0.0f, 0.0f)
			[
				BuildSerializedPropertyRow(*GroupChanges[0])
			];
			continue;
		}

		Rows->AddSlot().AutoHeight().Padding(0.0f, RowIndex++ == 0 ? 4.0f : 8.0f, 0.0f, 0.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f).VAlign(VAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("RemoveSerializedPropertyGroup", "移除整组"))
				.ToolTipText(LOCTEXT("RemoveSerializedPropertyGroupTip", "移除该控件此顶层属性下的全部配置。"))
				.OnClicked(this, &SUIStateConfigPanel::RemoveSerializedPropertyGroup, WidgetName, TopSegment)
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TopSegment))
				.Font(FAppStyle::GetFontStyle("BoldFont"))
			]
		];

		for (const FUMGStatePropertyChange* SerializedChange : GroupChanges)
		{
			FString RelativePath = SerializedChange->Value.SerializedPropertyPath;
			const FString Prefix = TopSegment + TEXT(".");
			if (RelativePath.StartsWith(Prefix))
			{
				RelativePath = RelativePath.RightChop(Prefix.Len());
			}
			Rows->AddSlot().AutoHeight().Padding(16.0f, 2.0f, 0.0f, 0.0f)
			[
				BuildSerializedPropertyRow(*SerializedChange, FText::FromString(RelativePath))
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



TSharedRef<SWidget> SUIStateConfigPanel::BuildSerializedPropertyRow(const FUMGStatePropertyChange& Change, TOptional<FText> LabelOverride)
{
	const FText Label = LabelOverride.IsSet() ? LabelOverride.GetValue() : GetSerializedPropertyDisplayName(Change.Value.SerializedPropertyPath);
	return SNew(SHorizontalBox)
	+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f).VAlign(VAlign_Center)
	[
		SNew(SButton)
		.Text(LOCTEXT("RemoveSerializedProperty", "移除"))
		.OnClicked(this, &SUIStateConfigPanel::RemoveSerializedPropertyChange, Change.TargetWidgetName, Change.Value.SerializedPropertyPath)
		.ToolTipText(LOCTEXT("RemoveSerializedPropertyTip", "移除该属性配置"))
		[
			SNew(SImage).Image(FAppStyle::GetBrush("Icons.Delete"))
		]
	]
	+ SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f).VAlign(VAlign_Center)
	[
		SNew(SBox)
		.WidthOverride(160.0f)
		[
			SNew(STextBlock).Text(Label)
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
			OpenWidgetDetailsPropertyPicker(WidgetName, false);
			return FReply::Handled();
		})

	];
}





void SUIStateConfigPanel::RefreshAll()
{
	NormalizeRedundantPropertyChanges();
	RefreshWidgetList();
	RefreshStateTabs();
	RefreshConfiguredWidgets();
	RefreshSummary();
}

void SUIStateConfigPanel::RefreshStatesAndConfig()
{
	NormalizeRedundantPropertyChanges();
	RefreshStateTabs();
	RefreshConfiguredWidgets();
	RefreshSummary();
}

void SUIStateConfigPanel::RefreshConfigOnly()
{
	NormalizeRedundantPropertyChanges();
	RefreshConfiguredWidgets();
	RefreshSummary();
}

void SUIStateConfigPanel::RequestPreviewRefresh()
{
	bPreviewRefreshPending = true;
	EnsureDeferredTimer();
}

void SUIStateConfigPanel::RequestConfigRefresh()
{
	bConfigRefreshPending = true;
	EnsureDeferredTimer();
}

void SUIStateConfigPanel::EnsureDeferredTimer()
{
	if (bDeferredTimerRegistered)
	{
		return;
	}
	bDeferredTimerRegistered = true;
	RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(this, &SUIStateConfigPanel::HandleDeferredRefresh));
}

EActiveTimerReturnType SUIStateConfigPanel::HandleDeferredRefresh(double InCurrentTime, float InDeltaTime)
{
	bDeferredTimerRegistered = false;
	if (bConfigRefreshPending)
	{
		bConfigRefreshPending = false;
		RefreshStatesAndConfig();
	}
	if (bPreviewRefreshPending)
	{
		bPreviewRefreshPending = false;
		ApplyPreviewState();
	}
	return EActiveTimerReturnType::Stop;
}

void SUIStateConfigPanel::RefreshStateTabs()
{
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
}

void SUIStateConfigPanel::RefreshWidgetList()
{
	RebuildWidgetRows();
	if (WidgetListView.IsValid())
	{
		WidgetListView->RequestListRefresh();
	}
}

void SUIStateConfigPanel::RefreshConfiguredWidgets()
{
	AppearanceDetailProxyObjects.Reset();
	DetailsProxyContexts.Reset();
	ConfiguredWidgetItems.Reset();
	for (const FName& WidgetName : GetConfiguredWidgetNames())
	{
		ConfiguredWidgetItems.Add(MakeShared<FName>(WidgetName));
	}
	if (ConfiguredWidgetsListView.IsValid())
	{
		ConfiguredWidgetsListView->RequestListRefresh();
	}
}

TSharedRef<ITableRow> SUIStateConfigPanel::GenerateConfiguredWidgetRow(TSharedPtr<FName> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	const FName WidgetName = Item.IsValid() ? *Item : NAME_None;
	return SNew(STableRow<TSharedPtr<FName>>, OwnerTable)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
		[
			BuildConfiguredWidgetCard(WidgetName)
		];
}

EVisibility SUIStateConfigPanel::GetConfiguredEmptyVisibility() const
{
	return ConfiguredWidgetItems.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed;
}


void SUIStateConfigPanel::NormalizeRedundantPropertyChanges()
{
}


void SUIStateConfigPanel::OnWidgetFilterChanged(const FText& InText)
{
	WidgetFilterText = InText.ToString();
	RebuildWidgetRows();
	if (WidgetListView.IsValid())
	{
		WidgetListView->RequestListRefresh();
	}
}

void SUIStateConfigPanel::RefreshSummary()
{
	if (SummaryTextBlock.IsValid())
	{
		SummaryTextBlock->SetText(GetSummaryText());
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
		if (!WidgetFilterText.IsEmpty() && !Widget->GetName().Contains(WidgetFilterText))
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
	ApplyPreviewState();
	RefreshStatesAndConfig();
}

void SUIStateConfigPanel::SelectChildState(FName StateName)
{
	SelectedStateName = StateName;
	ApplyPreviewState();
	RefreshStatesAndConfig();
}


TSharedRef<ITableRow> SUIStateConfigPanel::GenerateWidgetRow(TSharedPtr<FUMGStateConfigWidgetRow> RowItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	FString WidgetName = TEXT("Invalid");
	FString WidgetType = TEXT("-");
	const FSlateBrush* WidgetIcon = FSlateIconFinder::FindIconForClass(UWidget::StaticClass()).GetIcon();
	if (RowItem.IsValid())
	{
		if (UWidget* Widget = RowItem->Widget.Get())
		{
			WidgetName = Widget->GetFName().ToString();
			WidgetType = Widget->GetClass()->GetName();
			WidgetIcon = FSlateIconFinder::FindIconForClass(Widget->GetClass()).GetIcon();
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
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SImage).Image(WidgetIcon)
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
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
	RefreshConfigOnly();
}



void SUIStateConfigPanel::OpenWidgetDetailsPropertyPicker(FName WidgetName, bool bCommonOnly)
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
	FUMGStateConfigDetailsProxyContext& ProxyContext = DetailsProxyContexts.AddDefaulted_GetRef();
	ProxyContext.ProxyObject = ProxyWidget;
	ProxyContext.bCommonOnly = bCommonOnly;
	CaptureEditablePropertySnapshot(ProxyWidget, bCommonOnly, ProxyContext.Snapshot);


	FDetailsViewArgs DetailsViewArgs;

	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bShowOptions = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	if (bCommonOnly)
	{
		DetailsView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateLambda([](const FPropertyAndParent& PropertyAndParent)
		{
			return IsCommonSerializedPropertyPathOrParent(BuildDetailsPropertyPath(PropertyAndParent));
		}));
	}
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
					.Text(bCommonOnly
						? LOCTEXT("DetailsPropertyPickerHintCommon", "常用属性模式：只显示并保存常用 UI 外观属性；会通过快照 Diff 保存真实变化的叶子属性，并过滤联动噪音。")
						: LOCTEXT("DetailsPropertyPickerHintAdvanced", "全部 Details 模式：保存白名单内全部可支持属性；会通过快照 Diff 保存真实变化的叶子属性，并过滤联动噪音。"))




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
	UWidget* ProxyWidget = Cast<UWidget>(ProxyObject.Get());
	if (!ProxyWidget)
	{
		return;
	}

	FUMGStateConfigDetailsProxyContext* ProxyContext = FindDetailsProxyContext(ProxyWidget);
	if (!ProxyContext)
	{
		return;
	}

	TMap<FString, FUMGStateConfigPropertyValue> CurrentSnapshot;
	CaptureEditablePropertySnapshot(ProxyWidget, ProxyContext->bCommonOnly, CurrentSnapshot);

	TArray<FUMGStateConfigPropertyValue> ChangedValues;
	DiffPropertySnapshot(ProxyContext->Snapshot, CurrentSnapshot, ChangedValues);
	const int32 ChangedCountBeforeFilter = ChangedValues.Num();
	TArray<FString> FilteredPaths;
	FilterNoisyPropertyChanges(ChangedValues, FilteredPaths);

	for (const FUMGStateConfigPropertyValue& Value : ChangedValues)
	{
		AddOrUpdateSerializedPropertyChange(WidgetName, Value, ProxyWidget->GetClass());
	}

	ProxyContext->Snapshot = MoveTemp(CurrentSnapshot);
	const int32 FilteredCount = FilteredPaths.Num();
	const int32 SavedCount = ChangedValues.Num();
	if (SavedCount > 0)
	{
		LastDetailsCaptureMessage = FText::Format(
			LOCTEXT("DetailsCaptureSaved", "最近 Details 捕获：保存 {0} 条属性，过滤 {1} 条联动属性。"),
			FText::AsNumber(SavedCount),
			FText::AsNumber(FilteredCount));
		RequestConfigRefresh();
	}
	else
	{
		LastDetailsCaptureMessage = ChangedCountBeforeFilter > 0
			? FText::Format(LOCTEXT("DetailsCaptureOnlyFiltered", "最近 Details 捕获：检测到 {0} 条变化，但均被噪音规则过滤。"), FText::AsNumber(ChangedCountBeforeFilter))
			: LOCTEXT("DetailsCaptureNoChanges", "最近 Details 捕获：没有检测到可保存变化。常用属性模式下，可尝试“全部 Details”。");
		RefreshSummary();
	}
}

void SUIStateConfigPanel::CaptureEditablePropertySnapshot(UWidget* Widget, bool bCommonOnly, TMap<FString, FUMGStateConfigPropertyValue>& OutSnapshot) const
{
	OutSnapshot.Reset();
	if (!Widget)
	{
		return;
	}

	CaptureEditablePropertySnapshotRecursive(Widget, Widget->GetClass(), Widget, FString(), bCommonOnly, OutSnapshot);
}

void SUIStateConfigPanel::CaptureEditablePropertySnapshotRecursive(UWidget* Widget, UStruct* CurrentStruct, void* CurrentContainer, const FString& PathPrefix, bool bCommonOnly, TMap<FString, FUMGStateConfigPropertyValue>& OutSnapshot) const

{
	if (!Widget || !CurrentStruct || !CurrentContainer)
	{
		return;
	}

	for (TFieldIterator<FProperty> It(CurrentStruct); It; ++It)
	{
		FProperty* Property = *It;
		if (!IsSnapshotPropertySupported(Property))
		{
			continue;
		}

		const FString PropertyPath = PathPrefix.IsEmpty()
			? Property->GetName()
			: FString::Printf(TEXT("%s.%s"), *PathPrefix, *Property->GetName());
		void* ValuePtr = Property->ContainerPtrToValuePtr<void>(CurrentContainer);
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (PropertyPath != TEXT("Text"))
			{
				CaptureEditablePropertySnapshotRecursive(Widget, StructProperty->Struct, ValuePtr, PropertyPath, bCommonOnly, OutSnapshot);
				continue;
			}
		}

		if (bCommonOnly && !IsCommonSerializedPropertyPath(PropertyPath))
		{
			continue;
		}

		FUMGStateConfigPropertyValue Value;
		Value.SerializedPropertyPath = PropertyPath;
		if (FUMGStateConfigPropertyRuntimeLibrary::CaptureCurrentValue(Widget, EUMGStateConfigPropertyType::SerializedProperty, Value))
		{
			OutSnapshot.Add(PropertyPath, Value);
		}


	}
}

void SUIStateConfigPanel::DiffPropertySnapshot(const TMap<FString, FUMGStateConfigPropertyValue>& BeforeSnapshot, const TMap<FString, FUMGStateConfigPropertyValue>& AfterSnapshot, TArray<FUMGStateConfigPropertyValue>& OutChangedValues) const
{
	OutChangedValues.Reset();
	for (const TPair<FString, FUMGStateConfigPropertyValue>& AfterPair : AfterSnapshot)
	{
		const FUMGStateConfigPropertyValue* BeforeValue = BeforeSnapshot.Find(AfterPair.Key);
		if (!BeforeValue || !FUMGStateConfigPropertyRuntimeLibrary::ArePropertyValuesEqual(EUMGStateConfigPropertyType::SerializedProperty, *BeforeValue, AfterPair.Value))
		{
			OutChangedValues.Add(AfterPair.Value);
		}
	}
}

void SUIStateConfigPanel::FilterNoisyPropertyChanges(TArray<FUMGStateConfigPropertyValue>& InOutChangedValues, TArray<FString>& OutFilteredPaths) const
{
	OutFilteredPaths.Reset();
	for (int32 CandidateIndex = InOutChangedValues.Num() - 1; CandidateIndex >= 0; --CandidateIndex)
	{
		const FString CandidatePath = InOutChangedValues[CandidateIndex].SerializedPropertyPath;
		for (const FUMGStateConfigPropertyValue& PrimaryValue : InOutChangedValues)
		{
			const FString& PrimaryPath = PrimaryValue.SerializedPropertyPath;
			if (CandidatePath != PrimaryPath && IsNoisyPropertySuppressedByPrimary(CandidatePath, PrimaryPath))
			{
				OutFilteredPaths.Add(CandidatePath);
				InOutChangedValues.RemoveAt(CandidateIndex);
				break;
			}
		}
	}
}


FUMGStateConfigDetailsProxyContext* SUIStateConfigPanel::FindDetailsProxyContext(UObject* ProxyObject)
{
	if (!ProxyObject)
	{
		return nullptr;
	}

	return DetailsProxyContexts.FindByPredicate([ProxyObject](const FUMGStateConfigDetailsProxyContext& Context)
	{
		return Context.ProxyObject.Get() == ProxyObject;
	});
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
	RequestPreviewRefresh();
}



FReply SUIStateConfigPanel::RemoveSerializedPropertyChange(FName WidgetName, FString SerializedPropertyPath)
{
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
	RefreshConfigOnly();
	return FReply::Handled();
}

FReply SUIStateConfigPanel::RemoveSerializedPropertyGroup(FName WidgetName, FString TopLevelSegment)
{
	UUMGStateConfigBlueprintExtension* Extension = GetOrCreateExtension();
	FUMGStateConfigState* State = GetActiveState();
	if (!Extension || !State)
	{
		return FReply::Handled();
	}

	const FString Prefix = TopLevelSegment + TEXT(".");
	State->PropertyChanges.RemoveAll([&WidgetName, &TopLevelSegment, &Prefix](const FUMGStatePropertyChange& Change)
	{
		return Change.TargetWidgetName == WidgetName
			&& Change.PropertyType == EUMGStateConfigPropertyType::SerializedProperty
			&& (Change.Value.SerializedPropertyPath == TopLevelSegment
				|| Change.Value.SerializedPropertyPath.StartsWith(Prefix));
	});
	MarkConfigDirty(Extension);
	ApplyPreviewState();
	RefreshConfigOnly();
	return FReply::Handled();
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







FText SUIStateConfigPanel::GetSerializedPropertyDisplayName(const FString& SerializedPropertyPath) const
{
	if (SerializedPropertyPath == TEXT("Visibility"))
	{
		return LOCTEXT("PropertyDisplayVisibility", "Visibility");
	}
	if (SerializedPropertyPath == TEXT("RenderOpacity"))
	{
		return LOCTEXT("PropertyDisplayRenderOpacity", "Render Opacity");
	}
	if (SerializedPropertyPath == TEXT("Text"))
	{
		return LOCTEXT("PropertyDisplayText", "Text");
	}
	if (SerializedPropertyPath == TEXT("Brush.ResourceObject"))
	{
		return LOCTEXT("PropertyDisplayBrushImage", "Brush Image");
	}
	if (SerializedPropertyPath.StartsWith(TEXT("Brush.TintColor")) || SerializedPropertyPath.StartsWith(TEXT("ColorAndOpacity")))
	{
		return LOCTEXT("PropertyDisplayColorAndOpacity", "Color and Opacity");
	}
	if (SerializedPropertyPath.StartsWith(TEXT("Brush.ImageSize")))
	{
		return LOCTEXT("PropertyDisplayBrushImageSize", "Brush Image Size");
	}
	if (SerializedPropertyPath.StartsWith(TEXT("Brush")))
	{
		return FText::Format(LOCTEXT("PropertyDisplayBrushFallback", "Brush / {0}"), FText::FromString(SerializedPropertyPath.RightChop(6)));
	}
	if (SerializedPropertyPath.StartsWith(TEXT("Font")))
	{
		return FText::Format(LOCTEXT("PropertyDisplayFontFallback", "Font / {0}"), FText::FromString(SerializedPropertyPath.RightChop(5)));
	}
	if (SerializedPropertyPath.StartsWith(TEXT("RenderTransform")))
	{
		return FText::Format(LOCTEXT("PropertyDisplayRenderTransformFallback", "Render Transform / {0}"), FText::FromString(SerializedPropertyPath.RightChop(16)));
	}
	if (SerializedPropertyPath.StartsWith(TEXT("Shadow")))
	{
		return FText::Format(LOCTEXT("PropertyDisplayShadowFallback", "Shadow / {0}"), FText::FromString(SerializedPropertyPath));
	}

	FString DisplayPath = SerializedPropertyPath;
	DisplayPath.ReplaceInline(TEXT("."), TEXT(" / "));
	return FText::FromString(DisplayPath);
}

FString SUIStateConfigPanel::FormatPropertyChange(const FUMGStatePropertyChange& Change) const

{
	const UEnum* PropertyEnum = StaticEnum<EUMGStateConfigPropertyType>();
	const FString PropertyName = PropertyEnum ? PropertyEnum->GetNameStringByValue(static_cast<int64>(Change.PropertyType)) : TEXT("Unknown");
	return FString::Printf(TEXT("%s.%s"), *Change.TargetWidgetName.ToString(), *PropertyName);
}

void SUIStateConfigPanel::MarkConfigDirty(UUMGStateConfigBlueprintExtension* Extension)
{
	UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	if (WidgetBlueprint)
	{
		WidgetBlueprint->Modify();
		FBlueprintEditorUtils::MarkBlueprintAsModified(WidgetBlueprint);
		WidgetBlueprint->MarkPackageDirty();
	}

	if (Extension)
	{
		Extension->Modify();
	}
}

#undef LOCTEXT_NAMESPACE
