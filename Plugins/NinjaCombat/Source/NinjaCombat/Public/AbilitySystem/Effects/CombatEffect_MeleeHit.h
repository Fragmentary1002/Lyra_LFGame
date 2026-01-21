// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CombatEffect_ConsumePoise.h"
#include "CombatEffect_MeleeHit.generated.h"

/**
 * Base Gameplay Effect that can be used for Melee Damage.
 */
UCLASS(Abstract)
class NINJACOMBAT_API UCombatEffect_MeleeHit : public UCombatEffect_ConsumePoise
{
	GENERATED_BODY()

public:
	
	UCombatEffect_MeleeHit();
	
};
