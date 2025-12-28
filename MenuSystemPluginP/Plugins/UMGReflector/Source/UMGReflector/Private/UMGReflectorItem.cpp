#include "UMGReflectorItem.h"
#include "Components/Widget.h"

FUMGReflectorItem::FUMGReflectorItem(const UWidget* InWidget, const FString& InDisplayName)
	: Widget(InWidget)
	  , DisplayName(InDisplayName)
{
	if (Widget.IsValid())
	{
		if (const TSharedPtr<SWidget> S_Widget = InWidget->GetCachedWidget())
		{
			TypeName = S_Widget->GetTypeAsString();
		}
		WidgetName = Widget.Get()->GetName();
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
		+ SHorizontalBox::Slot()
		.FillWidth(0.6f)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->GetDisplayName()))
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.4f)
		.Padding(2.0f)
		[
			SNew(STextBlock)
			.Text(FText::FromString(InItem->GetTypeName()))
			.ColorAndOpacity(FLinearColor::Gray)
		];
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
