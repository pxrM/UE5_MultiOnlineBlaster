// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UILayerTypes.h"
#include "UIWidgetRegistry.generated.h"

class UUserWidget;

/**
 * One registry entry: which widget to spawn, which layer it goes on, and whether
 * duplicates are allowed. WidgetClass is a soft reference so nothing loads until
 * the UI is actually opened.
 */
USTRUCT(BlueprintType)
struct FUIWidgetEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftClassPtr<UUserWidget> WidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	EUILayer Layer = EUILayer::Menu;

	/** If false, opening an already-open key is a no-op instead of pushing another instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	bool bAllowMultiple = false;
};

/**
 * Maps a GameplayTag key to a widget entry, so callers open UI by key
 * (OpenUI(TAG_UI_Inventory)) instead of hardcoding a class + layer.
 * See Docs/DESIGN.md sections 3-4.
 */
UCLASS(BlueprintType)
class UIFRAMEWORKWIDGETS_API UUIWidgetRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Key -> widget entry. Keys are GameplayTags like UI.Menu, UI.Inventory, UI.Settings. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (ForceInlineRow))
	TMap<FGameplayTag, FUIWidgetEntry> Entries;

	/** Look up an entry; returns nullptr if the key is not registered. */
	const FUIWidgetEntry* FindEntry(const FGameplayTag& Key) const { return Entries.Find(Key); }
};
