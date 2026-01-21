// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Providers/NinjaCombatAnimationProvider.h"

#include "AbilitySystem/NinjaCombatGameplayAbility.h"
#include "Animation/AnimMontage.h"
#include "Interfaces/Animation/CombatAnimationMontageProviderInterface.h"

UNinjaCombatAnimationProvider::UNinjaCombatAnimationProvider()
{
	DefaultAnimationMontage = nullptr;
	DefaultSectionName = NAME_None;
}

UAnimMontage* UNinjaCombatAnimationProvider::GetMontageToPlay_Implementation(UNinjaCombatGameplayAbility* CombatAbility) const
{
	// This is a chained getter: Ability > Default.
	return GetAbilityAnimationMontage(CombatAbility);
}

FName UNinjaCombatAnimationProvider::GetSectionName_Implementation(UNinjaCombatGameplayAbility* CombatAbility) const
{
	// This is a chained getter: Ability > Default.
	return GetAbilitySectionName(CombatAbility);	
}

UAnimMontage* UNinjaCombatAnimationProvider::GetAbilityAnimationMontage_Implementation(const UNinjaCombatGameplayAbility* CombatAbility) const
{
	if (IsValid(CombatAbility) && CombatAbility->Implements<UCombatAnimationMontageProviderInterface>())
	{
		UAnimMontage* AnimMontage = ICombatAnimationMontageProviderInterface::Execute_GetAnimationMontage(CombatAbility);
		if (IsValid(AnimMontage)) return AnimMontage;
	}

	// Fallback: retrieve the Animation Montage set in the provider.
	return DefaultAnimationMontage;
}

FName UNinjaCombatAnimationProvider::GetAbilitySectionName_Implementation(const UNinjaCombatGameplayAbility* CombatAbility) const
{
	if (IsValid(CombatAbility) && CombatAbility->Implements<UCombatAnimationMontageProviderInterface>())
	{
		const FName Section = ICombatAnimationMontageProviderInterface::Execute_GetSectionName(CombatAbility); 
		if (Section != NAME_None) return Section;
	}

	return DefaultSectionName;
}
