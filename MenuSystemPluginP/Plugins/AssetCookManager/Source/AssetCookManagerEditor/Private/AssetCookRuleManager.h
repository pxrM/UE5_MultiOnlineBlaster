#pragma once

#include "CoreMinimal.h"

class UProjectPackagingSettings;

/** How a content directory is treated by the cooker. */
enum class ECookRuleType : uint8
{
	/** No explicit rule — included only if referenced from the cook root. */
	Default,
	/** Forced into the package even if unreferenced. */
	AlwaysCook,
	/** Excluded from the package even if referenced. */
	NeverCook,
};

/**
 * The rule that actually governs a directory once parent inheritance is taken
 * into account. The cooker treats Always/Never recursively, so a directory with
 * no explicit rule is still governed by the nearest ancestor that has one.
 */
struct FResolvedCookRule
{
	/** Effective rule after walking up to the nearest ruled ancestor. */
	ECookRuleType Rule = ECookRuleType::Default;
	/** True when Rule comes from an ancestor rather than this exact directory. */
	bool bInherited = false;
	/** Directory the rule was found on (this dir, or the ancestor when inherited). */
	FString SourceDir;
};

/**
 * Reads and writes the per-directory cook rules stored in
 * UProjectPackagingSettings (DirectoriesToAlwaysCook / DirectoriesToNeverCook),
 * persisted to DefaultGame.ini.
 *
 * Directories are addressed by long package path ("/Game/UI"). The packaging
 * settings store them relative to the content dir ("UI"), so this class
 * converts between the two forms.
 */
class FAssetCookRuleManager
{
public:
	/** Current rule for an exact directory (no inheritance from parents). */
	static ECookRuleType GetRule(const FString& PackagePath);

	/**
	 * Effective rule for a directory, walking up to the nearest ruled ancestor
	 * (mirrors how the cooker applies Always/Never recursively). NeverCook wins
	 * when an ancestor chain carries conflicting rules at the same depth.
	 */
	static FResolvedCookRule ResolveRule(const FString& PackagePath);

	/**
	 * Apply a set of directory rules and save them to DefaultGame.ini.
	 * Only directories present in the map are touched; Default removes any
	 * existing Always/Never entry for that directory.
	 */
	static void ApplyRules(const TMap<FString, ECookRuleType>& Rules);

	/** All directories (long package paths) that currently carry a rule. */
	static void GetAllRuledDirectories(TMap<FString, ECookRuleType>& OutRules);

	/**
	 * Directories whose explicit rule is already implied by a parent directory's
	 * rule (same Always/Never), making the entry redundant. Removing them leaves
	 * the effective cook behaviour unchanged and keeps DefaultGame.ini lean.
	 */
	static void GetRedundantDirectories(TArray<FString>& OutDirs);

	/** "/Game/UI" -> "UI"; returns false if not under /Game. */
	static bool ToContentRelative(const FString& PackagePath, FString& OutRelative);

	/** "UI" -> "/Game/UI". */
	static FString ToPackagePath(const FString& ContentRelative);

private:
	static UProjectPackagingSettings* GetSettings();
};
