// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "UIListEntryBase.generated.h"

/**
 * Base row widget for UUIListBase. Implements IUserObjectListEntry so UListView can
 * recycle it during virtualization. Bind data in the OnItemSet event / override.
 * See Docs/DESIGN.md sections 6-7.
 *
 * WBP pairing: reparent WBP_ListEntry to this class; the list's EntryWidgetClass
 * points at that WBP.
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUIListEntryBase : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	/** The data object this row currently represents (valid after it is assigned). */
	UFUNCTION(BlueprintCallable, Category = "UI|List")
	UObject* GetItem() const { return Item; }

protected:
	// IUserObjectListEntry
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;

	/** Fired when this recycled row is (re)assigned a data object. Bind visuals here. */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|List")
	void OnItemSet(UObject* InItem);

	/** Currently bound data object. */
	UPROPERTY(BlueprintReadOnly, Category = "UI|List")
	TObjectPtr<UObject> Item;
};
