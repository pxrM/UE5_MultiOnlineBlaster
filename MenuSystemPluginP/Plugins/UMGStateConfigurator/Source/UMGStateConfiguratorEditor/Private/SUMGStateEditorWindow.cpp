// Copyright Epic Games, Inc. All Rights Reserved.

#include "SUMGStateEditorWindow.h"
#include "UMGStateController.h"
#include "Blueprint/UserWidget.h"
#include "Misc/MessageDialog.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorFontGlyphs.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "SUMGStateEditorWindow"

void SUMGStateEditorWindow::Construct(const FArguments& InArgs)
{
    WeakController = InArgs._StateController;
    WeakPreviewWidget = InArgs._PreviewWidget;

    ChildSlot
    [
        SNew(SVerticalBox)

        // 顶部标题栏
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10, 10, 10, 5)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(STextBlock)
                .Text(LOCTEXT("WindowTitle", "🎨 UI State Configuration"))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
                .ColorAndOpacity(FLinearColor(0.8f, 0.9f, 1.0f))
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("RefreshButton", "🔄 Refresh"))
                .ToolTipText(LOCTEXT("RefreshTooltip", "Refresh the entire UI"))
                .OnClicked(this, &SUMGStateEditorWindow::OnRefreshUI)
            ]
        ]

        // 分隔线
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SSeparator)
            .Thickness(2.0f)
        ]

        // 主内容区域（可滚动）
        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(10)
        [
            SAssignNew(ContentScrollBox, SScrollBox)
            .Orientation(Orient_Vertical)
            + SScrollBox::Slot()
            [
                BuildStateManagerUI()
            ]
        ]

        // 底部工具栏
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10, 5)
        [
            SNew(SSeparator)
            .Thickness(1.0f)
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(10, 5, 10, 10)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .ContentPadding(FMargin(20, 10))
                .ButtonStyle(FAppStyle::Get(), "FlatButton.Success")
                .OnClicked(this, &SUMGStateEditorWindow::OnQuickRecordCurrentView)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 5, 0)
                    [
                        SNew(STextBlock)
                        .Font(FAppStyle::Get().GetFontStyle("FontAwesome.14"))
                        .Text(FEditorFontGlyphs::Circle)
                        .ColorAndOpacity(FLinearColor::Red)
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("QuickRecord", "Quick Record Current View"))
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
                    ]
                ]
            ]
        ]
    ];

    RefreshUI();
}

TSharedRef<SWidget> SUMGStateEditorWindow::BuildStateManagerUI()
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller)
    {
        return SNew(STextBlock)
            .Text(LOCTEXT("NoController", "No State Controller selected"))
            .Justification(ETextJustify::Center);
    }

    TSharedRef<SVerticalBox> MainBox = SNew(SVerticalBox);

    // 为每个分类构建 UI
    for (int32 i = 0; i < Controller->StateCategories.Num(); ++i)
    {
        MainBox->AddSlot()
            .AutoHeight()
            .Padding(0, 5)
            [
                BuildCategoryUI(i)
            ];
    }

    // 添加 "新建分类" 按钮
    MainBox->AddSlot()
        .AutoHeight()
        .Padding(0, 10)
        [
            SNew(SButton)
            .HAlign(HAlign_Center)
            .OnClicked(this, &SUMGStateEditorWindow::OnAddCategory)
            .ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
            .ContentPadding(FMargin(10, 5))
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                .Padding(0, 0, 5, 0)
                [
                    SNew(STextBlock)
                    .Font(FAppStyle::Get().GetFontStyle("FontAwesome.11"))
                    .Text(FEditorFontGlyphs::Plus_Circle)
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .VAlign(VAlign_Center)
                [
                    SNew(STextBlock)
                    .Text(LOCTEXT("AddCategory", "Add New State Category"))
                ]
            ]
        ];

    return MainBox;
}

TSharedRef<SWidget> SUMGStateEditorWindow::BuildStateListUI(int32 CategoryIndex)
{
    TSharedRef<SVerticalBox> StatesBox = SNew(SVerticalBox);

    UUMGStateController* Controller = WeakController.Get();
    if (Controller && Controller->StateCategories.IsValidIndex(CategoryIndex))
    {
        const FUIStateCategory& Cat = Controller->StateCategories[CategoryIndex];
        for (int32 StateIdx = 0; StateIdx < Cat.States.Num(); ++StateIdx)
        {
            StatesBox->AddSlot()
                .AutoHeight()
                .Padding(0, 2)
                [
                    BuildStateRowUI(CategoryIndex, StateIdx)
                ];
        }
    }

    return StatesBox;
}

TSharedRef<SWidget> SUMGStateEditorWindow::BuildCategoryUI(int32 CategoryIndex)
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
    {
        return SNullWidget::NullWidget;
    }

    const FUIStateCategory& Category = Controller->StateCategories[CategoryIndex];

    return SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        .Padding(10)
        [
            SNew(SVerticalBox)

            // 分类标题行
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 10)
            [
                SNew(SHorizontalBox)

                // 分类名称输入框
                + SHorizontalBox::Slot()
                .FillWidth(1.0f)
                .VAlign(VAlign_Center)
                [
                    SNew(SEditableTextBox)
                    .Text(this, &SUMGStateEditorWindow::GetCategoryNameText, CategoryIndex)
                    .OnTextChanged(this, &SUMGStateEditorWindow::OnCategoryNameChanged, CategoryIndex)
                    .OnTextCommitted(this, &SUMGStateEditorWindow::OnCategoryNameCommitted, CategoryIndex)
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
                    .HintText(LOCTEXT("CategoryNameHint", "Category Name (e.g., ButtonState)"))
                ]

                // 删除按钮
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(5, 0, 0, 0)
                .VAlign(VAlign_Center)
                [
                    SNew(SButton)
                    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                    .ToolTipText(LOCTEXT("DeleteCategory", "Delete this category"))
                    .OnClicked(this, &SUMGStateEditorWindow::OnDeleteCategory, CategoryIndex)
                    [
                        SNew(STextBlock)
                        .Font(FAppStyle::Get().GetFontStyle("FontAwesome.11"))
                        .Text(FEditorFontGlyphs::Trash)
                        .ColorAndOpacity(FLinearColor(1.0f, 0.3f, 0.3f))
                    ]
                ]
            ]

            // 分隔线
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 0, 0, 5)
            [
                SNew(SSeparator)
            ]

            // 状态列表
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                BuildStateListUI(CategoryIndex)
            ]

            // 添加状态按钮
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(0, 5, 0, 0)
            [
                SNew(SButton)
                .HAlign(HAlign_Left)
                .OnClicked(this, &SUMGStateEditorWindow::OnAddState, CategoryIndex)
                .ButtonStyle(FAppStyle::Get(), "FlatButton.Default")
                .ContentPadding(FMargin(5, 3))
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 5, 0)
                    [
                        SNew(STextBlock)
                        .Font(FAppStyle::Get().GetFontStyle("FontAwesome.9"))
                        .Text(FEditorFontGlyphs::Plus)
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .VAlign(VAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("AddState", "Add State"))
                    ]
                ]
            ]
        ];
}

TSharedRef<SWidget> SUMGStateEditorWindow::BuildStateRowUI(int32 CategoryIndex, int32 StateIndex)
{
    bool bIsRecording = IsStateRecording(CategoryIndex, StateIndex);

    return SNew(SBorder)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.DarkGroupBorder"))
        .Padding(5)
        [
            SNew(SHorizontalBox)

            // 状态名称
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            .VAlign(VAlign_Center)
            .Padding(0, 0, 5, 0)
            [
                SNew(SEditableTextBox)
                .Text(this, &SUMGStateEditorWindow::GetStateNameText, CategoryIndex, StateIndex)
                .OnTextChanged(this, &SUMGStateEditorWindow::OnStateNameChanged, CategoryIndex, StateIndex)
                .OnTextCommitted(this, &SUMGStateEditorWindow::OnStateNameCommitted, CategoryIndex, StateIndex)
                .HintText(LOCTEXT("StateNameHint", "State Name (e.g., Normal, Hovered, Pressed)"))
            ]

            // 预览按钮
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2, 0)
            .VAlign(VAlign_Center)
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ToolTipText(LOCTEXT("PreviewState", "Preview this state"))
                .OnClicked(this, &SUMGStateEditorWindow::OnPreviewState, CategoryIndex, StateIndex)
                [
                    SNew(STextBlock)
                    .Font(FAppStyle::Get().GetFontStyle("FontAwesome.11"))
                    .Text(FEditorFontGlyphs::Eye)
                    .ColorAndOpacity(FLinearColor(0.7f, 0.9f, 1.0f))
                ]
            ]

            // 录制按钮
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2, 0)
            .VAlign(VAlign_Center)
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ToolTipText(bIsRecording ? 
                    LOCTEXT("StopRecording", "Stop recording") : 
                    LOCTEXT("StartRecording", "Start recording property changes"))
                .OnClicked(this, &SUMGStateEditorWindow::OnToggleRecordMode, CategoryIndex, StateIndex)
                [
                    SNew(STextBlock)
                    .Font(FAppStyle::Get().GetFontStyle("FontAwesome.11"))
                    .Text(FEditorFontGlyphs::Circle)
                    .ColorAndOpacity(bIsRecording ? FLinearColor::Red : FLinearColor(0.5f, 0.5f, 0.5f))
                ]
            ]

            // 删除按钮
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2, 0)
            .VAlign(VAlign_Center)
            [
                SNew(SButton)
                .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                .ToolTipText(LOCTEXT("DeleteState", "Delete this state"))
                .OnClicked(this, &SUMGStateEditorWindow::OnDeleteState, CategoryIndex, StateIndex)
                [
                    SNew(STextBlock)
                    .Font(FAppStyle::Get().GetFontStyle("FontAwesome.9"))
                    .Text(FEditorFontGlyphs::Times)
                    .ColorAndOpacity(FLinearColor(1.0f, 0.5f, 0.5f))
                ]
            ]
        ];
}

void SUMGStateEditorWindow::RefreshUI()
{
    if (ContentScrollBox.IsValid())
    {
        ContentScrollBox->ClearChildren();
        ContentScrollBox->AddSlot()
            [
                BuildStateManagerUI()
            ];
    }
}

FReply SUMGStateEditorWindow::OnAddCategory()
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller) return FReply::Handled();

    const FScopedTransaction Transaction(LOCTEXT("AddCategory", "Add State Category"));
    Controller->Modify();

    FUIStateCategory NewCategory;
    NewCategory.EnumName = FString::Printf(TEXT("Category_%d"), Controller->StateCategories.Num());
    Controller->StateCategories.Add(NewCategory);
    Controller->InvalidateSnapshot();

    RefreshUI();
    return FReply::Handled();
}

FReply SUMGStateEditorWindow::OnDeleteCategory(int32 CategoryIndex)
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return FReply::Handled();

    // U1: Delete confirmation
    if (Controller->StateCategories.Num() > 1)
    {
        EAppReturnType::Type Result = FMessageDialog::Open(
            EAppMsgType::YesNo,
            LOCTEXT("ConfirmDeleteCategory", "Are you sure you want to delete this category and all its states?"));
        if (Result != EAppReturnType::Yes)
        {
            return FReply::Handled();
        }
    }

    const FScopedTransaction Transaction(LOCTEXT("DeleteCategory", "Delete State Category"));
    Controller->Modify();

    Controller->StateCategories.RemoveAt(CategoryIndex);
    Controller->InvalidateSnapshot();

    RefreshUI();
    return FReply::Handled();
}

FReply SUMGStateEditorWindow::OnAddState(int32 CategoryIndex)
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return FReply::Handled();

    const FScopedTransaction Transaction(LOCTEXT("AddState", "Add State"));
    Controller->Modify();

    FUIStateGroup NewState;
    NewState.StateName = FString::Printf(TEXT("State_%d"),
        Controller->StateCategories[CategoryIndex].States.Num());
    Controller->StateCategories[CategoryIndex].States.Add(NewState);
    Controller->InvalidateSnapshot();

    RefreshUI();
    return FReply::Handled();
}

FReply SUMGStateEditorWindow::OnDeleteState(int32 CategoryIndex, int32 StateIndex)
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return FReply::Handled();

    FUIStateCategory& Category = Controller->StateCategories[CategoryIndex];
    if (!Category.States.IsValidIndex(StateIndex))
        return FReply::Handled();

    // U1: Delete confirmation
    if (Category.States.Num() > 1)
    {
        EAppReturnType::Type Result = FMessageDialog::Open(
            EAppMsgType::YesNo,
            LOCTEXT("ConfirmDeleteState", "Are you sure you want to delete this state?"));
        if (Result != EAppReturnType::Yes)
        {
            return FReply::Handled();
        }
    }

    const FScopedTransaction Transaction(LOCTEXT("DeleteState", "Delete State"));
    Controller->Modify();

    Category.States.RemoveAt(StateIndex);
    Controller->InvalidateSnapshot();

    RefreshUI();
    return FReply::Handled();
}

FReply SUMGStateEditorWindow::OnPreviewState(int32 CategoryIndex, int32 StateIndex)
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return FReply::Handled();

    FUIStateCategory& Category = Controller->StateCategories[CategoryIndex];
    if (!Category.States.IsValidIndex(StateIndex))
        return FReply::Handled();

    const FUIStateGroup& State = Category.States[StateIndex];

    // 设置活跃状态
    Category.ActiveStateName = State.StateName;

    // 应用状态
    Controller->SetState(Category.EnumName, State.StateName);

    UE_LOG(LogTemp, Log, TEXT("Previewing state: %s - %s"), *Category.EnumName, *State.StateName);

    return FReply::Handled();
}

FReply SUMGStateEditorWindow::OnToggleRecordMode(int32 CategoryIndex, int32 StateIndex)
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return FReply::Handled();

    FUIStateCategory& Category = Controller->StateCategories[CategoryIndex];
    if (!Category.States.IsValidIndex(StateIndex))
        return FReply::Handled();

    const FScopedTransaction Transaction(LOCTEXT("ToggleRecord", "Toggle Record Mode"));
    Controller->Modify();

    FUIStateGroup& TargetState = Category.States[StateIndex];
    bool bNewRecordMode = !TargetState.bRecordMode;

    // 如果开启录制，关闭所有其他状态的录制
    if (bNewRecordMode)
    {
        for (int32 i = 0; i < Controller->StateCategories.Num(); ++i)
        {
            FUIStateCategory& Cat = Controller->StateCategories[i];
            for (int32 j = 0; j < Cat.States.Num(); ++j)
            {
                if (i == CategoryIndex && j == StateIndex) continue;
                Cat.States[j].bRecordMode = false;
            }
        }

        // 自动切换预览到该状态
        Controller->SetState(Category.EnumName, TargetState.StateName);
        Category.ActiveStateName = TargetState.StateName;
    }

    TargetState.bRecordMode = bNewRecordMode;

    RefreshUI();

    UE_LOG(LogTemp, Log, TEXT("Record mode %s for: %s - %s"), 
        bNewRecordMode ? TEXT("ENABLED") : TEXT("DISABLED"),
        *Category.EnumName, *TargetState.StateName);

    return FReply::Handled();
}

FReply SUMGStateEditorWindow::OnQuickRecordCurrentView()
{
    // 找到当前正在录制的状态，直接应用
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller) return FReply::Handled();

    for (int32 i = 0; i < Controller->StateCategories.Num(); ++i)
    {
        FUIStateCategory& Category = Controller->StateCategories[i];
        for (int32 j = 0; j < Category.States.Num(); ++j)
        {
            if (Category.States[j].bRecordMode)
            {
                UE_LOG(LogTemp, Log, TEXT("Quick record triggered for: %s - %s"), 
                    *Category.EnumName, *Category.States[j].StateName);
                return FReply::Handled();
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("No state is currently in record mode"));
    return FReply::Handled();
}

bool SUMGStateEditorWindow::IsStateRecording(int32 CategoryIndex, int32 StateIndex) const
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return false;

    const FUIStateCategory& Category = Controller->StateCategories[CategoryIndex];
    if (!Category.States.IsValidIndex(StateIndex))
        return false;

    return Category.States[StateIndex].bRecordMode;
}

FText SUMGStateEditorWindow::GetStateNameText(int32 CategoryIndex, int32 StateIndex) const
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return FText::GetEmpty();

    const FUIStateCategory& Category = Controller->StateCategories[CategoryIndex];
    if (!Category.States.IsValidIndex(StateIndex))
        return FText::GetEmpty();

    return FText::FromString(Category.States[StateIndex].StateName);
}

void SUMGStateEditorWindow::OnStateNameChanged(const FText& NewText, int32 CategoryIndex, int32 StateIndex)
{
    // 实时更新（可选）
}

void SUMGStateEditorWindow::OnStateNameCommitted(const FText& NewText, ETextCommit::Type CommitType, int32 CategoryIndex, int32 StateIndex)
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return;

    FUIStateCategory& Category = Controller->StateCategories[CategoryIndex];
    if (!Category.States.IsValidIndex(StateIndex))
        return;

    // C6: Reject empty names
    if (NewText.ToString().TrimStartAndEnd().IsEmpty())
    {
        RefreshUI();
        return;
    }

    const FScopedTransaction Transaction(LOCTEXT("RenameState", "Rename State"));
    Controller->Modify();

    Category.States[StateIndex].StateName = NewText.ToString();
}

FText SUMGStateEditorWindow::GetCategoryNameText(int32 CategoryIndex) const
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return FText::GetEmpty();

    return FText::FromString(Controller->StateCategories[CategoryIndex].EnumName);
}

void SUMGStateEditorWindow::OnCategoryNameChanged(const FText& NewText, int32 CategoryIndex)
{
    // 实时更新（可选）
}

void SUMGStateEditorWindow::OnCategoryNameCommitted(const FText& NewText, ETextCommit::Type CommitType, int32 CategoryIndex)
{
    UUMGStateController* Controller = WeakController.Get();
    if (!Controller || !Controller->StateCategories.IsValidIndex(CategoryIndex))
        return;

    // C6: Reject empty names
    if (NewText.ToString().TrimStartAndEnd().IsEmpty())
    {
        RefreshUI();
        return;
    }

    const FScopedTransaction Transaction(LOCTEXT("RenameCategory", "Rename Category"));
    Controller->Modify();

    Controller->StateCategories[CategoryIndex].EnumName = NewText.ToString();
}

FReply SUMGStateEditorWindow::OnRefreshUI()
{
    RefreshUI();
    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
