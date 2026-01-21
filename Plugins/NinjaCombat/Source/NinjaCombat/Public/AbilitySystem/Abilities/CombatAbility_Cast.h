// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaCombatMontageAbility.h"
#include "Types/TargetingSystemTypes.h"
#include "CombatAbility_Cast.generated.h"

class UAbilityTask_PerformTargeting;
class UAbilityTask_SpawnCast;
class UAbilityTask_WaitGameplayEvent;
class UTargetingPreset;

/**
 * Types of targets supported by the cast ability.
 */
UENUM(BlueprintType)
enum class ECombatCastTargeting : uint8
{
	/** The cast effect will be applied to all targets collected via the Targeting System. */
	TargetingSystem,

	/** The cast effect will be applied to all collected targets collected by a Spawned Actor. */
	SpawnActor,
};

/**
 * Handles a cast, something like a Buff, Incantation or Area of Effect.
 */
UCLASS()
class NINJACOMBAT_API UCombatAbility_Cast : public UNinjaCombatMontageAbility
{
	
	GENERATED_BODY()

public:
	
	UCombatAbility_Cast(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/**
	 * Determines if this cast has an effect that should be applied applied to the caster.
	 * You can also allow a Cast Actor to handle the effect applied to self.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Cast")
	bool bAppliesToSelf;

	/** Effect applied to the caster, if "bAppliesOnSelf" is set to true. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Cast", meta = (EditCondition = "bAppliesToSelf", EditConditionHides))
	TSubclassOf<UGameplayEffect> SelfEffectClass;

	/** Determines if this cast has an effect that should be applied applied to the caster. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Cast")
	bool bAppliesToTargets;

	/** Effect applied to targets, if "bAppliesOnTargets" is set to true. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Cast", meta = (EditCondition = "bAppliesToTargets", EditConditionHides))
	TSubclassOf<UGameplayEffect> TargetEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Cast", meta = (EditCondition = "bAppliesToTargets", EditConditionHides))
	ECombatCastTargeting Targeting;
	
	/** The Targeting Preset used to collect targets for the cast. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Cast", meta = (EditCondition = "bAppliesToTargets && Targeting == ECombatCastTargeting::TargetingSystem", EditConditionHides))
	TObjectPtr<UTargetingPreset> TargetingPreset;

	/** Determines if Targeting is done asynchronously. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Cast", meta = (EditCondition = "bAppliesToTargets && Targeting == ECombatCastTargeting::TargetingSystem", EditConditionHides))
	bool bPerformAsyncTargeting;

	/** The actor spawned and responsible for collecting targets. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Cast", meta = (EditCondition = "bAppliesToTargets && Targeting == ECombatCastTargeting::SpawnActor", EditConditionHides, MustImplement = "/Script/NinjaCombat.CombatCastInterface"))
	TSubclassOf<AActor> TargetingActorClass;
	
	/** A Gameplay Tag for the Cue applied when the cast succeeds. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Cast", meta = (GameplayTagFilter = "GameplayCue"))
	FGameplayTag CosmeticsGameplayCue;
	
	// -- Begin Gameplay Ability implementation
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void HandleEventReceived_Implementation(FGameplayEventData Payload) override;
	// -- End Gameplay Ability implementation

	UFUNCTION(BlueprintNativeEvent, Category = "Cast Ability")
	void CastToSelf();

	UFUNCTION(BlueprintNativeEvent, Category = "Cast Ability")
	void CastToTargets(const TArray<AActor*>& TargetsFound);
	
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	void CollectTargetsFromTargetingSystem();

	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	void SpawnTargetingActor();

	UFUNCTION()
	virtual void OnTargetsReady(FTargetingRequestHandle TargetingRequestHandle);
	
	UFUNCTION()
	virtual void OnCastActorReady(AActor* SpawnedActor);

	virtual FTransform GetCastLocation() const;
	
private:

	/** Keeps track of a cast events. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> CastEventTask;	

	/** Task used to collect targets. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PerformTargeting> TargetingTask;

	/** Task used to spawn the cast actor. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_SpawnCast> SpawnActorTask;

	/** Handle for the applied Cast Effect applied on self, which can be cancelled. */
	FActiveGameplayEffectHandle SelfCastEffectHandle;	
	
};
