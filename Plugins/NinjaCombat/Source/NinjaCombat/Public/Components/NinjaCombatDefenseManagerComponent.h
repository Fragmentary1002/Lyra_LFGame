// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "GameplayTagContainer.h"
#include "NinjaCombatBaseComponent.h"
#include "Interfaces/Components/CombatDefenseManagerInterface.h"
#include "Types/FDamageCalculation.h"
#include "NinjaCombatDefenseManagerComponent.generated.h"

class UAbilitySystemComponent;

/**
 * Manages defense systems available to the owner.
 */
UCLASS(Blueprintable, ClassGroup = NinjaCombat, meta = (BlueprintSpawnableComponent))
class NINJACOMBAT_API UNinjaCombatDefenseManagerComponent : public UNinjaCombatBaseComponent, public ICombatDefenseManagerInterface
{
	
	GENERATED_BODY()

public:

	UNinjaCombatDefenseManagerComponent();

	// -- Begin Actor Component implementation
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// -- End Actor Component implementation
	
	// -- Begin Defense Manager implementation
	virtual void BindToBlockingStateChangedDelegate_Implementation(const FBlockingStateChangedDelegate& Delegate) override;
	virtual void BindToInvulnerabilityStateChangedDelegate_Implementation(const FInvulnerabilityStateChangedDelegate& Delegate) override;
	virtual void UnbindFromBlockingStateChangedDelegate_Implementation(const UObject* Source) override;
	virtual void UnbindFromInvulnerabilityStateChangedDelegate_Implementation(const UObject* Source) override;
	virtual bool IsBlocking_Implementation() const override;
	virtual bool CanBlock_Implementation(const AActor* DamageInstigator) const override;
	virtual bool IsInvulnerable_Implementation() const override;
	virtual bool CheckLastStand_Implementation() const override;
	virtual void CommitLastStand_Implementation() override;
	virtual void DefendDamage_Implementation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const override;
	// -- End Defense Manager implementation

	// -- Begin Base Component implementation
	virtual void OnAbilitySystemComponentReceived() override;
	virtual void OnAbilitySystemComponentReset() override;
	// -- End Base Component implementation
	
protected:

	/** Broadcasts information about the blocking state. */
    UPROPERTY(BlueprintAssignable, Category = "Defense Manager")
    FBlockingStateChangedMCDelegate OnBlockingStateChanged;

    /** Broadcasts information about the invulnerability state. */
    UPROPERTY(BlueprintAssignable, Category = "Defense Manager")
	FInvulnerabilityStateChangedMCDelegate OnInvulnerabilityStateChanged;

	/** The Gameplay Attribute used for Block Angle.  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense Manager", NoClear)
	FGameplayAttribute BlockAngleAttribute;

	/** The Gameplay Attribute used for the Last Stand Count.  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense Manager", NoClear)
	FGameplayAttribute LastStandCountAttribute;

	/** The Gameplay Attribute used for the Last Stand Health.  */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense Manager", NoClear)
	FGameplayAttribute LastStandHealthPercentAttribute;

	/** Applies the effects of a Last Stand. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Defense Manager", NoClear)
	TSubclassOf<UGameplayEffect> LastStandEffectClass;
	
	/** Reacts to a change in the tracked tags. */
	virtual void HandleTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	/**
	 * Allows an external aspect such as the Game Mode or Game State to modify the damage.
	 * This is done by invoking the ICombatDamageModifierInterface.
	 */
	virtual void ModifyIncomingDamage(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const;

	/**
	 * Applies the Block Mitigation and store the results.
	 */
	virtual void ApplyBlockMitigation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const;

	/**
	 * Applies the Defense Mitigation and store the results.
	 */
	virtual void ApplyDefenseMitigation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const;

	/**
	 * Applies the passive Armor Mitigation and store the results.
	 */
	virtual void ApplyArmorMitigation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const;

	/**
	 * In case of Fatal Damage, evaluates if the character can be saved.
	 * You can also modify any other values as needed, such as setting health to "1", or reverting the "fatal" flag.
	 */
	virtual void ApplyLastStand(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const;
	
	/**
	 * Retrieves the Block Angle used by this component.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Defense Manager")	
	float GetBlockAngle() const;
	
	/**
	 * Reacts to the replication of the "Blocking" property.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Defense Manager")
	void OnRep_IsBlocking(bool bWasBlocking);

	/**
	 * Reacts to the replication of the "Blocking" property.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Defense Manager")
	void OnRep_IsInvulnerable(bool bWasInvulnerable);
	
private:

	/** Delegate Handle for the Blocking Tag event listener. */
	FDelegateHandle BlockingTagHandle;

	/** Delegate Handle for the Invulnerability Tag event listener. */
	FDelegateHandle InvulnerabilityTagHandle;
	
	/** Informs if the owning actor is blocking. */
	UPROPERTY(ReplicatedUsing = OnRep_IsBlocking)
	uint8 bIsBlocking:1;

	/** Informs if the owning actor is invulnerable. */
	UPROPERTY(ReplicatedUsing = OnRep_IsInvulnerable)
	uint8 bIsInvulnerable:1;
	
};

