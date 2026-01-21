// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Effects/CombatEffect_MeleeHit.h"

#include "NinjaCombatTags.h"
#include "AbilitySystem/NinjaCombatAttributeSet.h"
#include "GameplayEffectComponents/AssetTagsGameplayEffectComponent.h"

UCombatEffect_MeleeHit::UCombatEffect_MeleeHit()
{
	FAttributeBasedFloat MagnitudeAttribute;
	MagnitudeAttribute.Coefficient = 1.f;
	MagnitudeAttribute.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
		UNinjaCombatAttributeSet::GetBaseDamageAttribute(),
		EGameplayEffectAttributeCaptureSource::Source, false);

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UNinjaCombatAttributeSet::GetPendingDamageAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(MagnitudeAttribute);
	Modifiers.Add(Modifier);

	FInheritedTagContainer MyTags;
	MyTags.Added.AddTagFast(Tag_Combat_Effect_Damage_Melee);
	
	UAssetTagsGameplayEffectComponent* AssetTags = CreateDefaultSubobject<UAssetTagsGameplayEffectComponent>("AssetTags");
	AssetTags->SetAndApplyAssetTagChanges(MyTags);
	GEComponents.Add(AssetTags);
}
