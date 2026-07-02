// Copyright TikiStar. All Rights Reserved.

using UnrealBuildTool;

public class UIFrameworkWidgets : ModuleRules
{
	public UIFrameworkWidgets(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UMG",
			"Slate",
			"SlateCore",
			"CommonUI",
			"ModelViewViewModel",
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
