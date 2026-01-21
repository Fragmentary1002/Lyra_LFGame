// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "CombatEffect_ConsumePoise.h"
#include "CombatEffect_RangedHit.generated.h"

/**
 * Base Gameplay Effect that can be used for Ranged Damage.
 */
UCLASS(Abstract)
class NINJACOMBAT_API UCombatEffect_RangedHit : public UCombatEffect_ConsumePoise
{

	GENERATED_BODY()

public:
	
	UCombatEffect_RangedHit();
	
};
