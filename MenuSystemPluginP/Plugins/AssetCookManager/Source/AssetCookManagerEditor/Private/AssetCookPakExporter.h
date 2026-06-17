#pragma once

#include "CoreMinimal.h"

/** Exports already-cooked /Game assets into a small debug pak via UnrealPak. */
class FAssetCookPakExporter
{
public:
	static void ExportPackagesInteractive(const TArray<FName>& PackageNames, const FString& SuggestedBaseName);
	static void ExportDirectoriesInteractive(const TArray<FString>& PackagePaths, const FString& SuggestedBaseName);
};
