// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_Stagger.h"

#include "NinjaCombatTags.h"

UCombatAbility_Stagger::UCombatAbility_Stagger(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// On top of being a Hit Reaction (which stagger is too), we want to mark this
	// ability as the "stagger" so it's easier to cancel this at any moment, if needed.
	//
	AbilityTags.AddTag(Tag_Combat_Ability_Stagger);
	
	// Normal Hit Reactions will be blocked if the Avatar is Attacking or Evading.
	// We must remove these restrictions since stagger will actually cancel these abilities.
	//
	ActivationBlockedTags.RemoveTag(Tag_Combat_Ability_Attack);
	ActivationBlockedTags.RemoveTag(Tag_Combat_Ability_Evade);

	// And now we can configure this ability so when staggered, an avatar will interrupt
	// any Attack or Evade that is being executed.
	//
	CancelAbilitiesWithTag.AddTag(Tag_Combat_Ability_Attack);
	CancelAbilitiesWithTag.AddTag(Tag_Combat_Ability_Evade);

	// Finally, we want to remove the original trigger (Damage.Received) as that only
	// represents the basic Hit Reaction, and add the proper event tag that this ability
	// should react to: Damage.Stagger.
	//
	// Note: Like normal Hit Reactions, this ability must be added by default to the
	// avatar, so it can react to events as needed. It's never directly "activated".
	//
	FAbilityTriggerData StaggerTrigger;
	StaggerTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	StaggerTrigger.TriggerTag = Tag_Combat_Event_Damage_Stagger;
	AbilityTriggers.Empty();
	AbilityTriggers.Add(StaggerTrigger);
}

void UCombatAbility_Stagger::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (TriggerEventData->EventTag == Tag_Combat_Event_Damage_Stagger)
	{
		ExecuteHitReaction(*TriggerEventData);
	}
}
