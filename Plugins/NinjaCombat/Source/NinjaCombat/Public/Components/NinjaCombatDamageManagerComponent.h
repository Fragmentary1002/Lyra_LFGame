// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayCueInterface.h"
#include "GameplayTagContainer.h"
#include "NinjaCombatBaseComponent.h"
#include "NinjaCombatDelegates.h"
#include "Interfaces/Components/CombatDamageManagerInterface.h"
#include "Types/FDamageList.h"
#include "NinjaCombatDamageManagerComponent.generated.h"

class UNinjaCombatDamageHandlerData;

/**
 * Manages damage applied to the owner.
 */
UCLASS(Blueprintable, ClassGroup = NinjaCombat, meta = (BlueprintSpawnableComponent))
class NINJACOMBAT_API UNinjaCombatDamageManagerComponent : public UNinjaCombatBaseComponent, public IGameplayCueInterface,
	public ICombatDamageManagerInterface
{

	GENERATED_BODY()
	
public:

	UNinjaCombatDamageManagerComponent();

	// -- Begin Actor Component implementation
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// -- End Actor Component implementation

	// -- Begin Damage Manager implementation
	virtual bool IsDead_Implementation() const override;
	virtual void BindToDamageReceivedDelegate_Implementation(const FDamageReceivedDelegate& Delegate) override;
	virtual void BindToOwnerDiedDelegate_Implementation(const FOwnerDiedDelegate& Delegate) override;
	virtual void UnbindFromDamageReceivedDelegate_Implementation(const UObject* Source) override;
	virtual void UnbindFromOwnerDiedDelegate_Implementation(const UObject* Source) override;
	virtual void CalculateDamage_Implementation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const override;
	virtual void RegisterDamageReceived_Implementation(const FGameplayEffectSpec& EffectSpec) override;
	virtual void HandleDamageReceived_Implementation(const FDamageEntry& DamageEntry) override;
	virtual void StartDying_Implementation() override;
	virtual void FinishDying_Implementation() override;
	// -- End Damage Manager implementation

	// -- Begin Base Component implementation
	virtual void OnAbilitySystemComponentReceived() override;
	virtual void OnAbilitySystemComponentReset() override;
	// -- End Base Component implementation
	
	/**
	 * Handles The Hit Gameplay Cue defined by the "GameplayCue.Combat.Hit" Tag.
	 * Routing to this method happens through the Gameplay Cue Interface.
	 */	
	UFUNCTION(BlueprintNativeEvent, Category = "Damage Manager", DisplayName = "Handle Hit Gameplay Cue")
	void GameplayCue_Combat_Hit(EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters);

	/**
	 * Handles The Hit Gameplay Cue defined by the "GameplayCue.Combat.Death" Tag.
	 * Routing to this method happens through the Gameplay Cue Interface.
	 */	
	UFUNCTION(BlueprintNativeEvent, Category = "Damage Manager", DisplayName = "Handle Death Gameplay Cue")
	void GameplayCue_Combat_Death(EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters);	
	
protected:

	/**
	 * Broadcasts information about damage applied to the owner.
	 * 
	 * The Struct has a dedicated native breaker that should be transparent for Blueprints but C++ users
	 * can also benefit from it. It's available in as UNinjaCombatFunctionLibrary::BreakDamageStruct. 
	 */
	UPROPERTY(BlueprintAssignable, Category = "Damage Manager")
	FDamageReceivedMCDelegate OnDamageReceived;

	/**
	 * Broadcasts a change in the current Dead State of the owning character. 
	 */
	UPROPERTY(BlueprintAssignable, Category = "Damage Manager")
	FOwnerDiedMCDelegate OnOwnerDeath;

	/** All handlers available to this damage manager. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage Manager")
	TObjectPtr<UNinjaCombatDamageHandlerData> DamageHandlerData;
	
	/** Executes all damage handlers assigned to this manager. */
	void ExecuteDamageHandlers(const FGameplayCueParameters& Parameters);
	
	/** Reacts to a change in the tracked tags. */
	virtual void HandleTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	/** Triggers the Gameplay Event notifying the damage. */
	virtual void BroadcastDamageGameplayEvent(const FDamageEntry& DamageEntry);
	
	/** Plays a Gameplay Cue related to a damage event. */
	virtual void PlayDamageGameplayCue(const FDamageEntry& DamageEntry);

	/** Retrieves the instance of the Death Ability being used. */
	virtual UGameplayAbility* GetDeathAbility() const;
	
	/**
	 * Reacts to the replication of the "Dead" property.
	 * 
	 * Connect any events that should react to this, but keep in mind that both the damage and death 
	 * delegates can be used to notify external classes about this event.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Damage Manager")
	void OnRep_IsDead();

private:

	/** Delegate Handle for the Death Tag event listener. */
	FDelegateHandle DeathTagHandle;

	/** Informs if the owning actor has died. */
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	uint8 bIsDead:1;

	/** List of recent damage received by this component. */
	UPROPERTY(Replicated)
	FDamageList DamageTakenList;

};

