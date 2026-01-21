// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaCombatMontageAbility.h"
#include "Interfaces/Animation/CombatAnimationContextProviderInterface.h"
#include "CombatAbility_HitReaction.generated.h"

/**
 * Handles Hit Reactions on the Avatar.
 */
UCLASS()
class NINJACOMBAT_API UCombatAbility_HitReaction : public UNinjaCombatMontageAbility, public ICombatAnimationContextProviderInterface
{
	
	GENERATED_BODY()

public:

	UCombatAbility_HitReaction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Animation Provider Interface
	virtual void GetAnimationContext_Implementation(FGameplayTagContainer& OutContextTags) const override;
	// -- End Animation Provider Interface
	
protected:

	/** Is set to true, makes the avatar rotate to the damage source. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Hit Reaction")
	bool bRotateToSource;

	/**
	 * All filter tags used on the incoming instigator container.
	 * These are used to build the context for the Animation Provider.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Hit Reaction")
	FGameplayTagContainer InstigatorTagsFilter;

	/**
	 * All filter tags used on the incoming target container.
	 * These are used to build the context for the Animation Provider.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Hit Reaction")
	FGameplayTagContainer TargetTagsFilter;
	
	// -- Begin Gameplay Ability implementation
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	// -- End Gameplay Ability implementation	

	/**
	 * Handles the hit.
	 *
	 * You may build any custom behavior for your Hit Reactions in this method, without the
	 * need to worry about keeping the original behavior when the ability activates.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Hit Reaction Ability")
	void ExecuteHitReaction(const FGameplayEventData& TriggerEventData);
	
	/**
	 * Rotates the avatar to the event's instigator.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Hit Reaction Ability")
	void RotateToInstigator(const FGameplayEventData& TriggerEventData);

	/**
	 * Collects and adds all relevant tags into the context.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Hit Reaction Ability")
	void CollectContext(const FGameplayEventData& TriggerEventData, FGameplayTagContainer& Context) const;
	
private:

	FGameplayTagContainer AnimationContext;
	
};
