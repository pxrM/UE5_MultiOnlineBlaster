#include "AssetCookScanner.h"

#include "AssetCookRuleManager.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Misc/AssetRegistryInterface.h"
#include "Misc/PackageName.h"

namespace
{
	IAssetRegistry& GetAssetRegistry()
	{
		FAssetRegistryModule& Module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		return Module.Get();
	}

	/** True if PackageName sits inside (or is) directory Dir. */
	bool IsUnderDir(const FString& PackageName, const FString& Dir)
	{
		if (PackageName == Dir)
		{
			return true;
		}
		return PackageName.StartsWith(Dir + TEXT("/"));
	}

	/** Returns the effective NeverCook dir for PackageName, or empty. */
	FString ResolveNeverCookDir(const FString& PackageName)
	{
		const FResolvedCookRule Rule = FAssetCookRuleManager::ResolveRule(PackageName);
		return Rule.Rule == ECookRuleType::NeverCook ? Rule.SourceDir : FString();
	}
}

TArray<TSharedPtr<FCookDirNode>> FAssetCookScanner::BuildContentTree()
{
	IAssetRegistry& Registry = GetAssetRegistry();

	TArray<FString> SubPaths;
	Registry.GetSubPaths(TEXT("/Game"), SubPaths, /*bInRecurse=*/true);

	// Sort so parents are created before children.
	SubPaths.Sort();

	// PackagePath -> node, for O(1) parent lookup while building.
	TMap<FString, TSharedPtr<FCookDirNode>> NodeByPath;
	TArray<TSharedPtr<FCookDirNode>> Roots;

	for (const FString& Path : SubPaths)
	{
		TSharedPtr<FCookDirNode> Node = MakeShared<FCookDirNode>();
		Node->PackagePath = Path;

		FString ParentPath;
		FString LeafName;
		if (Path.Split(TEXT("/"), &ParentPath, &LeafName, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
		{
			Node->DisplayName = LeafName;
		}
		else
		{
			Node->DisplayName = Path;
		}

		NodeByPath.Add(Path, Node);

		if (const TSharedPtr<FCookDirNode>* Parent = NodeByPath.Find(ParentPath))
		{
			(*Parent)->Children.Add(Node);
		}
		else
		{
			// Parent is "/Game" (not in SubPaths) or otherwise absent — treat as root.
			Roots.Add(Node);
		}
	}

	return Roots;
}

namespace
{
	/** Project content roots to scan as cook sources (excludes engine/script/temp). */
	TArray<FName> GatherSourceRoots()
	{
		TArray<FString> Roots;
		FPackageName::QueryRootContentPaths(Roots);

		TArray<FName> Out;
		for (FString Root : Roots)
		{
			if (Root.StartsWith(TEXT("/Engine")) || Root.StartsWith(TEXT("/Script"))
				|| Root.StartsWith(TEXT("/Temp")) || Root.StartsWith(TEXT("/Memory")))
			{
				continue;
			}
			// QueryRootContentPaths yields "/Game/"; AssetRegistry paths want "/Game".
			Root.RemoveFromEnd(TEXT("/"));
			if (!Root.IsEmpty())
			{
				Out.Add(FName(*Root));
			}
		}
		return Out;
	}

	/** Append violations for one source package's dependencies of a given hardness. */
	void CollectDeps(
		IAssetRegistry& Registry,
		const FName PackageName,
		const FString& PackageNameStr,
		const bool bHard,
		TSet<FString>& AlreadyFlagged,
		TArray<FCookViolation>& OutViolations)
	{
		TArray<FName> Dependencies;
		Registry.GetDependencies(
			PackageName,
			Dependencies,
			UE::AssetRegistry::EDependencyCategory::Package,
			bHard ? UE::AssetRegistry::EDependencyQuery::Hard
			      : UE::AssetRegistry::EDependencyQuery::Soft);

		for (const FName& Dep : Dependencies)
		{
			const FString DepStr = Dep.ToString();

			// Hard pass runs first; don't downgrade an already-hard hit to soft.
			if (!bHard && AlreadyFlagged.Contains(DepStr))
			{
				continue;
			}

			const FString HitDir = ResolveNeverCookDir(DepStr);
			if (!HitDir.IsEmpty())
			{
				FCookViolation Violation;
				Violation.SourcePackage = PackageNameStr;
				Violation.ReferencedPackage = DepStr;
				Violation.NeverCookDir = HitDir;
				Violation.bHardRef = bHard;
				OutViolations.Add(Violation);
				AlreadyFlagged.Add(DepStr);
			}
		}
	}
}

TArray<FCookViolation> FAssetCookScanner::ValidateNeverCookReferences(FCookScanProgress Progress)
{
	TArray<FCookViolation> Violations;

	// Collect NeverCook directories as long package paths.
	TMap<FString, ECookRuleType> Ruled;
	FAssetCookRuleManager::GetAllRuledDirectories(Ruled);

	TArray<FString> NeverCookDirs;
	for (const TPair<FString, ECookRuleType>& Pair : Ruled)
	{
		if (Pair.Value == ECookRuleType::NeverCook)
		{
			NeverCookDirs.Add(Pair.Key);
		}
	}

	if (NeverCookDirs.IsEmpty())
	{
		return Violations;
	}

	IAssetRegistry& Registry = GetAssetRegistry();

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	for (const FName& Root : GatherSourceRoots())
	{
		Filter.PackagePaths.Add(Root);
	}

	TArray<FAssetData> Assets;
	Registry.GetAssets(Filter, Assets);

	// Deduplicate per source package (one package can hold many assets).
	TArray<FName> UniquePackages;
	{
		TSet<FName> Seen;
		Seen.Reserve(Assets.Num());
		UniquePackages.Reserve(Assets.Num());
		for (const FAssetData& Asset : Assets)
		{
			bool bAlready = false;
			Seen.Add(Asset.PackageName, &bAlready);
			if (!bAlready)
			{
				UniquePackages.Add(Asset.PackageName);
			}
		}
	}

	const int32 Total = UniquePackages.Num();
	for (int32 Index = 0; Index < Total; ++Index)
	{
		if (Progress(Index, Total))
		{
			break; // Cancelled — return what we have so far.
		}

		const FName PackageName = UniquePackages[Index];
		const FString PackageNameStr = PackageName.ToString();

		// A NeverCook asset referencing another NeverCook asset is fine — skip sources
		// that are themselves excluded.
		if (!ResolveNeverCookDir(PackageNameStr).IsEmpty())
		{
			continue;
		}

		// Hard references break the cook; soft references only risk a runtime null.
		// Run hard first so a dependency reachable both ways is reported as hard.
		TSet<FString> Flagged;
		CollectDeps(Registry, PackageName, PackageNameStr, /*bHard=*/true, Flagged, Violations);
		CollectDeps(Registry, PackageName, PackageNameStr, /*bHard=*/false, Flagged, Violations);
	}

	return Violations;
}

TArray<FCookViolation> FAssetCookScanner::FindReferencers(const FString& TargetDir)
{
	TArray<FCookViolation> Results;
	if (TargetDir.IsEmpty())
	{
		return Results;
	}

	IAssetRegistry& Registry = GetAssetRegistry();

	// Every asset package under the target directory.
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Add(FName(*TargetDir));

	TArray<FAssetData> Assets;
	Registry.GetAssets(Filter, Assets);

	TSet<FName> TargetPackages;
	for (const FAssetData& Asset : Assets)
	{
		TargetPackages.Add(Asset.PackageName);
	}
	if (TargetPackages.IsEmpty())
	{
		return Results;
	}

	TSet<FString> Seen; // dedup on "source->target"
	for (const FName& TargetPkg : TargetPackages)
	{
		const FString TargetStr = TargetPkg.ToString();

		// Hard pass first so a referencer reachable both ways is reported as hard.
		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			const bool bHard = (Pass == 0);
			TArray<FName> Referencers;
			Registry.GetReferencers(
				TargetPkg,
				Referencers,
				UE::AssetRegistry::EDependencyCategory::Package,
				bHard ? UE::AssetRegistry::EDependencyQuery::Hard
				      : UE::AssetRegistry::EDependencyQuery::Soft);

			for (const FName& Ref : Referencers)
			{
				const FString RefStr = Ref.ToString();

				// Skip referencers inside the target directory itself.
				if (IsUnderDir(RefStr, TargetDir))
				{
					continue;
				}

				const FString Key = RefStr + TEXT("->") + TargetStr;
				if (Seen.Contains(Key))
				{
					continue;
				}
				Seen.Add(Key);

				FCookViolation V;
				V.SourcePackage = RefStr;
				V.ReferencedPackage = TargetStr;
				V.NeverCookDir = TargetDir;
				V.bHardRef = bHard;
				Results.Add(V);
			}
		}
	}

	return Results;
}
