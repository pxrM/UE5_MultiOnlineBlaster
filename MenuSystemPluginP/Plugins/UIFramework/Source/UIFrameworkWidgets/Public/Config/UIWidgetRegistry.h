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
	EUILayer Layer = EUILayer::FullWindow;

	/** If false, opening an already-open key is a no-op instead of pushing another instance. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	bool bAllowMultiple = false;

	/** Whether this entry participates in input activation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	bool bBlocksInput = true;

	/** Whether this entry is eligible to consume the framework Back action. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	bool bHandlesBack = true;

	/** Maximum simultaneous instances when bAllowMultiple is enabled. 0 = unlimited. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (ClampMin = "0"))
	int32 MaxOpenInstances = 0;

	/** Maximum closed instances retained for this key. 0 = unlimited. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (ClampMin = "0"))
	int32 MaxCachedInstances = 8;

	/** Class / instance caching. Frequently toggled UI benefits from a Keep* policy. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	EUICachePolicy CachePolicy = EUICachePolicy::Transient;

	/** Idle seconds before a KeepUntilIdle instance is destroyed. Ignored by other policies. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (EditCondition = "CachePolicy == EUICachePolicy::KeepUntilIdle", ClampMin = "0.0"))
	float IdleTimeoutSeconds = 60.f;

	/**
	 * Optional script module bound to the instance at construction, e.g. "UI.MainMenu"
	 * for Content/Script/UI/MainMenu.lua. Empty means no script binding.
	 *
	 * Requires a script adapter plugin (see IUIScriptClassBinder). Because script
	 * runtimes bind per UClass, two entries must not map the same WidgetClass to
	 * different modules. Leave empty when the widget class already declares its own
	 * module statically.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Scripting")
	FString ScriptModule;
};

/** Validate one entry from either the asset registry or a runtime script overlay. */
inline void ValidateUIWidgetEntry(
	const FGameplayTag& Key,
	const FUIWidgetEntry& Entry,
	TArray<FString>& OutErrors)
{
	if (!Key.IsValid())
	{
		OutErrors.Add(TEXT("Registry contains an invalid GameplayTag key."));
	}
	if (Entry.WidgetClass.IsNull())
	{
		OutErrors.Add(FString::Printf(TEXT("Registry key '%s' has no WidgetClass."), *Key.ToString()));
	}
	if (!IsValidUILayer(Entry.Layer))
	{
		OutErrors.Add(FString::Printf(TEXT("Registry key '%s' uses invalid Layer %d."),
			*Key.ToString(), static_cast<int32>(Entry.Layer)));
	}
	if (Entry.MaxOpenInstances < 0 || Entry.MaxCachedInstances < 0)
	{
		OutErrors.Add(FString::Printf(TEXT("Registry key '%s' has a negative instance limit."), *Key.ToString()));
	}
	if (!Entry.bAllowMultiple && Entry.MaxOpenInstances > 0)
	{
		OutErrors.Add(FString::Printf(TEXT("Registry key '%s' sets MaxOpenInstances but disallows multiple instances."), *Key.ToString()));
	}
	// A zero TTL means the reaper frees the instance on its next pass, so the entry
	// pays the cost of instance caching and gets none of the benefit. Almost always a
	// forgotten field rather than an intent.
	if (Entry.CachePolicy == EUICachePolicy::KeepUntilIdle && Entry.IdleTimeoutSeconds <= 0.f)
	{
		OutErrors.Add(FString::Printf(
			TEXT("Registry key '%s' uses KeepUntilIdle with IdleTimeoutSeconds %.2f; ")
			TEXT("a non-positive timeout discards the instance immediately. Use Transient instead."),
			*Key.ToString(), Entry.IdleTimeoutSeconds));
	}
	if (!Entry.ScriptModule.IsEmpty())
	{
		// Script module paths are dot-separated and suffix-free; the runtime resolves
		// them against its own script root. Catch the two mistakes people actually make.
		if (Entry.ScriptModule.EndsWith(TEXT(".lua"), ESearchCase::IgnoreCase))
		{
			OutErrors.Add(FString::Printf(
				TEXT("Registry key '%s' ScriptModule '%s' must not include the .lua suffix."),
				*Key.ToString(), *Entry.ScriptModule));
		}
		if (Entry.ScriptModule.Contains(TEXT("/")) || Entry.ScriptModule.Contains(TEXT("\\")))
		{
			OutErrors.Add(FString::Printf(
				TEXT("Registry key '%s' ScriptModule '%s' must use dots, not path separators."),
				*Key.ToString(), *Entry.ScriptModule));
		}
		// Binding happens on the constructed widget, so a module with nothing to bind
		// to would be dropped without ever being loaded.
		if (Entry.WidgetClass.IsNull())
		{
			OutErrors.Add(FString::Printf(
				TEXT("Registry key '%s' sets ScriptModule '%s' but has no WidgetClass to bind it to."),
				*Key.ToString(), *Entry.ScriptModule));
		}
	}
}

/**
 * Script runtimes bind a module to a UClass, not to an instance, so the same widget
 * class cannot serve two different modules. Collect that conflict across a whole set
 * of entries. Entries with an empty ScriptModule are ignored.
 */
inline void ValidateUIScriptModuleUniqueness(
	const TMap<FGameplayTag, FUIWidgetEntry>& Entries,
	TArray<FString>& OutErrors)
{
	TMap<FSoftObjectPath, TPair<FGameplayTag, FString>> ClaimedByClass;
	for (const TPair<FGameplayTag, FUIWidgetEntry>& Pair : Entries)
	{
		if (Pair.Value.ScriptModule.IsEmpty() || Pair.Value.WidgetClass.IsNull())
		{
			continue;
		}
		const FSoftObjectPath ClassPath = Pair.Value.WidgetClass.ToSoftObjectPath();
		if (const TPair<FGameplayTag, FString>* Claim = ClaimedByClass.Find(ClassPath))
		{
			if (Claim->Value != Pair.Value.ScriptModule)
			{
				OutErrors.Add(FString::Printf(
					TEXT("Registry keys '%s' and '%s' both use WidgetClass '%s' but request different ")
					TEXT("ScriptModules ('%s' vs '%s'). Script binding is per class, so only one can win."),
					*Claim->Key.ToString(), *Pair.Key.ToString(), *ClassPath.ToString(),
					*Claim->Value, *Pair.Value.ScriptModule));
			}
			continue;
		}
		ClaimedByClass.Add(ClassPath, TPair<FGameplayTag, FString>(Pair.Key, Pair.Value.ScriptModule));
	}
}

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

	/**
	 * Validate configuration without loading any WidgetClass assets.
	 * ScriptModule uniqueness spans the DataAsset and the runtime overlay together, so
	 * it is checked by the manager (see ValidateUIScriptModuleUniqueness), not here.
	 */
	void ValidateEntries(TArray<FString>& OutErrors) const
	{
		for (const TPair<FGameplayTag, FUIWidgetEntry>& Pair : Entries)
		{
			ValidateUIWidgetEntry(Pair.Key, Pair.Value, OutErrors);
		}
	}
};
