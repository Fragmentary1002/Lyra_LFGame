// Ninja Bear Studio Inc., all rights reserved.
// ReSharper disable InconsistentNaming
using UnrealBuildTool;

public class NinjaCombatUI : ModuleRules
{
    public NinjaCombatUI(ReadOnlyTargetRules target) : base(target)
    {
	    PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

	    PublicDependencyModuleNames.AddRange(new []
	    {
		    "Core", 
		    "CoreUObject",
		    "Engine", 
		    "GameplayTags",
		    "GameplayTasks",
		    "GameplayAbilities",
		    "NetCore",
		    "NinjaCombat",
		    "UMG"
	    });
		
	    PrivateDependencyModuleNames.AddRange(new []
	    {
		    "NinjaCombat",
		    "Slate",
		    "SlateCore"
	    });
    }
}