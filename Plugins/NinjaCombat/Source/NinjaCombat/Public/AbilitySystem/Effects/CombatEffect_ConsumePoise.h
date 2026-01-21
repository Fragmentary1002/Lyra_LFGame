// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CombatEffect_ConsumePoise.generated.h"

class UTargetTagsGameplayEffectComponent;

/**
 * Consumes Poise by a certain amount and blocks poise replenishment for a moment.
 */
UCLASS()
class NINJACOMBAT_API UCombatEffect_ConsumePoise : public UGameplayEffect
{
	
	GENERATED_BODY()

public:

	UCombatEffect_ConsumePoise();

};

