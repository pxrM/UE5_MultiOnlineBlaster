// Source/CustomDataTypeEditor/CustomDataTypeEditor.Build.cs

using UnrealBuildTool;

public class CustomDataTypeEditor : ModuleRules
{
	public CustomDataTypeEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"CustomDataType",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}