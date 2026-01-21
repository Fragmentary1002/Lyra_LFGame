// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Providers/Animation/AnimationProvider_HitReaction.h"

#include "AbilitySystem/NinjaCombatGameplayAbility.h"
#include "Interfaces/Animation/CombatAnimationContextProviderInterface.h"

UAnimMontage* UAnimationProvider_HitReaction::GetMontageToPlay_Implementation(UNinjaCombatGameplayAbility* CombatAbility) const
{
	if (IsValid(CombatAbility) && CombatAbility->Implements<UCombatAnimationContextProviderInterface>())
	{
		FGameplayTagContainer Context;
		ICombatAnimationContextProviderInterface::Execute_GetAnimationContext(CombatAbility, Context);

		if (!Context.IsEmpty())
		{
			for (const FHitReactionContext& HitReactionContext : HitReactionContexts)
			{
				if (Applies(Context, HitReactionContext))
				{
					return HitReactionContext.AnimMontage;
				}
			}	
		}
	}

	return Super::GetMontageToPlay_Implementation(CombatAbility);
}

bool UAnimationProvider_HitReaction::Applies_Implementation(const FGameplayTagContainer& AbilityContext, const FHitReactionContext& HitReactionContext) const
{
	return HitReactionContext.ContextTags.HasAnyExact(AbilityContext)
		&& IsValid(HitReactionContext.AnimMontage);
}
