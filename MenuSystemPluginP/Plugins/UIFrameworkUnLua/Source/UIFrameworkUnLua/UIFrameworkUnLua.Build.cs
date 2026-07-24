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
			"UIFrameworkWidgets",
			"UnLua"
		});

		PrivateDependencyModuleNames.Add("Lua");
	}
}
