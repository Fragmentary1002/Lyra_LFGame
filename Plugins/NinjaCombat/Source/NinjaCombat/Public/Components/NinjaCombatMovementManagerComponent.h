// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaCombatBaseComponent.h"
#include "Interfaces/Components/CombatMovementManagerInterface.h"
#include "NinjaCombatMovementManagerComponent.generated.h"

class UCharacterMovementComponent;
/**
 * Reacts to combat events and perform any necessary logic in the owner's Movement Component.
 */
UCLASS(Blueprintable, ClassGroup = NinjaCombat, meta = (BlueprintSpawnableComponent))
class NINJACOMBAT_API UNinjaCombatMovementManagerComponent : public UNinjaCombatBaseComponent, public ICombatMovementManagerInterface
{
	
	GENERATED_BODY()

public:

	UNinjaCombatMovementManagerComponent();

	// -- Begin Actor Component implementation
	virtual void BeginPlay() override;
	// -- End Actor Component implementation

	// -- Begin Combat Movement Interface
	virtual void HandleCombatTargetChanged_Implementation(const AActor* CombatTarget, const AActor* OldCombatTarget) override;
	virtual void HandleBlockingStateChanged_Implementation(bool bNewIsBlocking) override;
	// -- End Combat Movement Interface

	/**
	 * Sets the blocking in the backing Movement System.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Movement Manager")
	void SetBlocking(bool bNewIsBlocking);

	/**
	 * Sets the strafing in the backing Movement System.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Movement Manager")
	void SetStrafing(bool bNewStrafing);
	
protected:
	
	/**
	 * Defines if the owner will strafe while locked on a target.
	 *
	 * By default, this leads to an adjustment in the owner's Character Movement Component's flags
	 * expected for such behavior: "bOrientRotationToMovement" and "bUseControllerDesiredRotation".
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Manager|Strafing")
	bool bStrafeWhileLockedOnTarget;

	/**
	 * Defines if the owner will walk while blocking.
	 *
	 * By default, this leads to a change in the owner's Character Movement Component's walk speed,
	 * walk acceleration and walk deceleration. All these values are restored to their original
	 * values when the block state stops.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Manager|Blocking")
	bool bWalkWhileBlocking;

	/** Blocking walk speed, obtained from the Movement Component. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Manager|Blocking", meta = (EditCondition = "bWalkWhileBlocking"))
	float BlockingWalkSpeed;

	/** Blocking acceleration, obtained from the Movement Component. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Manager|Blocking", meta = (EditCondition = "bWalkWhileBlocking"))
	float BlockingAcceleration;

	/** Blocking deceleration, obtained from the Movement Component. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement Manager|Blocking", meta = (EditCondition = "bWalkWhileBlocking"))
	float BlockingDeceleration;
	
	/**
	 * Binds to any relevant delegates in the Combat System.
	 */
	virtual void BindToCombatSystem();
	
	/**
	 * Opportunity to backup any values from the Owner Character Movement, as it's retrieved.
	 */
	virtual void BackupMovementValues(const UCharacterMovementComponent* OwnerCharacterMovement);
	
	/**
	 * Provides the cached Character Movement Component from this component's owner.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Movement Manager")
	UCharacterMovementComponent* GetCharacterMovementComponent() const;
	
	/**
	 * Provides the default walk speed, before any changes (like "blocking"). 
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Movement Manager")
	FORCEINLINE float GetDefaultWalkSpeed() const { return DefaultWalkSpeed; }

	/**
	 * Provides the default acceleration, before any changes (like "blocking"). 
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Movement Manager")
	FORCEINLINE float GetDefaultAcceleration() const { return DefaultAcceleration; }

	/**
	 * Provides the default deceleration, before any changes (like "blocking"). 
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Movement Manager")
	FORCEINLINE float GetDefaultDeceleration() const { return DefaultDeceleration; }
	
private:

	/** Internal flag to inform a blocking state. */
	bool bIsBlocking;

	/** Internal flag to inform a strafing state. */
	bool bIsStrafing;
	
	/** Default walk speed, obtained from the Movement Component. */
	float DefaultWalkSpeed;

	/** Default acceleration, obtained from the Movement Component. */
	float DefaultAcceleration;

	/** Default deceleration, obtained from the Movement Component. */
	float DefaultDeceleration;
	
	/** A weak pointer to the owner's Character Movement. Use the appropriate getter. */
	UPROPERTY()
	TWeakObjectPtr<UCharacterMovementComponent> OwnerCharacterMovementPtr;
	
};
