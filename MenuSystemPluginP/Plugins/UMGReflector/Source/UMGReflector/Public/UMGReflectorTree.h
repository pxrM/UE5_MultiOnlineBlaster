#pragma once

#include "CoreMinimal.h"
#include "Layout/WidgetPath.h"
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
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;
	virtual ~SUMGReflectorTree() override;
	

private:
	void UpdateWidgetTree();
	
	// === UI回调函数 ===
	/** 生成树视图行 */
	TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FUMGReflectorItem> InItem, const TSharedRef<STableViewBase>& InOwnerTable);
	/** 获取子节点 */
	void OnGetChildren(TSharedPtr<FUMGReflectorItem> InItem, TArray<TSharedPtr<FUMGReflectorItem>>& OutChildren);
	/** 选中变化回调 */
	void OnSelectionChanged(TSharedPtr<FUMGReflectorItem> InItem, ESelectInfo::Type SelectionType);
	/** 双击打开对应蓝图 */
	void OnItemDoubleClicked(TSharedPtr<FUMGReflectorItem> InItem);
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
	static FString GetUMGWidgetName(const UUserWidget* InWidget, const TSharedPtr<SWidget> InSlateWidget);
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

	// === 拾取功能 ===
	/** 设置拾取模式 */
	void SetPickingMode(bool bEnable);
	/** Pick按钮点击回调 */
	FReply OnPickButtonClicked();
	/** 是否在PIE中 */
	static bool IsInPIE();
	/** Tick中更新鼠标下方的UMG Widget */
	void UpdatePickingHover();
	/** 从FWidgetPath反向映射到树节点 */
	TSharedPtr<FUMGReflectorItem> FindTreeItemForWidgetPath(const FWidgetPath& InWidgetPath) const;
	/** 递归查找树中与指定SWidget对应的节点 */
	TSharedPtr<FUMGReflectorItem> FindTreeItemBySWidget(const TArray<TSharedPtr<FUMGReflectorItem>>& InItems, const TSharedRef<SWidget>& InSWidget) const;
	/** 通过几何包含关系查找光标下最深的UMG Widget（绕过HitTest） */
	static TSharedPtr<FUMGReflectorItem> FindDeepestItemUnderCursor(const TArray<TSharedPtr<FUMGReflectorItem>>& InItems, const FVector2D& AbsCursorPos);
	/** 选中并展开到目标节点 */
	void SelectAndExpandToItem(const TSharedPtr<FUMGReflectorItem>& InItem);
	/** 递归查找从根到目标节点的路径 */
	static bool FindPathToItem(
		const TArray<TSharedPtr<FUMGReflectorItem>>& InItems,
		const TSharedPtr<FUMGReflectorItem>& InTarget,
		TArray<TSharedPtr<FUMGReflectorItem>>& OutPath);
	/** 确认拾取 */
	void ConfirmPick();
	/** 取消拾取 */
	void CancelPick();
	/** 全局鼠标按下回调 */
	void OnGlobalMouseButtonDown(const FPointerEvent& MouseEvent);
	/** 全局键盘按下回调 */
	void OnGlobalKeyDown(const FKeyEvent& InKeyEvent);

#if WITH_SLATE_DEBUGGING
	/** 绘制高亮覆盖层 */
	void OnPaintDebugElements(const FPaintArgs& InArgs, const FGeometry& InAllottedGeometry, FSlateWindowElementList& InOutDrawElements, int32& InOutLayerId) const;
#endif

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

	// === 拾取状态 ===

	/** 是否处于拾取模式 */
	bool bPickingMode = false;

	/** 拾取按钮 */
	TSharedPtr<SButton> PickButton;

	/** 拾取状态提示文本 */
	TSharedPtr<STextBlock> PickingStatusText;

	/** 当前悬停的树节点 */
	TSharedPtr<FUMGReflectorItem> HoveredPickItem;

	/** 悬停Widget的几何信息（desktop space） */
	FGeometry HoveredWidgetGeometry;

	/** 悬停Widget所在的窗口 */
	TWeakPtr<SWindow> HoveredWidgetWindow;

	/** 是否有有效地悬停高亮目标 */
	bool bHasValidHoverTarget = false;

	/** 拾取模式下的树刷新计时器 */
	float PickingTreeRefreshTimer = 0.0f;

	/** 全局鼠标按下委托句柄 */
	FDelegateHandle GlobalMouseDownHandle;

	/** 全局键盘按下委托句柄 */
	FDelegateHandle GlobalKeyDownHandle;

#if WITH_SLATE_DEBUGGING
	/** PaintDebugElements委托句柄 */
	FDelegateHandle PaintDebugElementsHandle;
#endif
};
