#include "AssetCookRuleManager.h"

#include "Settings/ProjectPackagingSettings.h"

namespace
{
	const TCHAR* GGameContentRoot = TEXT("/Game/");

	/**
	 * Normalize a stored Path to content-relative form. Tolerates older/buggy
	 * absolute entries ("/Game/Blueprint") alongside the canonical relative
	 * form ("Blueprint"), so both compare and clean up consistently.
	 */
	FString ToRelativeForm(const FString& StoredPath)
	{
		if (StoredPath == TEXT("/Game"))
		{
			return FString();
		}
		if (StoredPath.StartsWith(GGameContentRoot))
		{
			return StoredPath.RightChop(FCString::Strlen(GGameContentRoot));
		}
		return StoredPath;
	}

	/** Remove every Always/Never entry that resolves to Relative (either form). */
	void RemoveRelative(TArray<FDirectoryPath>& Dirs, const FString& Relative)
	{
		Dirs.RemoveAll([&Relative](const FDirectoryPath& Dir)
		{
			return ToRelativeForm(Dir.Path) == Relative;
		});
	}

	bool ContainsRelative(const TArray<FDirectoryPath>& Dirs, const FString& Relative)
	{
		return Dirs.ContainsByPredicate([&Relative](const FDirectoryPath& Dir)
		{
			return ToRelativeForm(Dir.Path) == Relative;
		});
	}
}

UProjectPackagingSettings* FAssetCookRuleManager::GetSettings()
{
	return GetMutableDefault<UProjectPackagingSettings>();
}

bool FAssetCookRuleManager::ToContentRelative(const FString& PackagePath, FString& OutRelative)
{
	FString Normalized = PackagePath;
	if (!Normalized.StartsWith(GGameContentRoot))
	{
		// Allow the bare root "/Game".
		if (Normalized == TEXT("/Game"))
		{
			OutRelative.Empty();
			return true;
		}
		return false;
	}

	OutRelative = Normalized.RightChop(FCString::Strlen(GGameContentRoot));
	return true;
}

FString FAssetCookRuleManager::ToPackagePath(const FString& ContentRelative)
{
	if (ContentRelative.IsEmpty())
	{
		return TEXT("/Game");
	}
	// Already a full package path (e.g. /Interchange/..., /Engine/...): keep as-is.
	if (ContentRelative.StartsWith(TEXT("/")))
	{
		return ContentRelative;
	}
	return FString(GGameContentRoot) + ContentRelative;
}

ECookRuleType FAssetCookRuleManager::GetRule(const FString& PackagePath)
{
	FString Relative;
	if (!ToContentRelative(PackagePath, Relative))
	{
		return ECookRuleType::Default;
	}

	const UProjectPackagingSettings* Settings = GetSettings();
	if (ContainsRelative(Settings->DirectoriesToNeverCook, Relative))
	{
		return ECookRuleType::NeverCook;
	}
	if (ContainsRelative(Settings->DirectoriesToAlwaysCook, Relative))
	{
		return ECookRuleType::AlwaysCook;
	}
	return ECookRuleType::Default;
}

FResolvedCookRule FAssetCookRuleManager::ResolveRule(const FString& PackagePath)
{
	FResolvedCookRule Result;

	TMap<FString, ECookRuleType> Ruled;
	GetAllRuledDirectories(Ruled);
	if (Ruled.IsEmpty())
	{
		return Result;
	}

	// Walk from the directory itself up toward /Game, taking the first ruled hit.
	FString Current = PackagePath;
	while (!Current.IsEmpty())
	{
		if (const ECookRuleType* Found = Ruled.Find(Current))
		{
			Result.Rule = *Found;
			Result.bInherited = (Current != PackagePath);
			Result.SourceDir = Current;
			return Result;
		}

		if (Current == TEXT("/Game"))
		{
			break;
		}

		FString Parent;
		FString Leaf;
		if (Current.Split(TEXT("/"), &Parent, &Leaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd)
			&& !Parent.IsEmpty())
		{
			Current = Parent;
		}
		else
		{
			break;
		}
	}

	return Result; // Default — no rule on this directory or any ancestor.
}

void FAssetCookRuleManager::GetAllRuledDirectories(TMap<FString, ECookRuleType>& OutRules)
{
	OutRules.Reset();
	const UProjectPackagingSettings* Settings = GetSettings();

	for (const FDirectoryPath& Dir : Settings->DirectoriesToAlwaysCook)
	{
		OutRules.Add(ToPackagePath(Dir.Path), ECookRuleType::AlwaysCook);
	}
	for (const FDirectoryPath& Dir : Settings->DirectoriesToNeverCook)
	{
		// NeverCook wins if a directory somehow appears in both lists.
		OutRules.Add(ToPackagePath(Dir.Path), ECookRuleType::NeverCook);
	}
}

void FAssetCookRuleManager::GetRedundantDirectories(TArray<FString>& OutDirs)
{
	OutDirs.Reset();

	TMap<FString, ECookRuleType> Ruled;
	GetAllRuledDirectories(Ruled);

	for (const TPair<FString, ECookRuleType>& Pair : Ruled)
	{
		FString Parent;
		FString Leaf;
		if (!Pair.Key.Split(TEXT("/"), &Parent, &Leaf, ESearchCase::IgnoreCase, ESearchDir::FromEnd)
			|| Parent.IsEmpty())
		{
			continue; // Top-level (e.g. "/Game") has no parent to inherit from.
		}

		// Rule this directory would inherit if its own entry were removed.
		const FResolvedCookRule Inherited = ResolveRule(Parent);
		if (Inherited.Rule != ECookRuleType::Default && Inherited.Rule == Pair.Value)
		{
			OutDirs.Add(Pair.Key);
		}
	}
}


void FAssetCookRuleManager::ApplyRules(const TMap<FString, ECookRuleType>& Rules)
{
	if (Rules.IsEmpty())
	{
		return;
	}

	UProjectPackagingSettings* Settings = GetSettings();

	for (const TPair<FString, ECookRuleType>& Pair : Rules)
	{
		FString Relative;
		if (!ToContentRelative(Pair.Key, Relative))
		{
			continue;
		}

		// Clear any prior rule for this directory first, then re-add as needed.
		RemoveRelative(Settings->DirectoriesToAlwaysCook, Relative);
		RemoveRelative(Settings->DirectoriesToNeverCook, Relative);

		switch (Pair.Value)
		{
		case ECookRuleType::AlwaysCook:
		{
			FDirectoryPath Dir;
			Dir.Path = Relative;
			Settings->DirectoriesToAlwaysCook.Add(Dir);
			break;
		}
		case ECookRuleType::NeverCook:
		{
			FDirectoryPath Dir;
			Dir.Path = Relative;
			Settings->DirectoriesToNeverCook.Add(Dir);
			break;
		}
		case ECookRuleType::Default:
		default:
			// Already removed above.
			break;
		}
	}

	Settings->TryUpdateDefaultConfigFile();
}
