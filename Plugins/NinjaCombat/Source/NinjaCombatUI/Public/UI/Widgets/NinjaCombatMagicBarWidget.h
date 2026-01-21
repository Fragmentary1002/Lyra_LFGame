// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaCombatBaseOverheadBarWidget.h"
#include "NinjaCombatMagicBarWidget.generated.h"

/**
 * Base widget class implementing a magic bar.
 */
UCLASS(Abstract)
class NINJACOMBATUI_API UNinjaCombatMagicBarWidget : public UNinjaCombatBaseOverheadBarWidget
{
	GENERATED_BODY()

public:

	UNinjaCombatMagicBarWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
};