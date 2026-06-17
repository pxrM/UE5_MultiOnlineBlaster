#pragma once

#include "CoreMinimal.h"

/** A node in the /Game content directory tree. */
struct FCookDirNode
{
	/** Long package path, e.g. "/Game/UI". */
	FString PackagePath;
	/** Leaf folder name for display, e.g. "UI". */
	FString DisplayName;
	TArray<TSharedPtr<FCookDirNode>> Children;
};

/** One "packaged asset references a NeverCook asset" finding. */
struct FCookViolation
{
	/** Asset that pulls the excluded one into the package. */
	FString SourcePackage;
	/** The excluded (NeverCook) asset being referenced. */
	FString ReferencedPackage;
	/** NeverCook directory the referenced asset lives under. */
	FString NeverCookDir;
	/** True for a hard reference (breaks cook); false for a soft reference (runtime risk). */
	bool bHardRef = true;
};

/**
 * Progress callback for a validation pass. Invoked once per source package with
 * (processed so far, total). Return true to request cancellation — the scan then
 * stops and returns the violations found up to that point.
 */
using FCookScanProgress = TFunctionRef<bool(int32 Processed, int32 Total)>;

/** AssetRegistry-backed queries for the cook manager. */
class FAssetCookScanner
{
public:
	/** Build the /Game directory tree (recursive). Returns root-level nodes. */
	static TArray<TSharedPtr<FCookDirNode>> BuildContentTree();

	/**
	 * Find assets that would be cooked but reference an asset living under a
	 * NeverCook directory. Hard references break the cook or ship a broken
	 * reference; soft references survive the cook but risk a null at runtime
	 * (flagged with bHardRef=false). Scans every project content root, not just
	 * /Game. Returns empty if no NeverCook rules exist.
	 *
	 * Progress is reported per source package via Progress; returning true from
	 * it cancels the scan and yields the partial result.
	 */
	static TArray<FCookViolation> ValidateNeverCookReferences(FCookScanProgress Progress);
};
