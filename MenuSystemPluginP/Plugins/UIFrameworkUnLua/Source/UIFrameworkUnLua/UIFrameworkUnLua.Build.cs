// Copyright TikiStar. All Rights Reserved.

using UnrealBuildTool;

public class UIFrameworkUnLua : ModuleRules
{
	public UIFrameworkUnLua(ReadOnlyTargetRules Target) : base(Target)
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
			"CommonInput",
			"GameplayTags",
			"FieldNotification",
			"UIFrameworkCore",
			"UIFrameworkWidgets",
			"UnLua"
		});

		// Only the automation tests include lua.hpp directly; the runtime adapter
		// reaches Lua exclusively through UnLua's reflection layer.
		if (Target.bBuildDeveloperTools)
		{
			PrivateDependencyModuleNames.Add("Lua");
		}
	}
}
