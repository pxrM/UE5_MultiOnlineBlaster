using UnrealBuildTool;

public class AssetCookManagerEditor : ModuleRules
{
	public AssetCookManagerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"ApplicationCore",
				"AssetRegistry",
				"AssetTools",
				"ContentBrowser",
				"DesktopPlatform",
				"DeveloperToolSettings",
				"EditorFramework",
				"InputCore",
				"Projects",
				"ToolMenus",
				"UnrealEd",
				"WorkspaceMenuStructure",
			}
		);
	}
}
