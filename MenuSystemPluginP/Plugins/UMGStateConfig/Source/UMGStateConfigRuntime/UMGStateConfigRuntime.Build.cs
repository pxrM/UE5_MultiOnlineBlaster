using UnrealBuildTool;

public class UUsers\userb85bbe4d\projectsGStateConfigRuntime : ModuleRules
{
	public UMGStateConfigRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = Users\userb85bbe4d\projectsoduleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		OptimizeCode = CodeOptimization.InShippingBuildsOnly;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"SlateCore",
				"UMG",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
			}
		);
	}
}
