// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaCombatGameplayAbility.h"
#include "CombatAbility_Combo.generated.h"

class UInputAction;
class UNinjaCombatComboSetupData;
class UAbilityTask_WaitGameplayEvent;

/**
 * An ability that works in conjunction with the Combo Component to orchestrate multiple abilities.
 */
UCLASS()
class NINJACOMBAT_API UCombatAbility_Combo : public UNinjaCombatGameplayAbility
{
	
	GENERATED_BODY()

public:

	UCombatAbility_Combo(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Checks if this ability is in the combo window.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	bool InComboWindow() const;

	/**
	 * Provides the current counter for the combo. 
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	int32 GetComboCounter() const;	
	
protected:
	
	/** Data representing the current combo. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Combo")
	TObjectPtr<UNinjaCombatComboSetupData> ComboData;

	/** Gameplay Effect applied when the character is in a Combo Window. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Combo")
	TSubclassOf<UGameplayEffect> ComboWindowEffectClass;
	
	// -- Begin Gameplay Ability implementation
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void HandleEventReceived_Implementation(FGameplayEventData Payload) override;
	// -- End Gameplay Ability implementation

	/**
	 * Handles the Combo Finished event, broadcast by the Combo Manager.
	 */
	UFUNCTION()
	void HandleComboFinished(const UNinjaCombatComboSetupData* FinishedComboData, bool bSucceeded);

	/**
	 * Retrieves the Input Action from an Event Payload.
	 * 
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combo Ability")
	const UInputAction* GetInputActionFromEvent(const FGameplayEventData& Payload) const;

	/**
	 * Applies the Combo Window Effect.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	void GrantComboWindowEffect();

	/**
	 * Revokes the active Combo Window Effect.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	void RevokeComboWindowEffect();
	
private:

	/** Combo component assigned to the owner. */
	UPROPERTY()
	TObjectPtr<UActorComponent> ComboManager;

	/** Keeps track of a combo events. */
    UPROPERTY()
    TObjectPtr<UAbilityTask_WaitGameplayEvent> ComboEventTask;

	/** Handle for the applied Combo Window Effect. */
	FActiveGameplayEffectHandle ComboWindowEffectHandle;
	
};
