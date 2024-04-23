// ExtendEditorCommand.Build.cs

using UnrealBuildTool;

public class ExtendEditorCommand : ModuleRules
{
	public ExtendEditorCommand(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"Slate",
            "ExtendEditorStyle",
        });

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}