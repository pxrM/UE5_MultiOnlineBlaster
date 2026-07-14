// Copyright TikiStar. All Rights Reserved.

using UnrealBuildTool;

public class UIFrameworkCore : ModuleRules
{
	public UIFrameworkCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
		});
	}
}
