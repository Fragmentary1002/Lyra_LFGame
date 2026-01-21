// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaCombatBaseComponent.h"
#include "NinjaCombatDelegates.h"
#include "Interfaces/Components/CombatTargetManagerInterface.h"
#include "NinjaCombatTargetManagerComponent.generated.h"

/**
 * Tracks combat targets, implementing the appropriate interface.
 */
UCLASS(Blueprintable, ClassGroup = NinjaCombat, meta = (BlueprintSpawnableComponent))
class NINJACOMBAT_API UNinjaCombatTargetManagerComponent : public UNinjaCombatBaseComponent, public ICombatTargetManagerInterface
{
	
	GENERATED_BODY()

public:

	UNinjaCombatTargetManagerComponent();

	// -- Begin Actor Component implementation
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// -- End Actor Component implementation
	
	// -- Begin Target Manager implementation
	virtual void BindToCombatTargetChangedDelegate_Implementation(const FCombatTargetChangedDelegate& Delegate) override;
	virtual void UnbindFromTargetChangedDelegate_Implementation(const UObject* Source) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	virtual void SetCombatTarget_Implementation(AActor* NewCombatTarget) override;
	virtual void ReceiveTargetingSource_Implementation(AActor* TargetingSource) override;
	virtual void ClearTargetingSource_Implementation(AActor* TargetingSource) override;
	// -- End Target Manager implementation

protected:

	/** Notifies a change in the combat target. */
	UPROPERTY(BlueprintAssignable)
	FCombatTargetChangedMCDelegate OnCombatTargetChanged;

	/**
	 * If set to true, allows locally controlled clients to predict targets, before replication from server.
	 * This will provide an immediate targeting reaction to local player, at the expense of trusting it.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Target Manager")
	bool bCanPredictTarget;

	/**
	 * Reacts to the replication of a combat target.
	 * 
	 * @param OldCombatTarget
	 *		Old combat target, before the replication occured.
	 */
	UFUNCTION()
	void OnRep_CombatTarget(AActor* OldCombatTarget);

	/**
	 * Triggers an RPC to set a new Combat Target in the authority.
	 * 
	 * @param NewCombatTarget
	 *		New combat target to be set in the authority.
	 */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetCombatTarget(AActor* NewCombatTarget);
	
private:

	/** Current combat target, replicated across all actors. */
	UPROPERTY(ReplicatedUsing = OnRep_CombatTarget)
	TObjectPtr<AActor> CombatTarget;
	
};
