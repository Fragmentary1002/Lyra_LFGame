// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "CombatAnimationContextProviderInterface.generated.h"

UINTERFACE(MinimalAPI)
class UCombatAnimationContextProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Defines an object that can provide a context for an animation.
 */
class NINJACOMBAT_API ICombatAnimationContextProviderInterface
{
	
	GENERATED_BODY()

public:

	/**
	 * Provides the context for an animation, via Gameplay Tags.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Animation Context Provider Interface")
	void GetAnimationContext(FGameplayTagContainer& OutContextTags) const;

};