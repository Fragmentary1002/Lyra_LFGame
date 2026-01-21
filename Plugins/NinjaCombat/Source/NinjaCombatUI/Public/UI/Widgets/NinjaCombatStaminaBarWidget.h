// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaCombatBaseOverheadBarWidget.h"
#include "NinjaCombatStaminaBarWidget.generated.h"

/**
 * Base widget class implementing a stamina bar. 
 */
UCLASS(Abstract)
class NINJACOMBATUI_API UNinjaCombatStaminaBarWidget : public UNinjaCombatBaseOverheadBarWidget
{
	GENERATED_BODY()

public:

	UNinjaCombatStaminaBarWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
};

