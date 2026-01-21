// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/NinjaCombatGameplayAbility.h"
#include "Types/TargetingSystemTypes.h"
#include "CombatAbility_TargetLock.generated.h"

class UAbilityTask_PerformTargeting;
class UAbilityTask_TrackDistance;
class UAbilityTask_WaitGameplayEvent;
class UGameplayEffect;
class UTargetingPreset;

/**
 * Locks on a target, acquired by a certain targeting preset.
 */
UCLASS()
class NINJACOMBAT_API UCombatAbility_TargetLock : public UNinjaCombatGameplayAbility
{
	
	GENERATED_BODY()

public:

	UCombatAbility_TargetLock();

	// -- Begin Gameplay Ability implementation
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	// -- End Gameplay Ability implementation
	
protected:
	
	/** The Targeting Preset used by the targeting system. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Target Lock")
	TObjectPtr<UTargetingPreset> TargetingPreset;

	/** Determines if Targeting is done asynchronously. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Target Lock")
	bool bPerformAsyncTargeting;

	/** Threshold where a tracked target will be disengaged. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Target Lock")
	float DistanceThreshold;
	
	/** Optional Gameplay Effect applied if a target is acquired. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Target Lock")
	TSubclassOf<UGameplayEffect> TargetLockEffectClass;

	// -- Begin Gameplay Ability implementation
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	virtual void HandleEventReceived_Implementation(FGameplayEventData Payload) override;
	// -- End Gameplay Ability implementation

	/**
	 * Reacts to a response from the Targeting System.
	 */
	UFUNCTION()
	void OnTargetReady(FTargetingRequestHandle TargetingRequestHandle);

	/**
	 * Reacts to the distance from the current target being exceeded.
	 */
	UFUNCTION()
	void OnDistanceExceeded(const AActor* Target, float Distance);
	
	/**
	 * Actually handles the new Target, once it has been initially processed.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Target Lock")
	void HandleNewTarget(AActor* NewTarget);

	virtual void SetTargetOnOwnerComponent();
	virtual void BindToDeathDelegate();
	virtual void ApplyTargetLockEffect();
	virtual void TrackDistance();
	
	/**
	 * Dismiss the current target, unbinding from it.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Target Lock")
	void DismissCurrentTarget();

	virtual void ClearTargetFromOwnerComponent();
	virtual void UnbindFromDeathDelegate();
	virtual void RemoveTargetLockEffect();
	virtual void StopTrackingDistance();
	
private:

	/** Task used to collect targets. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PerformTargeting> TargetingTask;

	/** Task to track the distance from an active target. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_TrackDistance> DistanceTask;

	/** Task to track an external event informing that the target was dismissed. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> TargetDismissedEventTag;

	/** Current target collected by the task. */
	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;
	
	/** Handle for the applied Targeting Handle. */
	FActiveGameplayEffectHandle ActiveTargetingEffectHandle;
	
};
