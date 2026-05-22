using UnrealBuildTool;

public class UMGStateConfigEditor : ModuleRules
{
	public UMGStateConfigEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UMG",
				"UMGStateConfigRuntime",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"BlueprintGraph",
				"EditorFramework",
				"InputCore",
				"Kismet",
				"KismetCompiler",
				"PropertyEditor",
				"Projects",
				"ToolMenus",
				"UMGEditor",
				"UnrealEd",
			}
		);
	}
}
