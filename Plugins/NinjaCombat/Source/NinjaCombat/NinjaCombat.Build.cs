// Ninja Bear Studio Inc., all rights reserved.
using UnrealBuildTool;

public class NinjaCombat : ModuleRules
{
	public NinjaCombat(ReadOnlyTargetRules target) : base(target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new []
		{
			"AIModule",
			"Core", 
			"CoreUObject",
			"DeveloperSettings",
			"Engine", 
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayStateTreeModule",
			"GameplayTags",
			"GameplayTasks",
			"MotionWarping",
			"NetCore",
			"Niagara",
			"PhysicsCore",
			"StateTreeModule",
			"TargetingSystem",
			"LyraGame"
		});
		
		PrivateDependencyModuleNames.AddRange(new []
		{
			"NetCore",
			"Niagara"
		});
	}
}
