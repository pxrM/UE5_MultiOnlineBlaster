using UnrealBuildTool;

public class WidgetAnimTimelineEditor : ModuleRules
{
	public WidgetAnimTimelineEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		bUseUnity = false;
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
			});

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"WidgetAnimTimeline",
				"Slate",
				"SlateCore",
				"UMG",
				"UMGEditor",
				"UnrealEd",
				"ToolMenus",
				"Kismet",
				"PropertyEditor",
				"EditorSubsystem",
				"InputCore",
				"ApplicationCore",
				"MovieScene",
				"MovieSceneTracks"
			});
	}
}
