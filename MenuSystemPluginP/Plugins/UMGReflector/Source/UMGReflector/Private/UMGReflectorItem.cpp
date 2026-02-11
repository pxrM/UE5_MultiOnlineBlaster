#include "UMGReflectorItem.h"

#include "Components/PanelWidget.h"
#include "Components/Widget.h"

FUMGReflectorItem::FUMGReflectorItem(const UWidget* InWidget, const FString& InDisplayName)
	: Widget(InWidget)
	  , DisplayName(InDisplayName)
{
	if (Widget.IsValid())
	{
		// 获取Slate类型名称
		if (const TSharedPtr<SWidget> S_Widget = InWidget->GetCachedWidget())
		{
			TypeName = S_Widget->GetTypeAsString();
		}
		// 获取Widget名称
		WidgetName = Widget.Get()->GetName();

		CacheWidgetInfo();
	}
}

void FUMGReflectorItem::AddChildNodeItem(const TSharedPtr<FUMGReflectorItem>& Item)
{
	if (Item.IsValid())
	{
		ChildrenItems.Add(Item);
	}
}

TSharedRef<SWidget> FUMGReflectorItem::GenerateTableRowForItem(const TSharedPtr<FUMGReflectorItem>& InItem)
{
	if (!InItem.IsValid()) return SNew(STextBlock).Text(FText::FromString("Invalid Item"));

	return SNew(SHorizontalBox)

		// Widget名称列 (35%)
		+SHorizontalBox::Slot()
		.FillWidth(0.35f)
		.Padding(2.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->GetDisplayName()))
			.ToolTipText(FText::FromString(
				FString::Printf(TEXT("Full Name: %s"), *InItem->GetWidgetName())
			))
		]

		// 类型列 (25%)
		+SHorizontalBox::Slot()
		.FillWidth(0.25f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->GetTypeName()))
			.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f))
			.ToolTipText(FText::FromString(
				FString::Printf(TEXT("Slate Type: %s"), *InItem->GetTypeName())
			))
		]

		// 可见性列 (15%)
		+SHorizontalBox::Slot()
		.FillWidth(0.15f)
		.Padding(2.f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->GetVisibilityText()))
			.ColorAndOpacity(FLinearColor(0.5f, 0.8f, 0.5f))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
		]

		// 位置列 (15%)
		+ SHorizontalBox::Slot()
		.FillWidth(0.15f)
		.Padding(2.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->GetPositionText()))
			.ColorAndOpacity(FLinearColor(0.6f, 0.6f, 0.8f))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
		]
		
		// 大小列 (10%)
		+ SHorizontalBox::Slot()
		.FillWidth(0.1f)
		.Padding(2.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->GetSizeText()))
			.ColorAndOpacity(FLinearColor(0.8f, 0.6f, 0.6f))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 8))
		];
}

FString FUMGReflectorItem::GetVisibilityText() const
{
	if (!Widget.IsValid())
	{
		return TEXT("N/A");
	}

	// 简化可见性文本显示
	FString FullVisibility = CachedVisibility;
	
	if (FullVisibility.Contains(TEXT("Visible")))
	{
		return TEXT("Visible");
	}
	else if (FullVisibility.Contains(TEXT("Collapsed")))
	{
		return TEXT("Collapsed");
	}
	else if (FullVisibility.Contains(TEXT("Hidden")))
	{
		return TEXT("Hidden");
	}
	else if (FullVisibility.Contains(TEXT("HitTestInvisible")))
	{
		return TEXT("HitTest-");
	}
	else if (FullVisibility.Contains(TEXT("SelfHitTestInvisible")))
	{
		return TEXT("SelfHT-");
	}
	
	return FullVisibility;
}

FString FUMGReflectorItem::GetPositionText() const
{
	if (!Widget.IsValid() || !bCacheValid)
	{
		return TEXT("N/A");
	}
	return  FString::Printf(TEXT("%.0f, %.0f"), CachedPosition.X, CachedPosition.Y);
}

FString FUMGReflectorItem::GetSizeText() const
{
	if (!Widget.IsValid() || !bCacheValid)
	{
		return TEXT("N/A");
	}
	return FString::Printf(TEXT("%.0f, %.0f"), CachedSize.X, CachedSize.Y);
}

bool FUMGReflectorItem::IsWidgetEnabled() const
{
	if (!Widget.IsValid())
	{
		return false;
	}

	return bCachedIsEnabled;
}

int32 FUMGReflectorItem::GetZOrder() const
{
	return CachedZOrder;
}

bool FUMGReflectorItem::MatchesSearchText(const FString& SearchText) const
{
	if (SearchText.IsEmpty())
	{
		return true;
	}

	// 搜索显示名称
	if (DisplayName.Contains(SearchText, ESearchCase::IgnoreCase))
	{
		return true;
	}

	// 搜索类型名称
	if (TypeName.Contains(SearchText, ESearchCase::IgnoreCase))
	{
		return true;
	}

	// 搜索Widget名称
	if (WidgetName.Contains(SearchText, ESearchCase::IgnoreCase))
	{
		return true;
	}

	// 递归搜索子项
	for (const TSharedPtr<FUMGReflectorItem>& Child : ChildrenItems)
	{
		if (Child.IsValid() && Child->MatchesSearchText(SearchText))
		{
			return true;
		}
	}
	
	return false;
}

void FUMGReflectorItem::CacheWidgetInfo()
{
	if (!Widget.IsValid())
	{
		bCacheValid = false;
		return;
	}

	const UWidget* WidgetPtr = Widget.Get();

	// 缓存可见性
	CachedVisibility = StaticEnum<ESlateVisibility>()->GetNameStringByValue(static_cast<int32>(Widget->GetVisibility()));
	// 缓存是否启用
	bCachedIsEnabled = WidgetPtr->GetIsEnabled();
	// 尝试获取位置和大小信息
	if (auto SlateWidget = WidgetPtr->GetCachedWidget())
	{
		FGeometry Geometry = SlateWidget->GetCachedGeometry();
		CachedPosition = Geometry.GetAbsolutePosition();
		CachedSize = Geometry.GetAbsoluteSize();
	}
	// 获取Z-Order（如果是PanelWidget的子项）
	if (WidgetPtr->GetParent())
	{
		CachedZOrder = WidgetPtr->GetParent()->GetChildIndex(WidgetPtr);
	}

	bCacheValid = true;
}

// TSharedRef<SWidget> FUMGReflectorItem::GenerateWidgetForColumn(const FName& ColumnName)
// {
// 	return SNew(SHorizontalBox)
// 		+ SHorizontalBox::Slot()
// 		.FillWidth(0.6f)
// 		.Padding(2.0f)
// 		[
// 			SNew(STextBlock)
// 			.Text(FText::FromString(GetDisplayName()))
// 		]
// 		+ SHorizontalBox::Slot()
// 		.FillWidth(0.4f)
// 		.Padding(2.0f)
// 		[
// 			SNew(STextBlock)
// 			.Text(FText::FromString(GetTypeName()))
// 			.ColorAndOpacity(FLinearColor::Gray)
// 		];
// }
