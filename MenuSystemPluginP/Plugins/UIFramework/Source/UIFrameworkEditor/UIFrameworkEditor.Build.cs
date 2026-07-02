// Copyright TikiStar. All Rights Reserved.

using UnrealBuildTool;

public class UIFrameworkEditor : ModuleRules
{
	public UIFrameworkEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"Slate",
			"SlateCore",
			"UIFrameworkWidgets"
		});
	}
}
