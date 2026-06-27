// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class LyraFPSGameRuntime : ModuleRules
{
	public LyraFPSGameRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", "LyraGame", "GameplayTags","ModularGameplay","EnhancedInput","GameplayAbilities", "GameplayTasks"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"CommonUser",
				"AIModule", //继承LyraPlayerState需要
				"ModularGameplayActors",  //继承LyraPlayerState需要
				"EasyFPSProcedualAnim",
				"PhysicsCore"			//WeaponBase的Fire函数
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
