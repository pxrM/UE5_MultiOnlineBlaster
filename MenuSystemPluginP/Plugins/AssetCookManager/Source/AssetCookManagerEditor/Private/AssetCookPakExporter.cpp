#include "AssetCookPakExporter.h"

#include "AssetRegistry/ARFilter.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "DesktopPlatformModule.h"
#include "HAL/FileManager.h"
#include "IDesktopPlatform.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "Misc/Paths.h"
#include "Settings/ProjectPackagingSettings.h"

#define LOCTEXT_NAMESPACE "AssetCookPakExporter"

namespace
{
	enum class EDebugCookContainerFormat : uint8
	{
		Pak,
		IoStore,
	};

	IAssetRegistry& GetPakExporterAssetRegistry()
	{
		FAssetRegistryModule& Module = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		return Module.Get();
	}

	FString SanitizeBaseName(FString Name)
	{
		if (Name.IsEmpty())
		{
			Name = TEXT("DebugAssets");
		}

		const TCHAR* InvalidChars = INVALID_LONGPACKAGE_CHARACTERS TEXT("\\/:*?\"<>| ");
		for (int32 Index = 0; InvalidChars[Index] != 0; ++Index)
		{
			Name.ReplaceCharInline(InvalidChars[Index], TEXT('_'));
		}
		return Name;
	}

	bool TryGetGameRelativePath(const FString& PackageName, FString& OutRelative)
	{
		if (PackageName == TEXT("/Game"))
		{
			OutRelative.Reset();
			return true;
		}

		const FString Prefix = TEXT("/Game/");
		if (!PackageName.StartsWith(Prefix))
		{
			return false;
		}

		OutRelative = PackageName.RightChop(Prefix.Len());
		return !OutRelative.IsEmpty();
	}

	bool TryFindCookedProjectRoot(FString& OutCookedProjectRoot, FString& OutPlatformName)
	{
		const FString CookedRoot = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Cooked"));
		if (!IFileManager::Get().DirectoryExists(*CookedRoot))
		{
			return false;
		}

		TArray<FString> PlatformDirs;
		IFileManager::Get().FindFiles(PlatformDirs, *CookedRoot, false, true);

		const FString ProjectName = FApp::GetProjectName();
		TArray<FString> PreferredPlatforms = { TEXT("Windows"), TEXT("Win64"), TEXT("WindowsEditor") };
		PreferredPlatforms.Append(PlatformDirs);

		TSet<FString> Seen;
		for (const FString& Platform : PreferredPlatforms)
		{
			if (Platform.IsEmpty() || Seen.Contains(Platform))
			{
				continue;
			}
			Seen.Add(Platform);

			const FString Candidate = FPaths::Combine(CookedRoot, Platform, ProjectName);
			if (IFileManager::Get().DirectoryExists(*FPaths::Combine(Candidate, TEXT("Content"))))
			{
				OutCookedProjectRoot = Candidate;
				OutPlatformName = Platform;
				return true;
			}
		}

		return false;
	}

	bool TryGetUnrealPakPath(FString& OutUnrealPakPath)
	{
#if PLATFORM_WINDOWS
		OutUnrealPakPath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries/Win64/UnrealPak.exe")));
#else
		OutUnrealPakPath = FPaths::ConvertRelativePathToFull(
			FPaths::Combine(FPaths::EngineDir(), TEXT("Binaries"), FPlatformProcess::GetBinariesSubdirectory(), TEXT("UnrealPak")));
#endif
		return FPaths::FileExists(OutUnrealPakPath);
	}

	EDebugCookContainerFormat GetPreferredContainerFormat()
	{
		const UProjectPackagingSettings* PackagingSettings = GetDefault<UProjectPackagingSettings>();
		return PackagingSettings && PackagingSettings->bUseIoStore
			? EDebugCookContainerFormat::IoStore
			: EDebugCookContainerFormat::Pak;
	}

	void CollectPackagesFromDirectories(const TArray<FString>& PackagePaths, TArray<FName>& OutPackageNames)
	{
		FARFilter Filter;
		Filter.bRecursivePaths = true;

		for (const FString& PackagePath : PackagePaths)
		{
			if (PackagePath == TEXT("/Game") || PackagePath.StartsWith(TEXT("/Game/")))
			{
				Filter.PackagePaths.AddUnique(FName(*PackagePath));
			}
		}

		if (Filter.PackagePaths.IsEmpty())
		{
			return;
		}

		TArray<FAssetData> Assets;
		GetPakExporterAssetRegistry().GetAssets(Filter, Assets);

		TSet<FName> Seen;
		for (const FAssetData& Asset : Assets)
		{
			bool bAlreadySeen = false;
			Seen.Add(Asset.PackageName, &bAlreadySeen);
			if (!bAlreadySeen)
			{
				OutPackageNames.Add(Asset.PackageName);
			}
		}
	}

	void CollectCookedPackageFiles(
		const FString& CookedProjectRoot,
		const TArray<FName>& PackageNames,
		TArray<FString>& OutCookedFiles,
		TArray<FString>& OutMissingPackages,
		int32& OutSkippedNonGame)
	{
		static const TCHAR* CookedExtensions[] =
		{
			TEXT(".uasset"),
			TEXT(".umap"),
			TEXT(".uexp"),
			TEXT(".ubulk"),
			TEXT(".uptnl"),
		};

		TSet<FString> SeenFiles;
		OutSkippedNonGame = 0;

		for (const FName& PackageName : PackageNames)
		{
			FString RelativePackage;
			if (!TryGetGameRelativePath(PackageName.ToString(), RelativePackage))
			{
				++OutSkippedNonGame;
				continue;
			}

			const FString CookedBasePath = FPaths::Combine(CookedProjectRoot, TEXT("Content"), RelativePackage);
			bool bFoundAnyFile = false;
			for (const TCHAR* Extension : CookedExtensions)
			{
				const FString FilePath = CookedBasePath + Extension;
				if (FPaths::FileExists(FilePath))
				{
					FString FullPath = FPaths::ConvertRelativePathToFull(FilePath);
					FPaths::NormalizeFilename(FullPath);

					if (!SeenFiles.Contains(FullPath))
					{
						SeenFiles.Add(FullPath);
						OutCookedFiles.Add(FullPath);
					}
					bFoundAnyFile = true;
				}
			}

			if (!bFoundAnyFile)
			{
				OutMissingPackages.Add(PackageName.ToString());
			}
		}
	}

	bool PromptForOutputPath(const FString& SuggestedBaseName, EDebugCookContainerFormat Format, FString& OutPath)
	{
		IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
		if (!DesktopPlatform)
		{
			return false;
		}

		const FString DefaultDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Paks"));
		IFileManager::Get().MakeDirectory(*DefaultDir, true);

		TArray<FString> OutFiles;
		const FString Extension = Format == EDebugCookContainerFormat::IoStore ? TEXT(".utoc") : TEXT(".pak");
		const FString FileTypes = Format == EDebugCookContainerFormat::IoStore
			? FString(TEXT("IoStore TOC (*.utoc)|*.utoc"))
			: FString(TEXT("Pak (*.pak)|*.pak"));
		const bool bPicked = DesktopPlatform->SaveFileDialog(
			nullptr,
			Format == EDebugCookContainerFormat::IoStore
				? LOCTEXT("SaveIoStoreDialogTitle", "Export Debug IoStore Container").ToString()
				: LOCTEXT("SavePakDialogTitle", "Export Debug Pak").ToString(),
			DefaultDir,
			SanitizeBaseName(SuggestedBaseName) + Extension,
			FileTypes,
			EFileDialogFlags::None,
			OutFiles);

		if (!bPicked || OutFiles.IsEmpty())
		{
			return false;
		}

		OutPath = OutFiles[0];
		if (FPaths::GetExtension(OutPath).IsEmpty())
		{
			OutPath += Extension;
		}
		return true;
	}

	FString MakePakMountPath(const FString& CookedProjectRoot, const FString& CookedFile)
	{
		FString RelativePath = CookedFile;
		FPaths::MakePathRelativeTo(RelativePath, *CookedProjectRoot);
		FPaths::NormalizeFilename(RelativePath);
		return FString::Printf(TEXT("../../../%s/%s"), FApp::GetProjectName(), *RelativePath);
	}

	bool TryGetIoStoreMetadataPaths(const FString& CookedProjectRoot, FString& OutPackageStoreManifest, FString& OutScriptObjects)
	{
		const FString MetadataDir = FPaths::Combine(CookedProjectRoot, TEXT("Metadata"));
		OutPackageStoreManifest = FPaths::Combine(MetadataDir, TEXT("packagestore.manifest"));
		OutScriptObjects = FPaths::Combine(MetadataDir, TEXT("scriptobjects.bin"));
		return FPaths::FileExists(OutPackageStoreManifest) && FPaths::FileExists(OutScriptObjects);
	}

	void ShowMissingCookedFilesMessage(const FString& CookedRoot, const FString& Platform, const TArray<FString>& MissingPackages, int32 SkippedNonGame)
	{
		FString Details;
		if (SkippedNonGame > 0)
		{
			Details += FString::Printf(TEXT("Skipped %d non-/Game package(s).\n\n"), SkippedNonGame);
		}

		Details += FString::Printf(TEXT("Cooked root:\n%s\n\n"), *CookedRoot);
		Details += FString::Printf(TEXT("Platform: %s\n\n"), *Platform);
		Details += TEXT("Missing cooked package files. Cook the project for this platform first, then export again.");

		const int32 MaxShown = FMath::Min(20, MissingPackages.Num());
		if (MaxShown > 0)
		{
			Details += TEXT("\n\nFirst missing package(s):");
			for (int32 Index = 0; Index < MaxShown; ++Index)
			{
				Details += TEXT("\n");
				Details += MissingPackages[Index];
			}
		}

		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Details));
	}

	bool RunUnrealPak(const FString& Args, FString& OutStdOut, FString& OutStdErr, int32& OutReturnCode)
	{
		FString UnrealPakPath;
		if (!TryGetUnrealPakPath(UnrealPakPath))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoUnrealPak", "UnrealPak was not found in the current engine installation."));
			return false;
		}

		if (!FPlatformProcess::ExecProcess(*UnrealPakPath, *Args, &OutReturnCode, &OutStdOut, &OutStdErr))
		{
			OutReturnCode = -1;
			return false;
		}

		return OutReturnCode == 0;
	}

	void ShowUnrealPakFailure(const FString& Operation, int32 ReturnCode, const FString& StdOut, const FString& StdErr)
	{
		const FString Message = FString::Printf(
			TEXT("%s failed.\n\nReturn code: %d\n\n%s\n%s"),
			*Operation,
			ReturnCode,
			*StdOut,
			*StdErr);
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
	}

	void ExportPak(
		const FString& CookedProjectRoot,
		const FString& SuggestedBaseName,
		const TArray<FString>& CookedFiles)
	{
		FString PakPath;
		if (!PromptForOutputPath(SuggestedBaseName, EDebugCookContainerFormat::Pak, PakPath))
		{
			return;
		}

		const FString WorkDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AssetCookManager"), TEXT("PakResponses"));
		IFileManager::Get().MakeDirectory(*WorkDir, true);

		const FString ResponsePath = FPaths::Combine(WorkDir, SanitizeBaseName(SuggestedBaseName) + TEXT(".txt"));
		TArray<FString> Lines;
		for (const FString& CookedFile : CookedFiles)
		{
			Lines.Add(FString::Printf(TEXT("\"%s\" \"%s\""), *CookedFile, *MakePakMountPath(CookedProjectRoot, CookedFile)));
		}

		if (!FFileHelper::SaveStringArrayToFile(Lines, *ResponsePath))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ResponseWriteFailed", "Failed to write the UnrealPak response file."));
			return;
		}

		IFileManager::Get().MakeDirectory(*FPaths::GetPath(PakPath), true);

		FString StdOut;
		FString StdErr;
		int32 ReturnCode = 0;
		const FString Args = FString::Printf(TEXT("\"%s\" -Create=\"%s\""), *PakPath, *ResponsePath);
		if (!RunUnrealPak(Args, StdOut, StdErr, ReturnCode) || !FPaths::FileExists(PakPath))
		{
			ShowUnrealPakFailure(TEXT("UnrealPak"), ReturnCode, StdOut, StdErr);
			return;
		}

		const FText Done = FText::Format(
			LOCTEXT("PakExportDone", "Created debug pak with {0} cooked file(s):\n{1}"),
			FText::AsNumber(CookedFiles.Num()),
			FText::FromString(PakPath));
		FMessageDialog::Open(EAppMsgType::Ok, Done);
	}

	void ExportIoStore(
		const FString& CookedProjectRoot,
		const FString& SuggestedBaseName,
		const TArray<FString>& CookedFiles)
	{
		FString UtocPath;
		if (!PromptForOutputPath(SuggestedBaseName, EDebugCookContainerFormat::IoStore, UtocPath))
		{
			return;
		}

		UtocPath = FPaths::ChangeExtension(UtocPath, TEXT("utoc"));
		const FString UcasPath = FPaths::ChangeExtension(UtocPath, TEXT("ucas"));
		const FString GlobalUtocPath = FPaths::Combine(FPaths::GetPath(UtocPath), SanitizeBaseName(SuggestedBaseName) + TEXT("_global.utoc"));

		FString PackageStoreManifest;
		FString ScriptObjects;
		if (!TryGetIoStoreMetadataPaths(CookedProjectRoot, PackageStoreManifest, ScriptObjects))
		{
			const FString Message = FString::Printf(
				TEXT("IoStore export requires cooked metadata.\n\nExpected:\n%s\n%s\n\nCook or package the project with IoStore enabled, then export again."),
				*FPaths::Combine(CookedProjectRoot, TEXT("Metadata/packagestore.manifest")),
				*FPaths::Combine(CookedProjectRoot, TEXT("Metadata/scriptobjects.bin")));
			FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Message));
			return;
		}

		const FString WorkDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("AssetCookManager"), TEXT("IoStoreResponses"));
		IFileManager::Get().MakeDirectory(*WorkDir, true);
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(UtocPath), true);

		const FString BaseName = SanitizeBaseName(SuggestedBaseName);
		const FString ResponsePath = FPaths::Combine(WorkDir, BaseName + TEXT("_Response.txt"));
		const FString CommandsPath = FPaths::Combine(WorkDir, BaseName + TEXT("_Commands.txt"));
		const FString ContainerName = BaseName + TEXT("_Debug");

		TArray<FString> ResponseLines;
		for (const FString& CookedFile : CookedFiles)
		{
			ResponseLines.Add(FString::Printf(TEXT("\"%s\" \"%s\""), *CookedFile, *MakePakMountPath(CookedProjectRoot, CookedFile)));
		}

		if (!FFileHelper::SaveStringArrayToFile(ResponseLines, *ResponsePath))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("IoResponseWriteFailed", "Failed to write the IoStore response file."));
			return;
		}

		const FString CommandLine = FString::Printf(
			TEXT("-Output=\"%s\" -ContainerName=%s -ResponseFile=\"%s\""),
			*UtocPath,
			*ContainerName,
			*ResponsePath);
		if (!FFileHelper::SaveStringToFile(CommandLine + LINE_TERMINATOR, *CommandsPath))
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("IoCommandsWriteFailed", "Failed to write the IoStore commands file."));
			return;
		}

		const FString ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath());
		const FString Args = FString::Printf(
			TEXT("\"%s\" -CreateGlobalContainer=\"%s\" -CookedDirectory=\"%s\" -Commands=\"%s\" -PackageStoreManifest=\"%s\" -ScriptObjects=\"%s\" -unattended"),
			*ProjectPath,
			*GlobalUtocPath,
			*CookedProjectRoot,
			*CommandsPath,
			*PackageStoreManifest,
			*ScriptObjects);

		FString StdOut;
		FString StdErr;
		int32 ReturnCode = 0;
		if (!RunUnrealPak(Args, StdOut, StdErr, ReturnCode) || !FPaths::FileExists(UtocPath) || !FPaths::FileExists(UcasPath))
		{
			ShowUnrealPakFailure(TEXT("IoStore export"), ReturnCode, StdOut, StdErr);
			return;
		}

		const FText Done = FText::Format(
			LOCTEXT("IoStoreExportDone", "Created debug IoStore container with {0} cooked file(s):\n{1}\n{2}\n\nCopy the .utoc and .ucas beside your packaged build's other containers."),
			FText::AsNumber(CookedFiles.Num()),
			FText::FromString(UtocPath),
			FText::FromString(UcasPath));
		FMessageDialog::Open(EAppMsgType::Ok, Done);
	}

	void ExportPackagesInternal(const TArray<FName>& PackageNames, const FString& SuggestedBaseName)
	{
		if (PackageNames.IsEmpty())
		{
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("NoPackages", "No asset packages were selected."));
			return;
		}

		FString CookedProjectRoot;
		FString PlatformName;
		if (!TryFindCookedProjectRoot(CookedProjectRoot, PlatformName))
		{
			FMessageDialog::Open(EAppMsgType::Ok,
				LOCTEXT("NoCookedRoot", "No cooked project content was found under Saved/Cooked. Cook the project first, then export a debug pak."));
			return;
		}

		TArray<FString> CookedFiles;
		TArray<FString> MissingPackages;
		int32 SkippedNonGame = 0;
		CollectCookedPackageFiles(CookedProjectRoot, PackageNames, CookedFiles, MissingPackages, SkippedNonGame);

		if (CookedFiles.IsEmpty())
		{
			ShowMissingCookedFilesMessage(CookedProjectRoot, PlatformName, MissingPackages, SkippedNonGame);
			return;
		}

		if (!MissingPackages.IsEmpty() || SkippedNonGame > 0)
		{
			const FText Confirm = FText::Format(
				LOCTEXT("PartialPakConfirm", "Found {0} cooked file(s), but {1} package(s) are missing cooked files and {2} non-/Game package(s) were skipped.\n\nCreate a partial pak anyway?"),
				FText::AsNumber(CookedFiles.Num()),
				FText::AsNumber(MissingPackages.Num()),
				FText::AsNumber(SkippedNonGame));

			if (FMessageDialog::Open(EAppMsgType::YesNo, Confirm) != EAppReturnType::Yes)
			{
				ShowMissingCookedFilesMessage(CookedProjectRoot, PlatformName, MissingPackages, SkippedNonGame);
				return;
			}
		}

		if (GetPreferredContainerFormat() == EDebugCookContainerFormat::IoStore)
		{
			ExportIoStore(CookedProjectRoot, SuggestedBaseName, CookedFiles);
		}
		else
		{
			ExportPak(CookedProjectRoot, SuggestedBaseName, CookedFiles);
		}
	}
}

void FAssetCookPakExporter::ExportPackagesInteractive(const TArray<FName>& PackageNames, const FString& SuggestedBaseName)
{
	ExportPackagesInternal(PackageNames, SuggestedBaseName);
}

void FAssetCookPakExporter::ExportDirectoriesInteractive(const TArray<FString>& PackagePaths, const FString& SuggestedBaseName)
{
	TArray<FName> PackageNames;
	CollectPackagesFromDirectories(PackagePaths, PackageNames);
	ExportPackagesInternal(PackageNames, SuggestedBaseName);
}

#undef LOCTEXT_NAMESPACE
