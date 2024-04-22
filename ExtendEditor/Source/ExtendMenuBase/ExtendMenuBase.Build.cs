// ExtendMenuBase.Build.cs

using UnrealBuildTool;

public class ExtendMenuBase : ModuleRules
{
	public ExtendMenuBase(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
            "SlateCore",
            "Slate",
			"ToolMenus"
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}