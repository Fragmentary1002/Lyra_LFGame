// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Effects/CombatEffect_ConsumePoise.h"

#include "NinjaCombatTags.h"
#include "AbilitySystem/NinjaCombatAttributeSet.h"
#include "GameplayEffectComponents/TargetTagsGameplayEffectComponent.h"

UCombatEffect_ConsumePoise::UCombatEffect_ConsumePoise()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	// Configure the base modifier, using a flat float.
	
	FScalableFloat PoiseDecreaseFloat;
	PoiseDecreaseFloat.Value = -10.f;

	FGameplayModifierInfo ModifierInfoMagnitude;
	ModifierInfoMagnitude.Attribute = UNinjaCombatAttributeSet::GetPoiseAttribute();
	ModifierInfoMagnitude.ModifierMagnitude = FGameplayEffectModifierMagnitude(PoiseDecreaseFloat);
	ModifierInfoMagnitude.ModifierOp = EGameplayModOp::Additive;
	Modifiers.Add(ModifierInfoMagnitude);

	// Next, we want to also support a "Set By Caller" value.
	// This might be retrieved from the Melee or Projectile interfaces.
	
	FSetByCallerFloat PoiseDecreaseData;
	PoiseDecreaseData.DataName = TEXT("PoiseConsumption");
	PoiseDecreaseData.DataTag = Tag_Combat_Data_PoiseConsumption;
	
	FGameplayModifierInfo ModifierInfoDataCaller;
	ModifierInfoDataCaller.Attribute = UNinjaCombatAttributeSet::GetPoiseAttribute();
	ModifierInfoDataCaller.ModifierMagnitude = FGameplayEffectModifierMagnitude(PoiseDecreaseData);
	ModifierInfoDataCaller.ModifierOp = EGameplayModOp::Additive;
	Modifiers.Add(ModifierInfoDataCaller);

	// Finally, we'll configure the tag that blocks poise replenishment.

	FInheritedTagContainer MyTags;
	MyTags.Added.AddTagFast(Tag_Combat_Effect_Cancel_Regeneration_Poise);
	
	UTargetTagsGameplayEffectComponent* TargetTags = CreateDefaultSubobject<UTargetTagsGameplayEffectComponent>("TargetTags");
	TargetTags->SetAndApplyTargetTagChanges(MyTags);
	GEComponents.Add(TargetTags);
}
