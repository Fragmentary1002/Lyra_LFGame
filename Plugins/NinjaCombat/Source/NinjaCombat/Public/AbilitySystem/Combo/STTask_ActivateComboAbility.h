// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Blueprint/StateTreeTaskBlueprintBase.h"
#include "STTask_ActivateComboAbility.generated.h"

/** Possible changes that this task will do to the combo change on activation. */
UENUM(BlueprintType)
enum class EComboCountChange : uint8
{
	/** The count won't be changed. Something else will have to do it, like a successful hit detection. */
	NoChange,

	/** Increments the count, moving the Combo to the next state, ready for the next input. */
	IncrementCount,

	/** Resets the count, allowing the combo to loop from the start. */
	ResetCount,
};

/**
 * Activates a Gameplay Ability based on a given criteria.
 */
UCLASS(DisplayName = "Activate Combo Ability", Category = "Combat")
class NINJACOMBAT_API USTTask_ActivateComboAbility : public UStateTreeTaskBlueprintBase
{
	
	GENERATED_BODY()

public:

	USTTask_ActivateComboAbility(const FObjectInitializer& ObjectInitializer);

	/** Gameplay Tags used to activate the ability. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameter)
	FGameplayTagContainer AbilityTags;

	/** How to modify the Combo Count if there's an activation. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Parameter)
	EComboCountChange ChangeOnActivation;
	
protected:

	/** Delegate handle for our ability execution. */
	FDelegateHandle AbilityCallbackDelegateHandle;
	
	// -- Begin State Tree Task implementation
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) override;
	// -- End State Tree Task implementation

	/**
	 * Obtains the Combo Component from the Owner and modifies the Combo Count as necessary.
	 */
	void UpdateComboCount(const AActor* Owner) const;
	
	/**
	 * Notifies that the ability being executed has finished.
	 */
	UFUNCTION()
	void OnAbilityEnded(const FAbilityEndedData& Data, const AActor* Owner);
	
};
