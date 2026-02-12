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
	// === UI回调函数 ===
	/** 生成树视图行 */
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FUMGReflectorItem> InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	/** 获取子节点 */
	void OnGetChildren(TSharedPtr<FUMGReflectorItem> InItem, TArray<TSharedPtr<FUMGReflectorItem>>& OutChildren);
	/** 选中变化回调 */
	void OnSelectionChanged(TSharedPtr<FUMGReflectorItem> InItem, ESelectInfo::Type SelectionType);
	/** 刷新按钮点击 */
	FReply OnRefreshButtonClicked();
	/** 搜索文本变化 */
	void OnSearchTextChanged(const FText& InText);
	/** 自动刷新复选框状态变化 */
	void OnAutoRefreshChanged(ECheckBoxState NewState);
	/** 清空搜索按钮点击 */
	FReply OnClearSearchClicked();

	// === 核心逻辑函数 ===
	/** 
	 * 递归构建UMG Widget树
	 * @param InWBPWidget 当前UserWidget
	 * @param InCurrentWidget 当前Slate Widget
	 * @param InParent 父节点
	 */
	void BuildUMGWidgetTree(const UUserWidget* InWBPWidget, const TSharedPtr<SWidget>& InCurrentWidget, const TSharedPtr<FUMGReflectorItem>& InParent);
	/** 
	 * 获取Widget在UMG中的名称
	 * @param InWidget UserWidget实例
	 * @param InSlateWidget Slate Widget
	 * @return Widget名称
	 */
	FString GetUMGWidgetName(const UUserWidget* InWidget, const TSharedPtr<SWidget> InSlateWidget);
	/** 
	 * 查找所有在视口中的所有UserWidget
	 * @param InWorld PIE World
	 * @param OutAllUserWidget 输出所有找到的Widget
	 */
	void FindAllUserWidget(const UWorld* InWorld, TArray<TSharedPtr<FUMGReflectorItem>>& OutAllUserWidget);
	/** 
	 * 过滤Widget项（用于搜索）
	 * @param InItem 要检查的节点
	 * @param SearchString 搜索字符串
	 * @return 是否匹配搜索条件
	 */
	bool FilterWidgetItem(const TSharedPtr<FUMGReflectorItem>& InItem, const FString& SearchString) const;
	/** 
	 * 递归过滤树节点
	 */
	void FilterTreeItems(const TArray<TSharedPtr<FUMGReflectorItem>>& SourceItems, TArray<TSharedPtr<FUMGReflectorItem>>& FilteredItems, const FString& SearchString) const;
	/**
	 * 递归检查节点是否有匹配的后代
	 */
	bool HasMatchingDescendants(const TSharedPtr<FUMGReflectorItem>& Item, const FString& SearchString) const;
	/**
	 * 递归展开匹配项
	 */
	void ExpandMatchingItems(const TSharedPtr<FUMGReflectorItem>& Item, const FString& SearchString);
	/**
	 * 递归折叠节点
	 */
	void CollapseItemRecursively(const TSharedPtr<FUMGReflectorItem>& Item);
	
	// === 编辑器事件回调 ===
	/** PIE启动回调 */
	void OnPostPIEStarted(bool bIsSimulating);
	/** PIE结束回调 */
	void OnEndPIE(bool bIsSimulating);

	// === 初始化函数 ===
	/** 创建属性详情面板 */
	void CreateInstanceDetailsView();
	/** 启动自动刷新定时器 */
	void StartAutoRefreshTimer();
	/** 停止自动刷新定时器 */
	void StopAutoRefreshTimer();
	/** 自动刷新定时器回调 */
	void OnAutoRefreshTimer();

	
private:
	// === UI组件 ===
	
	/** Widget树视图 */
	TSharedPtr<STreeView<TSharedPtr<FUMGReflectorItem>>> UMGTreeViewSlate;
	
	/** 根节点数组 */
	TArray<TSharedPtr<FUMGReflectorItem>> UMGRootItems;
	
	/** 过滤后的根节点（用于搜索） */
	TArray<TSharedPtr<FUMGReflectorItem>> FilteredRootItems;

	/** 搜索框 */
	TSharedPtr<SSearchBox> SearchBox;

	/** 自动刷新复选框 */
	TSharedPtr<SCheckBox> AutoRefreshCheckBox;
	
	/** Tab管理器 */
	TSharedPtr<FTabManager> TabManager;
	
#if WITH_EDITOR
	/** 属性详情面板 */
	TSharedPtr<IDetailsView> PropertyViewPtr;
#endif

	// === 配置参数 ===
	
	/** 当前搜索文本 */
	FString CurrentSearchText;
	
	/** 是否启用自动刷新 */
	bool bAutoRefreshEnabled = false;
	
	/** 自动刷新间隔（秒） */
	float AutoRefreshInterval = 1.0f;
	
	/** 自动刷新定时器句柄 */
	FTimerHandle AutoRefreshTimerHandle;
};
