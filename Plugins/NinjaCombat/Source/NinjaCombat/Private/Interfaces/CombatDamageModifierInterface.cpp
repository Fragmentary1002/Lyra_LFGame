// Ninja Bear Studio Inc., all rights reserved.
#include "Interfaces/CombatDamageModifierInterface.h"

float ICombatDamageModifierInterface::ModifyDamage_Implementation(const float Damage, const AActor* Instigator,
	const AActor* Target, FGameplayTagContainer SourceTags, FGameplayTagContainer TargetTags) const
{
	return Damage;
}
