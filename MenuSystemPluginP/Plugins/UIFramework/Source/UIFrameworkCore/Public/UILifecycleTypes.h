// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UILayerTypes.h"
#include "UILifecycleTypes.generated.h"

/** Why a managed widget is leaving the UI stack. */
UENUM(BlueprintType)
enum class EUIWidgetCloseReason : uint8
{
	Requested       UMETA(DisplayName = "Requested"),
	Back            UMETA(DisplayName = "Back"),
	LayerPop        UMETA(DisplayName = "Layer Pop"),
	LayerClear      UMETA(DisplayName = "Layer Clear"),
	RootTeardown    UMETA(DisplayName = "Root Teardown"),
	SceneChange     UMETA(DisplayName = "Scene Change"),
	SubsystemShutdown UMETA(DisplayName = "Subsystem Shutdown")
};

/** Immutable information supplied for one managed open cycle. */
USTRUCT(BlueprintType)
struct UIFRAMEWORKCORE_API FUIOpenContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "UI|Lifecycle")
	FGameplayTag Key;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Lifecycle")
	EUILayer Layer = EUILayer::FullWindow;

	/** Optional caller-owned request data. The manager keeps it alive for async opens. */
	UPROPERTY(BlueprintReadOnly, Category = "UI|Lifecycle")
	TObjectPtr<UObject> Payload = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Lifecycle")
	bool bRestoredFromCache = false;
};

/** Immutable information supplied for one managed close cycle. */
USTRUCT(BlueprintType)
struct UIFRAMEWORKCORE_API FUICloseContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "UI|Lifecycle")
	FGameplayTag Key;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Lifecycle")
	EUILayer Layer = EUILayer::FullWindow;

	UPROPERTY(BlueprintReadOnly, Category = "UI|Lifecycle")
	EUIWidgetCloseReason Reason = EUIWidgetCloseReason::Requested;

	/** Optional result returned by the closing widget/caller. */
	UPROPERTY(BlueprintReadOnly, Category = "UI|Lifecycle")
	TObjectPtr<UObject> Result = nullptr;

	/** True when the manager will retain this instance after OnUIClosed. */
	UPROPERTY(BlueprintReadOnly, Category = "UI|Lifecycle")
	bool bWillBeCached = false;
};
