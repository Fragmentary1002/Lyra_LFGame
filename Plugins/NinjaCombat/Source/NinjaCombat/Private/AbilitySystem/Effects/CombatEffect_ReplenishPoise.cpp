// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Effects/CombatEffect_ReplenishPoise.h"

#include "NinjaCombatTags.h"
#include "AbilitySystem/NinjaCombatAttributeSet.h"
#include "GameplayEffectComponents/TargetTagRequirementsGameplayEffectComponent.h"

UCombatEffect_ReplenishPoise::UCombatEffect_ReplenishPoise()
{
	// Period setup is different than other "replenish" effects.
	//
	// This is a periodic effect, happening every 3.5 seconds (or any other value configured). It
	// never triggers as soon as it's applied but instead it's triggered after the first period elapses.
	//
	// Furthermore, this can be interrupted but the Tag_Combat_Effect_Cancel_Regeneration_Poise tag and
	// whenever this inhibition ends, it will reset the period and then wait again, like a "cooldown".
	//
	DurationPolicy = EGameplayEffectDurationType::Infinite;
	Period = FScalableFloat(3.5f);
	bExecutePeriodicEffectOnApplication = false;
	PeriodicInhibitionPolicy = EGameplayEffectPeriodInhibitionRemovedPolicy::ResetPeriod;

	// As for the backing attribute for the magnitude, by default poise will always replenish in full.
	//
	FAttributeBasedFloat MagnitudeAttribute;
	MagnitudeAttribute.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
		UNinjaCombatAttributeSet::GetMaxPoiseAttribute(),
		EGameplayEffectAttributeCaptureSource::Source, false);
	
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UNinjaCombatAttributeSet::GetPoiseAttribute();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(MagnitudeAttribute);
	Modifiers.Add(ModifierInfo);

	// And finally, as mentioned before, replenishing poise can be interrupted if the owner 
	// has the "Tag_Combat_Effect_Cancel_Regeneration_Poise". This is most likely applied by
	// another Gameplay Effect, probably attached to an Attack.
	//
	UTargetTagRequirementsGameplayEffectComponent* TargetRequirements = CreateDefaultSubobject<UTargetTagRequirementsGameplayEffectComponent>("TargetRequirements");
	TargetRequirements->OngoingTagRequirements.IgnoreTags.AddTag(Tag_Combat_Effect_Cancel_Regeneration_Poise);
	GEComponents.Add(TargetRequirements);
}
