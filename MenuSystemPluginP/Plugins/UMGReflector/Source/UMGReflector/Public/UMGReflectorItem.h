#pragma once


class UWidget;
class UWidgetTree;

//class FUMGReflectorItem : public SMultiColumnTableRow<TSharedRef<FUMGReflectorItem>>
class FUMGReflectorItem : public TSharedFromThis<FUMGReflectorItem>
{
public:
	/**
	 * 构造函数
	 * @param InWidget UWidget指针
	 * @param InDisplayName 显示名称
	 */
	FUMGReflectorItem(const UWidget* InWidget, const FString& InDisplayName);
	/** 添加子节点 */
	void AddChildNodeItem(const TSharedPtr<FUMGReflectorItem>& Item);
	/** 生成表格行Widget */
	static TSharedRef<SWidget> GenerateTableRowForItem(const TSharedPtr<FUMGReflectorItem>& InItem);

	FString GetVisibilityText() const;
	FString GetPositionText() const;
	FString GetSizeText() const;
	bool IsWidgetEnabled() const;
	int32 GetZOrder() const;
	bool MatchesSearchText(const FString& SearchText) const;

	// === Getter函数 ===
	
	FORCEINLINE FString GetDisplayName()const { return DisplayName; }
	FORCEINLINE FString GetTypeName() const { return TypeName; }
	FORCEINLINE FString GetWidgetName() const { return WidgetName; }
	FORCEINLINE bool HasChildren() const { return ChildrenItems.Num() > 0; }
	FORCEINLINE const TArray<TSharedPtr<FUMGReflectorItem>>& GetChildrenData() const { return ChildrenItems; }
	FORCEINLINE TWeakObjectPtr<const UWidget> GetWidget()const { return Widget; }

	//virtual TSharedRef<SWidget> GenerateWidgetForColumn( const FName& ColumnName ) override;
private:
	/** 缓存Widget信息 */
	void CacheWidgetInfo();
	
private:
	/** Widget弱引用 */
	TWeakObjectPtr<const UWidget> Widget;
	
	/** 显示名称 */
	FString DisplayName;
	
	/** Slate类型名称（如SButton, STextBlock） */
	FString TypeName;
	
	/** UMG Widget名称 */
	FString WidgetName;
	
	/** 子节点数组 */
	TArray<TSharedPtr<FUMGReflectorItem>> ChildrenItems;

	// === 缓存的扩展信息 ===
	
	/** 可见性状态 */
	FString CachedVisibility;
	
	/** 位置信息 */
	FVector2D CachedPosition;
	
	/** 大小信息 */
	FVector2D CachedSize;
	
	/** 是否启用 */
	bool bCachedIsEnabled;
	
	/** Z-Order */
	int32 CachedZOrder;
	
	/** 缓存是否有效 */
	bool bCacheValid;

};
