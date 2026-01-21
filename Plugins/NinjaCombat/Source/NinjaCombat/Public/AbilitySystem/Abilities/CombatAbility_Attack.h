// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaCombatMontageAbility.h"
#include "CombatAbility_Attack.generated.h"

class UNinjaCombatProjectileRequest;
class UNinjaCombatMeleeScan;
class UAbilityTask_WaitGameplayEvent;
class UAbilityTask_ScanMeleeTarget;
class UAbilityTask_SpawnProjectile;

/**
 * Handles a Melee or Ranged attack.
 *
 * An attack is very flexible and can both perform a Melee Scan and launch Projectiles. It will
 * listen to events and react accordingly for each scenario. These events are usually triggered
 * by animations, using the appropriate Notifies: "Melee Scan" and "Launch Projectile".
 */
UCLASS()
class NINJACOMBAT_API UCombatAbility_Attack : public UNinjaCombatMontageAbility
{
	
	GENERATED_BODY()

public:
	
	UCombatAbility_Attack(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * Provides the current combo counter that can be added to damage effects.
	 * 
	 * @return
	 *		The Combo Counter currently stored in the combo component.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	int32 GetComboCounter() const;
	
protected:

	// -- Melee Attack --------------------------------------------------------
	
	/** Enables or disables the Melee Attack aspect of this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Attack")
	bool bIsMeleeAttack;

	/**
	 * Gameplay effect applied when the target is hit by a melee attack.
	 * The causer can implement the Melee interface and provide a Gameplay Effect that will override this.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Attack", meta = (EditCondition = "bIsMeleeAttack", EditConditionHides))
	TSubclassOf<UGameplayEffect> MeleeDamageEffectClass;

	/**
	 * Level applied to the Melee Damage Effect.
	 * If you want more flexibility instead of this flag value, please check "GetMeleeGameplayEffectLevel".
	 * Also, the causer can implement the Melee interface and provide a level to be used instead.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Attack", meta = (EditCondition = "bIsMeleeAttack", EditConditionHides))
	float DefaultMeleeEffectLevel;
	
	// -- Ranged Attack -------------------------------------------------------
	
	/** Enables or disables the Ranged Attack aspect of this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Attack")
	bool bIsRangedAttack;

	/**
	 * Actor class for the projectile being spawned.
	 * Classes provided in the event payload can override this value.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Attack", meta = (EditCondition = "bIsRangedAttack", EditConditionHides, MustImplement = "/Script/NinjaCombat.CombatProjectileInterface"))
	TSubclassOf<AActor> ProjectileClass;

	/**
	 * Gameplay effect applied when the target is hit by projectile.
	 * The causer can implement the Projectile interface and provide a Gameplay Effect that will override this.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Attack", meta = (EditCondition = "bIsRangedAttack", EditConditionHides))
	TSubclassOf<UGameplayEffect> ProjectileDamageEffectClass;
	
	/**
	 * Level applied to the Ranged Damage Effect.
	 * If you want more flexibility instead of this flag value, please check "GetRangedGameplayEffectLevel".
	 * Also, the causer can implement the Projectile interface and provide a level to be used instead.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Attack", meta = (EditCondition = "bIsRangedAttack", EditConditionHides))
	float DefaultProjectileEffectLevel;
	
	// -- Begin Gameplay Ability implementation
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void HandleEventReceived_Implementation(FGameplayEventData Payload) override;
	// -- End Gameplay Ability implementation

	/**
	 * Handles the initialization of the melee scan task. It won't be active yet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	UAbilityTask_ScanMeleeTarget* InitializeMeleeScanTask(const UNinjaCombatMeleeScan* MeleeScan);

	/**
	 * Handles the initialization of the spawn projectile task. It won't be active yet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	UAbilityTask_SpawnProjectile* InitializeProjectileTask(const UNinjaCombatProjectileRequest* ProjectileRequest);

	/**
	 * Handles targets received from a melee scan.
	 * 
	 * @param MeleeScan		Information about the scan that generated hits.
	 * @param Data			Target data received from the scan.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Attack Ability")
	void HandleMeleeScanTargetsReceived(const UNinjaCombatMeleeScan* MeleeScan, const FGameplayAbilityTargetDataHandle& Data);

	/**
	 * Handles a projectile created by the combat system.
	 * 
	 * @param Request		Request that originated the projectile.
	 * @param Projectile	Actor implementing the Projectile interface.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Attack Ability")
	void HandleProjectileLaunched(const UNinjaCombatProjectileRequest* Request, AActor* Projectile);

	/**
	 * Provides the Gameplay Effect class to be applied by this ability, on successful melee attacks.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Attack Ability")
	TSubclassOf<UGameplayEffect> GetMeleeGameplayEffectClass(const AActor* EffectCauser) const;
	
	/**
	 * Provides the Gameplay Effect level to be applied by this ability, on successful melee attacks.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Attack Ability")
	float GetMeleeGameplayEffectLevel(const AActor* EffectCauser) const;

	/**
	 * Provides the Gameplay Effect class to be applied by this ability, on successful ranged impacts.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Attack Ability")
	TSubclassOf<UGameplayEffect> GetProjectileGameplayEffectClass(const AActor* Projectile) const;
	
	/**
	 * Provides the Gameplay Effect level to be applied by this ability, on successful ranged impacts.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Attack Ability")
	float GetProjectileGameplayEffectLevel(const AActor* Projectile) const;
	
	/**
	 * Creates an effect spec for a melee hit.
	 *
	 * @param MeleeScan		Source that is responsible for the hit.
	 * @param OutHandle		The handle generated by the method.
	 * @return				True if a valid handle was created.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	virtual bool CreateSpecHandleForMeleeHit(const UNinjaCombatMeleeScan* MeleeScan,
		FGameplayEffectSpecHandle& OutHandle) const;

	/**
	 * Applies a Gameplay Effect Spec to an actor available in a Hit Result.
	 *
	 * @param SpecHandle		Preprocessed gameplay effect to be applied.
	 * @param HitResult			Hit Result containing the target.
	 * @return					An array of handles representing the outcome.
	 */
	TArray<FActiveGameplayEffectHandle> ApplyGameplayEffectSpecToHitResult(
		const FGameplayEffectSpecHandle& SpecHandle, const FHitResult& HitResult);

	/**
	 * Applies additional (Caller By Magnitudes) data to a Gameplay Effect Handle Spec.
	 *
	 * @param DamageSource	Actor who's inflicting the damage.
	 * @param SpecHandle	Spec Handle that allows modification of the effect.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	virtual void ApplyDamageValues(const AActor* DamageSource, FGameplayEffectSpecHandle& SpecHandle) const;

	/**
	 * Retrieves the Projectile Request from the payload.
	 */
	virtual UNinjaCombatProjectileRequest* GetProjectileRequest(const FGameplayEventData& Payload) const;
	
private:

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> MeleeScanEventTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> LaunchProjectileEventTask;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_ScanMeleeTarget> MeleeScanTask;

	UPROPERTY()
	TObjectPtr<UAbilityTask_SpawnProjectile> SpawnProjectileTask;
	
};
