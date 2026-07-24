// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UILayerTypes.h"
#include "UICoverageConfig.generated.h"

/** One coverage rule: when CoveringLayer has a visible frame above, CoveredLayer hides. */
USTRUCT(BlueprintType)
struct FUICoverageRule
{
	GENERATED_BODY()

	/** The layer that gets hidden. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	EUILayer CoveredLayer = EUILayer::Dock;

	/** The layer whose visible frame (higher in the stack) causes the hide. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	EUILayer CoveringLayer = EUILayer::FullWindow;
};

/**
 * Data-driven coverage matrix: which layer hides which. Replaces the per-UI occlusion
 * flag with a central, editable table (modeled on UIStack's UIStackFrameTypeCoveredableCfg).
 * A frame is hidden when some visible frame above it belongs to a layer configured to
 * cover this frame's layer. See Docs/DESIGN.md section 4.
 *
 * Example rules:
 *   Covered = Dock,       Covering = FullWindow  -> a full window hides dock UI
 *   Covered = FullWindow, Covering = PopupWindow -> a popup hides the full window
 *   (no rule for PopupWindow covered by Notification -> both remain visible)
 */
UCLASS(BlueprintType)
class UIFRAMEWORKWIDGETS_API UUICoverageConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TArray<FUICoverageRule> Rules;

	/** True if CoveringLayer is configured to cover CoveredLayer. */
	bool CanCover(EUILayer CoveringLayer, EUILayer CoveredLayer) const
	{
		for (const FUICoverageRule& Rule : Rules)
		{
			if (Rule.CoveringLayer == CoveringLayer && Rule.CoveredLayer == CoveredLayer)
			{
				return true;
			}
		}
		return false;
	}

	/** Validate rules for self-coverage and duplicate entries. */
	void ValidateRules(TArray<FString>& OutErrors) const
	{
		TSet<uint16> Seen;
		for (const FUICoverageRule& Rule : Rules)
		{
			if (!IsValidUILayer(Rule.CoveredLayer) || !IsValidUILayer(Rule.CoveringLayer))
			{
				OutErrors.Add(TEXT("CoverageConfig contains an invalid Layer."));
				continue;
			}
			if (Rule.CoveredLayer == Rule.CoveringLayer)
			{
				OutErrors.Add(FString::Printf(TEXT("CoverageConfig contains self-coverage for Layer %d."),
					static_cast<int32>(Rule.CoveredLayer)));
			}
			const uint16 Pair = (static_cast<uint16>(Rule.CoveringLayer) << 8)
				| static_cast<uint16>(Rule.CoveredLayer);
			if (Seen.Contains(Pair))
			{
				OutErrors.Add(FString::Printf(TEXT("CoverageConfig contains a duplicate rule for Layers %d -> %d."),
					static_cast<int32>(Rule.CoveringLayer), static_cast<int32>(Rule.CoveredLayer)));
			}
			else
			{
				Seen.Add(Pair);
			}
		}
	}
};
