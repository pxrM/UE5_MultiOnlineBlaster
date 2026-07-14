// Copyright TikiStar. All Rights Reserved.

using System.IO;
using UnrealBuildTool;

public class UIFrameworkWidgets : ModuleRules
{
	public UIFrameworkWidgets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Public headers are grouped in subfolders; expose each so includes stay
		// path-independent (a header can be included by bare name, and files can be
		// reorganized between these folders without touching include statements).
		PublicIncludePaths.AddRange(new string[]
		{
			Path.Combine(ModuleDirectory, "Public", "Config"),
			Path.Combine(ModuleDirectory, "Public", "Layers"),
			Path.Combine(ModuleDirectory, "Public", "Management"),
			Path.Combine(ModuleDirectory, "Public", "Pool"),
			Path.Combine(ModuleDirectory, "Public", "ViewModels"),
			Path.Combine(ModuleDirectory, "Public", "Styles"),
			Path.Combine(ModuleDirectory, "Public", "Widgets")
		});

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",
			"Slate",
			"SlateCore",
			"CommonUI",
			"CommonInput",
			"ModelViewViewModel",
			"FieldNotification",
			"GameplayTags",
			"DeveloperSettings",
			"UIFrameworkCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"InputCore"
		});
	}
}
