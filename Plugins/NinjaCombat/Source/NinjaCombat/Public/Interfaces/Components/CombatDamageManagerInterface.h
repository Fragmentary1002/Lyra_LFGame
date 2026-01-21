// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaCombatDelegates.h"
#include "Types/FDamageCalculation.h"
#include "Types/FDamageEntry.h"
#include "UObject/Interface.h"
#include "CombatDamageManagerInterface.generated.h"

class UGameplayAbility;

UINTERFACE()
class UCombatDamageManagerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Defines the API for a component that manages damage.
 */
class NINJACOMBAT_API ICombatDamageManagerInterface
{
	
	GENERATED_BODY()

public:
	
	/**
	 * Binds to a Multicast Delegate that will notify about damage received.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage Manager Interface")
	void BindToDamageReceivedDelegate(const FDamageReceivedDelegate& Delegate);

	/**
	 * Binds to a Multicast Delegate that will notify about the owner's death.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage Manager Interface")
	void BindToOwnerDiedDelegate(const FOwnerDiedDelegate& Delegate);

	/**
	 * Unbinds an object from the damage received delegate.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage Manager Interface")
	void UnbindFromDamageReceivedDelegate(const UObject* Source);
	
	/**
	 * Unbinds an object from the death delegate.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage Manager Interface")
	void UnbindFromOwnerDiedDelegate(const UObject* Source);
	
	/**
	 * Checks if the owner is dead.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage Manager Interface")
	bool IsDead() const;

	/**
	 * Calculates incoming damage.
	 * 
	 * @param Input			Details about the incoming damage.
	 * @param Output		Results of the damage calculation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, BlueprintNativeEvent, Category = "Damage Manager Interface")
	void CalculateDamage(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const;
	
	/**
	 * Receives damage that was received by this component's owner and must be handled.
	 *
	 * Usually called by the Attribute Set managing the incoming damage.
	 * 
	 * @param EffectSpec
	 *		Spec containing all the information regarding the effect that applied the damage.
	 *		It can be further expanded using the appropriate Effect Context proxy.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "Damage Manager Interface")
	void RegisterDamageReceived(const FGameplayEffectSpec& EffectSpec);

	/**
	 * Handles damage that was received by this component's owner.
	 *
	 * Usually called by the Damage List, once a damage entry replicates.
	 * 
	 * @param DamageEntry
	 *		Details about the damage. Provides access to the time when it was received and
	 *		to the Gameplay Effect Spec that was originally used to generate the entry.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage Manager Interface")
	void HandleDamageReceived(const FDamageEntry& DamageEntry);

	/** 
	 * Starts the Death Event. This is where you would stop movement, interrupt abilities, etc.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage Manager Interface")
	void StartDying();

	/**
	 * Finishes the Death Event, handling things like un-possess and destroying actors.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage Manager Interface")
	void FinishDying();
	
};
