// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class UUMGStateController;
class UUserWidget;
class IDetailsView;
class SScrollBox;

/**
 * 独立的 UI 状态编辑器窗口
 * 为美术人员提供友好的状态配置界面
 */
class SUMGStateEditorWindow : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SUMGStateEditorWindow) {}
        SLATE_ARGUMENT(UUMGStateController*, StateController)
        SLATE_ARGUMENT(UUserWidget*, PreviewWidget)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    /** 刷新整个 UI */
    void RefreshUI();

private:
    /** 当前编辑的 StateController */
    TWeakObjectPtr<UUMGStateController> WeakController;
    
    /** 预览的 UserWidget */
    TWeakObjectPtr<UUserWidget> WeakPreviewWidget;

    /** 主内容滚动框 */
    TSharedPtr<SScrollBox> ContentScrollBox;

    /** 构建状态管理器 UI */
    TSharedRef<SWidget> BuildStateManagerUI();

    /** 构建单个分类的 UI */
    TSharedRef<SWidget> BuildCategoryUI(int32 CategoryIndex);

    /** 构建分类下的状态列表 */
    TSharedRef<SWidget> BuildStateListUI(int32 CategoryIndex);

    /** 构建单个状态的 UI 行 */
    TSharedRef<SWidget> BuildStateRowUI(int32 CategoryIndex, int32 StateIndex);

    /** 添加新分类 */
    FReply OnAddCategory();

    /** 删除分类 */
    FReply OnDeleteCategory(int32 CategoryIndex);

    /** 添加新状态 */
    FReply OnAddState(int32 CategoryIndex);

    /** 删除状态 */
    FReply OnDeleteState(int32 CategoryIndex, int32 StateIndex);

    /** 预览状态 */
    FReply OnPreviewState(int32 CategoryIndex, int32 StateIndex);

    /** 切换录制模式 */
    FReply OnToggleRecordMode(int32 CategoryIndex, int32 StateIndex);

    /** 快速录制当前视图 */
    FReply OnQuickRecordCurrentView();

    /** 检查某个状态是否正在录制 */
    bool IsStateRecording(int32 CategoryIndex, int32 StateIndex) const;

    /** 获取状态名称文本 */
    FText GetStateNameText(int32 CategoryIndex, int32 StateIndex) const;

    /** 状态名称改变回调 */
    void OnStateNameChanged(const FText& NewText, int32 CategoryIndex, int32 StateIndex);

    /** 状态名称提交回调 */
    void OnStateNameCommitted(const FText& NewText, ETextCommit::Type CommitType, int32 CategoryIndex, int32 StateIndex);

    /** 获取分类名称文本 */
    FText GetCategoryNameText(int32 CategoryIndex) const;

    /** 分类名称改变回调 */
    void OnCategoryNameChanged(const FText& NewText, int32 CategoryIndex);

    /** 分类名称提交回调 */
    void OnCategoryNameCommitted(const FText& NewText, ETextCommit::Type CommitType, int32 CategoryIndex);

    /** 应用所有活跃状态（用于预览） */
    void ApplyAllActiveStates();
};
