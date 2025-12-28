#pragma once


class UWidget;
class UWidgetTree;

//class FUMGReflectorItem : public SMultiColumnTableRow<TSharedRef<FUMGReflectorItem>>
class FUMGReflectorItem : public TSharedFromThis<FUMGReflectorItem>
{
public:
	FUMGReflectorItem(const UWidget* InWidget, const FString& InDisplayName);
	void AddChildNodeItem(const TSharedPtr<FUMGReflectorItem>& Item);
	static TSharedRef<SWidget> GenerateTableRowForItem(const TSharedPtr<FUMGReflectorItem>& InItem);

	FORCEINLINE FString GetDisplayName()const { return DisplayName; }
	FORCEINLINE FString GetTypeName() const { return TypeName; }
	FORCEINLINE FString GetWidgetName() const { return WidgetName; }
	FORCEINLINE bool HasChildren() const { return ChildrenItems.Num() > 0; }
	FORCEINLINE const TArray<TSharedPtr<FUMGReflectorItem>>& GetChildrenData() const { return ChildrenItems; }
	FORCEINLINE TWeakObjectPtr<const UWidget> GetWidget()const { return Widget; }

	//virtual TSharedRef<SWidget> GenerateWidgetForColumn( const FName& ColumnName ) override;

private:
	TWeakObjectPtr<const UWidget> Widget;
	FString DisplayName;
	FString TypeName;
	FString WidgetName;
	TArray<TSharedPtr<FUMGReflectorItem>> ChildrenItems;

};
