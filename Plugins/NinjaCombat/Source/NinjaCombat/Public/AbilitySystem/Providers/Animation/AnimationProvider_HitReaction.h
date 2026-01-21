// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/Providers/NinjaCombatAnimationProvider.h"
#include "AnimationProvider_HitReaction.generated.h"

/** Configuration Class for the Evade Direction. */
USTRUCT(BlueprintType)
struct FHitReactionContext
{
	GENERATED_BODY()

	/** Context for the hit reaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction Context")
	FGameplayTagContainer ContextTags;
	
	/** Context for this Hit Reaction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hit Reaction Context")
	TObjectPtr<UAnimMontage> AnimMontage;
};

/**
 * Provides appropriate animations for different types of damage impacts. 
 */
UCLASS(meta = (DisplayName = "Hit Reactions"))
class NINJACOMBAT_API UAnimationProvider_HitReaction : public UNinjaCombatAnimationProvider
{
	
	GENERATED_BODY()

public:

	// -- Begin Animation Provider implementation
	virtual UAnimMontage* GetMontageToPlay_Implementation(UNinjaCombatGameplayAbility* CombatAbility) const override;
	// -- End Animation Provider implementation
	
protected:
	
	/**
	 * Maps specific types of damage to certain hit reactions.
	 * If no mapping is found, then the default animation montage is used.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Provider", meta = (TitleProperty = "AnimMontage"))
	TArray<FHitReactionContext> HitReactionContexts;

	/**
	 * Evaluates if a given context applies to a hit reaction.
	 *
	 * By default, this is a "HasAnyExact" check. However, you can extend this function to create
	 * other types of filtering, such as using Gameplay Tag Queries and so on.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Hit Reaction Animation Provider")
	bool Applies(const FGameplayTagContainer& AbilityContext, const FHitReactionContext& HitReactionContext) const;
	
};
