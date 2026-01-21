// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NinjaCombatDelegates.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FCombatTargetChangedDelegate, const AActor*, NewCombatTarget, const AActor*, OldCombatTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatTargetChangedMCDelegate, const AActor*, NewCombatTarget, const AActor*, OldCombatTarget);

DECLARE_DYNAMIC_DELEGATE_OneParam(FBlockingStateChangedDelegate, bool, bIsBlocking);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlockingStateChangedMCDelegate, bool, bIsBlocking);

DECLARE_DYNAMIC_DELEGATE_OneParam(FInvulnerabilityStateChangedDelegate, bool, bIsInvulnerable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FInvulnerabilityStateChangedMCDelegate, bool, bIsInvulnerable);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FComboWindowChangedDelegate, const UNinjaCombatComboSetupData*, ComboData, bool, bInComboWindow);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FComboWindowChangedMCDelegate, const UNinjaCombatComboSetupData*, ComboData, bool, bInComboWindow);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FComboFinishedDelegate, const UNinjaCombatComboSetupData*, ComboData, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FComboFinishedMCDelegate, const UNinjaCombatComboSetupData*, ComboData, bool, bWasSuccessful);

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDamageReceivedDelegate, AActor*, Causer, AActor*, Target, const FDamageEntry&, DamageEntry);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FDamageReceivedMCDelegate, AActor*, Causer, AActor*, Target, const FDamageEntry&, DamageEntry);

DECLARE_DYNAMIC_DELEGATE(FOwnerDiedDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOwnerDiedMCDelegate);

UCLASS()
class NINJACOMBAT_API UNinjaCombatDelegates : public UObject
{
	GENERATED_BODY()
};
