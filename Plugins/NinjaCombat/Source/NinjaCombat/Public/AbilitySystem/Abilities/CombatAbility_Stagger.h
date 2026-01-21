// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CombatAbility_HitReaction.h"
#include "CombatAbility_Stagger.generated.h"

/**
 * Handles a Stagger on the Avatar.
 *
 * This is a specialized Hit Reaction that will cancel Attacks and Evades.
 */
UCLASS()
class NINJACOMBAT_API UCombatAbility_Stagger : public UCombatAbility_HitReaction
{
	
	GENERATED_BODY()

public:

	UCombatAbility_Stagger(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Gameplay Ability implementation
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// -- End Gameplay Ability implementation	
	
};
