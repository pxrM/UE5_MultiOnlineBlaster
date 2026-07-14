// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UILayerTypes.h"
#include "UIWidgetRegistry.generated.h"

class UUserWidget;

/** How a UI's class / instance is cached across open-close cycles, and when it's freed. */
UENUM(BlueprintType)
enum class EUICachePolicy : uint8
{
	/** Class unloaded when unused; instance destroyed on close. Lowest memory. */
	Transient				UMETA(DisplayName = "Transient"),
	/** Class kept resident (no re-load from disk); instance still rebuilt each open. */
	CacheClass				UMETA(DisplayName = "Cache Class"),
	/** Instance kept alive after close, destroyed once idle for IdleTimeoutSeconds. */
	KeepUntilIdle			UMETA(DisplayName = "Keep Until Idle (TTL)"),
	/** Instance kept alive across opens, destroyed on scene / level change. */
	KeepUntilSceneChange	UMETA(DisplayName = "Keep Until Scene Change"),
	/** Instance kept alive for the whole session, survives scene changes. Highest memory. */
	KeepPersistent			UMETA(DisplayName = "Keep Persistent")
};

/**
 * One registry entry: which widget to spawn, which layer it goes on, duplicate rule
 * and cache policy. WidgetClass is a soft reference so nothing loads until the UI is
 * actually opened.
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

	/** Class / instance caching. Frequently toggled UI benefits from a Keep* policy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	EUICachePolicy CachePolicy = EUICachePolicy::Transient;

	/** Idle seconds before a KeepUntilIdle instance is destroyed. Ignored by other policies. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (EditCondition = "CachePolicy == EUICachePolicy::KeepUntilIdle", ClampMin = "0.0"))
	float IdleTimeoutSeconds = 60.f;
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
