// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_TargetLock.h"

#include "NinjaCombatFunctionLibrary.h"
#include "NinjaCombatTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Effects/CombatEffect_LockedOnTarget.h"
#include "AbilitySystem/Tasks/AbilityTask_TrackDistance.h"
#include "AbilityTasks/AbilityTask_PerformTargeting.h"
#include "Interfaces/Components/CombatDamageManagerInterface.h"
#include "Interfaces/Components/CombatTargetManagerInterface.h"
#include "TargetingSystem/TargetingPreset.h"
#include "TargetingSystem/TargetingSubsystem.h"

UCombatAbility_TargetLock::UCombatAbility_TargetLock()
{
	bPerformAsyncTargeting = true;
	DistanceThreshold = 500.f;
	AbilityTags.AddTag(Tag_Combat_Ability_TargetLock);
	TargetLockEffectClass = UCombatEffect_LockedOnTarget::StaticClass();

	FAbilityTriggerData& EventTrigger = AbilityTriggers.AddDefaulted_GetRef();
	EventTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	EventTrigger.TriggerTag = Tag_Combat_Event_Target_Acquired;
}

bool UCombatAbility_TargetLock::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)
		&& IsValid(TargetingPreset)
		&& IsValid(GetTargetManagerComponentFromActorInfo());
}

void UCombatAbility_TargetLock::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!K2_CommitAbility())
	{
		const FString& Message = FString::Printf(TEXT("Unable to activate, cost requirements not met."));
        AddDebugMessage(Message);
		K2_CancelAbility();
	}
	
	if (TriggerEventData != nullptr && TriggerEventData->EventTag == Tag_Combat_Event_Target_Acquired)
	{
		UObject* OptionalObject = const_cast<UObject*>(TriggerEventData->OptionalObject.Get());
		AActor* NewTarget = Cast<AActor>(OptionalObject);
		if (IsValid(NewTarget))
		{
			HandleNewTarget(NewTarget);
			K2_CancelAbility();
		}
	}
	else
	{
		TargetingTask = UAbilityTask_PerformTargeting::PerformTargetingRequest(this, TargetingPreset, bPerformAsyncTargeting);
		TargetingTask->OnTargetReady.AddUniqueDynamic(this, &ThisClass::OnTargetReady);
		TargetingTask->ReadyForActivation();	
	}
}

void UCombatAbility_TargetLock::OnTargetReady(const FTargetingRequestHandle TargetingRequestHandle)
{
	const UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::Get(GetWorld());
	check(IsValid(TargetingSubsystem));

	TArray<AActor*> TargetsFound;
	TargetingSubsystem->GetTargetingResultsActors(TargetingRequestHandle, TargetsFound);

	AActor* NewTarget = TargetsFound.IsValidIndex(0) ? TargetsFound[0] : nullptr;
	if (IsValid(NewTarget))
	{
		HandleNewTarget(NewTarget);
	}
	else
	{
		K2_CancelAbility();
		
		const FString& Message = FString::Printf(TEXT("No targets found!"));
		AddDebugMessage(Message);
	}
}

void UCombatAbility_TargetLock::HandleNewTarget_Implementation(AActor* NewTarget)
{
	DismissCurrentTarget();
	CurrentTarget = NewTarget;
	
	SetTargetOnOwnerComponent();
	BindToDeathDelegate();
	ApplyTargetLockEffect();
	TrackDistance();

	TargetDismissedEventTag = InitializeEventTask(Tag_Combat_Event_Target_Dismissed);
	TargetDismissedEventTag->ReadyForActivation();
	
	const FString& Message = FString::Printf(TEXT("Locked on Target %s."), *GetNameSafe(CurrentTarget));
	AddDebugMessage(Message);
}

void UCombatAbility_TargetLock::SetTargetOnOwnerComponent()
{
	UActorComponent* TargetManager = GetTargetManagerComponentFromActorInfo();
	check(IsValid(TargetManager));
	ICombatTargetManagerInterface::Execute_SetCombatTarget(TargetManager, CurrentTarget);
}

void UCombatAbility_TargetLock::BindToDeathDelegate()
{
	UActorComponent* DamageManager = UNinjaCombatFunctionLibrary::GetDamageManagerComponent(CurrentTarget);
	if (IsValid(DamageManager))
	{
		FOwnerDiedDelegate Delegate;
		Delegate.BindDynamic(this, &ThisClass::K2_EndAbility);
		ICombatDamageManagerInterface::Execute_BindToOwnerDiedDelegate(DamageManager, Delegate);
	}
}

void UCombatAbility_TargetLock::ApplyTargetLockEffect()
{
	if (IsValid(TargetLockEffectClass))
	{
		const FGameplayEffectSpecHandle BlockingEffectHandle = MakeOutgoingGameplayEffectSpec(TargetLockEffectClass);
		ActiveTargetingEffectHandle = K2_ApplyGameplayEffectSpecToOwner(BlockingEffectHandle);
	}
}

void UCombatAbility_TargetLock::TrackDistance()
{
	if (IsValid(DistanceTask))
	{
		// Just replace the target in the current task. No need to re-instantiate it.
		DistanceTask->SetTarget(CurrentTarget);
	}
	else
	{
		DistanceTask = UAbilityTask_TrackDistance::CreateTask(this, CurrentTarget, DistanceThreshold);
		DistanceTask->OnDistanceExceeded.AddUniqueDynamic(this, &ThisClass::OnDistanceExceeded);
		DistanceTask->ReadyForActivation();
	}
}

void UCombatAbility_TargetLock::OnDistanceExceeded(const AActor* Target, float Distance)
{
	K2_EndAbility();
}

void UCombatAbility_TargetLock::DismissCurrentTarget_Implementation()
{
	if (IsValid(CurrentTarget))
	{
		ClearTargetFromOwnerComponent();
		UnbindFromDeathDelegate();
		RemoveTargetLockEffect();
		StopTrackingDistance();

		CurrentTarget = nullptr;

		const FString& Message = FString::Printf(TEXT("Unlocked from Target."));
		AddDebugMessage(Message);
	}
}

void UCombatAbility_TargetLock::ClearTargetFromOwnerComponent()
{
	UActorComponent* TargetManager = GetTargetManagerComponentFromActorInfo();
	check(IsValid(TargetManager));
	ICombatTargetManagerInterface::Execute_SetCombatTarget(TargetManager, nullptr);
}

void UCombatAbility_TargetLock::UnbindFromDeathDelegate()
{
	UActorComponent* DamageManager = UNinjaCombatFunctionLibrary::GetDamageManagerComponent(CurrentTarget);
	if (IsValid(DamageManager))
	{
		ICombatDamageManagerInterface::Execute_UnbindFromOwnerDiedDelegate(DamageManager, this);
	}
}

void UCombatAbility_TargetLock::RemoveTargetLockEffect()
{
	if (ActiveTargetingEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(ActiveTargetingEffectHandle);
		ActiveTargetingEffectHandle.Invalidate();
	}
}

void UCombatAbility_TargetLock::StopTrackingDistance()
{
	FinishLatentTask(DistanceTask);
}

void UCombatAbility_TargetLock::HandleEventReceived_Implementation(const FGameplayEventData Payload)
{
	if (Payload.EventTag == Tag_Combat_Event_Target_Dismissed)
	{
		K2_EndAbility();
	}
}

void UCombatAbility_TargetLock::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	FinishLatentTasks({ TargetingTask, DistanceTask, TargetDismissedEventTag });
	DismissCurrentTarget();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
