// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/NinjaCombatMontageAbility.h"

#include "NinjaCombatSettings.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Providers/NinjaCombatAnimationProvider.h"
#include "AbilitySystem/Providers/NinjaCombatMotionWarpingTargetProvider.h"
#include "Animation/AnimMontage.h"
#include "Components/NinjaCombatMotionWarpingComponent.h"

FName UNinjaCombatMontageAbility::AnimationProviderName = TEXT("AnimationProvider");
FName UNinjaCombatMontageAbility::WarpTargetProviderName = TEXT("WarpTargetProvider");

UNinjaCombatMontageAbility::UNinjaCombatMontageAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bPlayMontageWhenActivated = true;
	bMatchAnimationEnd = true;
	
	bEnableMotionWarping = GetDefault<UNinjaCombatSettings>()->bEnableMotionWarpingByDefault;
	bWaitOnWarpTargetBeforePlayingAnimation = false;
	WarpName = TEXT("CombatWarp");
	WarpOffset = 0.f;

	AnimationProvider = CreateDefaultSubobject<UNinjaCombatAnimationProvider>(AnimationProviderName);
	WarpTargetProvider = CreateOptionalDefaultSubobject<UNinjaCombatMotionWarpingTargetProvider>(WarpTargetProviderName);
}

void UNinjaCombatMontageAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{			
		constexpr bool bReplicateEndAbility = true;
		CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility);
	}

	if (bPlayMontageWhenActivated)
	{
		PlayAnimationMontage();	
	}
}

void UNinjaCombatMontageAbility::PlayAnimationMontage()
{
	if (bEnableMotionWarping && IsValid(WarpTargetProvider))
	{
		WarpTargetProvider->RequestWarpTarget(this);
	}
	
	if (!bEnableMotionWarping || !bWaitOnWarpTargetBeforePlayingAnimation)
	{
		SetupAndPlayAnimation();
	}
}

UAbilityTask_PlayMontageAndWait* UNinjaCombatMontageAbility::InitializeAnimationTask()
{
	check(IsValid(AnimationProvider));

	UAnimMontage* Montage = AnimationProvider->GetMontageToPlay(this);
	const FName SectionName = AnimationProvider->GetSectionName(this);
	
	if (IsValid(Montage))
	{
		static const FName TaskInstanceName = TEXT("PlayMontage");
		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, TaskInstanceName, Montage, 1.f, SectionName);
		checkf(IsValid(Task), TEXT("Unexpected invalid Montage Task instance."));
		
		Task->OnCompleted.AddDynamic(this, &ThisClass::OnAnimationFinished);
		Task->OnBlendOut.AddDynamic(this, &ThisClass::OnAnimationFinished);
		Task->OnCancelled.AddDynamic(this, &ThisClass::OnAnimationCancelled);
		Task->OnInterrupted.AddDynamic(this, &ThisClass::OnAnimationCancelled);
	
		return Task;
	}

	return nullptr;
}

void UNinjaCombatMontageAbility::ReceiveCombatTarget_Implementation(AActor* CombatTarget)
{
	UActorComponent* MotionWarpingComponent = GetMotionWarpingComponentFromActorInfo();
	if (IsValid(MotionWarpingComponent) && ensure(MotionWarpingComponent->Implements<UCombatMotionWarpingInterface>()))
	{
		if (IsValid(CombatTarget))
		{
			ICombatMotionWarpingInterface::Execute_SetCombatWarpTarget(MotionWarpingComponent, WarpName, CombatTarget, WarpOffset);
		}
		else
		{
			ICombatMotionWarpingInterface::Execute_ClearCombatWarpTarget(MotionWarpingComponent, WarpName);
		}
	}

	// If we were waiting for the warp target, this is where we can finally play the animation.
	// It does not matter if the Combat Target was valid or null. The animation always plays.
	//
	if (bWaitOnWarpTargetBeforePlayingAnimation)
	{
		SetupAndPlayAnimation();
	}
}

void UNinjaCombatMontageAbility::SetupAndPlayAnimation()
{
	PlayMontageTask = InitializeAnimationTask();
	if (IsValid(PlayMontageTask))
	{
		PlayMontageTask->ReadyForActivation();	
	}
	else
	{
		K2_CancelAbility();
	}
}

void UNinjaCombatMontageAbility::OnAnimationFinished_Implementation()
{
	if (bMatchAnimationEnd)
	{
		K2_EndAbility();	
	}
}

void UNinjaCombatMontageAbility::OnAnimationCancelled_Implementation()
{
	if (bMatchAnimationEnd)
	{
		K2_CancelAbility();	
	}
}

void UNinjaCombatMontageAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	FinishLatentTask(PlayMontageTask);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
