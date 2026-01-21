// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaCombatMontageAbility.h"
#include "Interfaces/Animation/CombatAnimationMontageProviderInterface.h"
#include "CombatAbility_Evade.generated.h"

class UAbilityTask_WaitGameplayEvent;

/**
 * Implementation of the Evade Mechanic, including the handling of the Invulnerability Effect.
 */
UCLASS()
class NINJACOMBAT_API UCombatAbility_Evade : public UNinjaCombatMontageAbility, public ICombatAnimationMontageProviderInterface
{
	
	GENERATED_BODY()

public:

	UCombatAbility_Evade(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/** Gameplay Effect applied on Invulnerability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Evade")
	TSubclassOf<UGameplayEffect> InvulnerabilityEffectClass;

	// -- Begin Gameplay Ability implementation
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void HandleEventReceived_Implementation(FGameplayEventData Payload) override;
	// -- End Gameplay Ability implementation
	
	/**
	 * Applies the Invulnerability Effect.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	void GrantInvulnerabilityEffect();

	/**
	 * Revokes the active Invulnerability Effect.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	void RevokeInvulnerabilityEffect();
	
private:

	/** Handles Invulnerability Effects. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> InvulnerabilityEventTask;

	/** Handle for the applied Invulnerability Effect. */
	FActiveGameplayEffectHandle InvulnerabilityEffectHandle;
	
};
