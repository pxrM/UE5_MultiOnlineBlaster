// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UIListBase.generated.h"

class UListView;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUIListItemClicked, UObject*, Item);

/**
 * Virtualized list control wrapping a UListView, so only visible rows exist as
 * widgets no matter how many items you feed it. Rows must derive from
 * UUIListEntryBase. See Docs/DESIGN.md sections 6-7 (performance).
 *
 * WBP pairing: reparent WBP_List to this class, name the inner UListView "ListView",
 * and set its EntryWidgetClass to your WBP_ListEntry.
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUIListBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Fired when a row is clicked, passing the row's data object. */
	UPROPERTY(BlueprintAssignable, Category = "UI|List")
	FUIListItemClicked OnItemClicked;

	/** Replace all items. */
	UFUNCTION(BlueprintCallable, Category = "UI|List")
	void SetItems(const TArray<UObject*>& InItems);

	/** Append one item. */
	UFUNCTION(BlueprintCallable, Category = "UI|List")
	void AddItem(UObject* InItem);

	/** Remove all items. */
	UFUNCTION(BlueprintCallable, Category = "UI|List")
	void ClearItems();

	/** Currently selected data object, or nullptr. */
	UFUNCTION(BlueprintCallable, Category = "UI|List")
	UObject* GetSelectedItem() const;

protected:
	virtual void NativeOnInitialized() override;

	/** Native handler bound to the inner list view's click delegate. */
	void HandleItemClicked(UObject* ClickedItem);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "UI|List")
	TObjectPtr<UListView> ListView;
};
