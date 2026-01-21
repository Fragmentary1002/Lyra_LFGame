// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaCombatBaseOverheadBarWidget.h"
#include "NinjaCombatHealthBarWidget.generated.h"

/**
 * Base widget class implementing a health bar.
 */
UCLASS(Abstract)
class NINJACOMBATUI_API UNinjaCombatHealthBarWidget : public UNinjaCombatBaseOverheadBarWidget
{
	GENERATED_BODY()

public:

	UNinjaCombatHealthBarWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
};