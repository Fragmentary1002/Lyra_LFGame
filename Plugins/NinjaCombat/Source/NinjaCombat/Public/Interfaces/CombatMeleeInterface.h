// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "UObject/Interface.h"
#include "CombatMeleeInterface.generated.h"

class UMeshComponent;
class UNiagaraComponent;

UINTERFACE()
class UCombatMeleeInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Defines basic methods for a source of melee damage.
 */
class NINJACOMBAT_API ICombatMeleeInterface
{
	
	GENERATED_BODY()

public:

	/**
	 * Provides the main mesh component that represents the melee attacker.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee Interface")
	UMeshComponent* GetMeleeMesh() const;

	/**
	 * Provides the Gameplay Effect applied when the source hits a target.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee Interface")
	TSubclassOf<UGameplayEffect> GetHitEffectClass() const;
	virtual TSubclassOf<UGameplayEffect> GetHitEffectClass_Implementation() const;

	/**
	 * Provides the Gameplay Effect level used when applying this interface's Effect Class.
	 * Returning zero or lower will nullify the outcome of this method.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee Interface")
	float GetHitEffectLevel() const;
	virtual float GetHitEffectLevel_Implementation() const;

	/**
	 * Allows a damage value to be optionally provided by a Weapon Implementation.
	 * If no damage value is to be provided, then this function will return false.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee Interface")
	bool GetDamage(float& OutDamage) const;
	virtual bool GetDamage_Implementation(float& OutDamage) const;

	/**
	 * Allows a poise consumption value to be optionally provided by a Weapon Implementation.
	 * If no poise consumption value is to be provided, then this function will return false.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee Interface")
	bool GetPoiseConsumption(float& OutPoiseConsumption) const;
	virtual bool GetPoiseConsumption_Implementation(float& OutPoiseConsumption) const;
	
	/**
	 * Provides a Niagara System used as the Attack Trail.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee Interface")
	TArray<UNiagaraComponent*> GetAttackTrails() const;
	virtual TArray<UNiagaraComponent*> GetAttackTrails_Implementation() const;

	/**
	 * Provides the strength of the impact, for physical animations. 
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee Interface")
	float GetMeleeImpactStrength() const;
	virtual float GetMeleeImpactStrength_Implementation() const;
	
	/**
	 * Provides a Niagara System used as the Impact Trail.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee Interface")
	TArray<UNiagaraComponent*> GetImpactTrails() const;
	virtual TArray<UNiagaraComponent*> GetImpactTrails_Implementation() const;

	/**
	 * Allows the weapon to react to melee damage that has been applied to a target.
	 *
	 * @param HitResult
	 *		Information about the Hit that generated this damage.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Melee Interface")
	void HandleMeleeDamageCosmetics(const FHitResult& HitResult) const;
	virtual void HandleMeleeDamageCosmetics_Implementation(const FHitResult& HitResult) const { }	
	
};
