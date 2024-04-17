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
            "Slate",
			"SlateCore",
            "CustomDataType",
			"UnrealEd",
			"AssetTools",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}