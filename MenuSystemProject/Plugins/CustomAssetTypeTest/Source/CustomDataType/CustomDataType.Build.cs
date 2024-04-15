// Source/CustomDataType/CustomDataType.Build.cs

using UnrealBuildTool;

public class CustomDataType : ModuleRules
{
	public CustomDataType(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{ 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}