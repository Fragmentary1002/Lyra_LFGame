// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CombatEffect_ReplenishPoise.generated.h"

/**
 * Replenishes Poise based on the backing attribute.
 * By default, poise is replenished straight back to its maximum value.
 */
UCLASS()
class NINJACOMBAT_API UCombatEffect_ReplenishPoise : public UGameplayEffect
{
	
	GENERATED_BODY()

public:

	UCombatEffect_ReplenishPoise();
	
};
