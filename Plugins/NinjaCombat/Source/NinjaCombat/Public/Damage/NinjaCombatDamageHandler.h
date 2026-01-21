// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaCombatGameplayCueManager.h"
#include "UObject/Object.h"
#include "NinjaCombatDamageHandler.generated.h"

class UNinjaCombatManagerComponent;
class UNinjaCombatDamageManagerComponent;

/**
 * Base implementation for a Damage Handler.
 */
UCLASS(Abstract, Const, Blueprintable, BlueprintType, DefaultToInstanced, EditInlineNew)
class NINJACOMBAT_API UNinjaCombatDamageHandler : public UObject
{
	
	GENERATED_BODY()

public:
	
	/**
	 * Handles a cosmetic aspect of incoming damage.
	 *
	 * @param DamageManager		Damage Manager that received the damage.
	 * @param Parameters		Gameplay Cue Parameters containing the damage information.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Damage Handler")
	void HandleDamage(UNinjaCombatDamageManagerComponent* DamageManager, const FGameplayCueParameters& Parameters) const;

protected:

	/** Evaluates if the Cue Parameters Struct contains a melee damage. */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Damage Handler")
	static bool IsMeleeDamage(const FGameplayCueParameters& Parameters);

	/** Evaluates if the Cue Parameters Struct contains a ranged damage. */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Damage Handler")
	static bool IsRangedDamage(const FGameplayCueParameters& Parameters);

	/** Evaluates of the damage source is authoritative. */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Damage Handler")
	static bool IsSourceAuthoritative(const FGameplayCueParameters& Parameters);

	/** Evaluates of the damage source is locally Controlled. */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Damage Handler")
	static bool IsSourceLocallyController(const FGameplayCueParameters& Parameters);

	/** Evaluates of the damage target is authoritative. */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Damage Handler")
	static bool IsTargetAuthoritative(const FGameplayCueParameters& Parameters);

	/** Evaluates of the damage target is locally Controlled. */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Damage Handler")
	static bool IsTargetLocallyController(const FGameplayCueParameters& Parameters);
	
};
