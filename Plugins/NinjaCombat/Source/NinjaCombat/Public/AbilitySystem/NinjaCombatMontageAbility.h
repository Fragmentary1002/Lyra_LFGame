// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaCombatGameplayAbility.h"
#include "Interfaces/CombatTargetReceiverInterface.h"
#include "NinjaCombatMontageAbility.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UNinjaCombatAnimationProvider;
class UNinjaCombatMotionWarpingComponent;
class UNinjaCombatMotionWarpingTargetProvider;

/**
 * A combat ability that plays an Animation Montage.
 */
UCLASS(Abstract)
class NINJACOMBAT_API UNinjaCombatMontageAbility : public UNinjaCombatGameplayAbility, public ICombatTargetReceiverInterface
{
	
	GENERATED_BODY()

public:

	static FName AnimationProviderName;
	static FName WarpTargetProviderName;
	
	UNinjaCombatMontageAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Combat Target Receiver implementation
	virtual void ReceiveCombatTarget_Implementation(AActor* CombatTarget) override;
	// -- End Combat Target Receiver implementation
	
	/**
	 * Plays the Animation Montage at the current Section.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	void PlayAnimationMontage();

protected:

	/** Provider used to retrieve an appropriate animation for this montage. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Animation", Instanced, NoClear, meta = (ShowInnerProperties))
	TObjectPtr<UNinjaCombatAnimationProvider> AnimationProvider;
	
	/**
	 * Determines if the montage will be played as soon as the ability activates.
	 *
	 * You may choose not to do that if you want to have some additional condition to execute
	 * before actually triggering the animation, therefore overriding the default behavior.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Animation")
	bool bPlayMontageWhenActivated;

	/**
	 * Determines if the ability should end/cancel with the Animation Montage.
	 *
	 * Montage events for Completing and Blending Out will result in the Ability being
	 * properly finished and Interruptions or Cancellations will also cancel the ability.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Animation")
	bool bMatchAnimationEnd;
	
	/** Determines if this Ability will use Motion Warping. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Motion Warping")
	bool bEnableMotionWarping;

	/**
	 * If set to true, it will wait for the Warping Target (valid or null) before playing the animation.
	 * Otherwise, the Warping Target will still be requested as usual, but the Animation Montage will
	 * start playing immediately.
	 *
	 * This is something to consider if you are using a Warp Target Provider that performs asynchronous
	 * tasks (from EQS or the Targeting System) and the montage has a few frames before actually needing
	 * the target. In cases like that it may be worth setting this to false.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Motion Warping", meta = (EditCondition = "bEnableMotionWarping"))
	bool bWaitOnWarpTargetBeforePlayingAnimation;
	
	/** Name of the warp used for this ability. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Motion Warping", meta = (EditCondition = "bEnableMotionWarping"))
	FName WarpName;

	/** Additional offset, positive or negative to be added to the warp. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Motion Warping", meta = (EditCondition = "bEnableMotionWarping"))
	float WarpOffset;
	
	/** Provides the Warp Target for this ability */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Motion Warping", meta = (EditCondition = "bEnableMotionWarping"), Instanced, NoClear)
	TObjectPtr<UNinjaCombatMotionWarpingTargetProvider> WarpTargetProvider;
	
	// -- Begin Gameplay Ability implementation
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	// -- End Gameplay Ability implementation

	/** The Ability Task used to play the montage. */
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageTask;
	
	/**
	 * Handles the initialization of the animation task. It won't be active yet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	UAbilityTask_PlayMontageAndWait* InitializeAnimationTask();

	/**
	 * Flexible method to initialize the Montage Task using the provided function and once
	 * initialized, mark as ready to activate so the framework will activate the task.
	 *
	 * Override this if you want to treat scenarios where the task must be reused, such as
	 * a Combo System or an ongoing animation that can reuse the task.
	 */
	virtual void SetupAndPlayAnimation();
	
	/**
	 * Handles a Montage that has been completed.
	 */	
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Ability")
	void OnAnimationFinished();
	virtual void OnAnimationFinished_Implementation();

	/**
	 * Handles a Montage that has been cancelled or interrupted.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Ability")
	void OnAnimationCancelled();
	virtual void OnAnimationCancelled_Implementation();

};
