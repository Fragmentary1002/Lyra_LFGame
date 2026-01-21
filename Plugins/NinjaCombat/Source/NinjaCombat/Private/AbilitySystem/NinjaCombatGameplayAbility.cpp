// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/NinjaCombatGameplayAbility.h"

#include "NinjaCombatSettings.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Interfaces/CombatActorInfoProxyInterface.h"

DEFINE_LOG_CATEGORY(LogNinjaCombatAbility);

UNinjaCombatGameplayAbility::UNinjaCombatGameplayAbility(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	bEnableDebug = false;	
}

UActorComponent* UNinjaCombatGameplayAbility::GetComboManagerComponentFromActorInfo() const
{
	const ICombatActorInfoProxyInterface* Proxy = GetActorInfoProxy();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	
	if (Proxy && ActorInfo)
	{
		return Proxy->GetComboManagerComponent(ActorInfo);
	}

	return nullptr;
}

UActorComponent* UNinjaCombatGameplayAbility::GetDamageManagerComponentFromActorInfo() const
{
	const ICombatActorInfoProxyInterface* Proxy = GetActorInfoProxy();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	
	if (Proxy && ActorInfo)
	{
		return Proxy->GetDamageManagerComponent(ActorInfo);
	}

	return nullptr;
}

UActorComponent* UNinjaCombatGameplayAbility::GetDefenseManagerComponentFromActorInfo() const
{
	const ICombatActorInfoProxyInterface* Proxy = GetActorInfoProxy();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	
	if (Proxy && ActorInfo)
	{
		return Proxy->GetDefenseManagerComponent(ActorInfo);
	}

	return nullptr;	
}

UActorComponent* UNinjaCombatGameplayAbility::GetMotionWarpingComponentFromActorInfo() const
{
	const ICombatActorInfoProxyInterface* Proxy = GetActorInfoProxy();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	
	if (Proxy && ActorInfo)
	{
		return Proxy->GetMotionWarpingComponent(ActorInfo);
	}

	return nullptr;
}

UActorComponent* UNinjaCombatGameplayAbility::GetMovementManagerComponentFromActorInfo() const
{
	const ICombatActorInfoProxyInterface* Proxy = GetActorInfoProxy();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	
	if (Proxy && ActorInfo)
	{
		return Proxy->GetMovementManagerComponent(ActorInfo);
	}

	return nullptr;
}

UActorComponent* UNinjaCombatGameplayAbility::GetTargetManagerComponentFromActorInfo() const
{
	const ICombatActorInfoProxyInterface* Proxy = GetActorInfoProxy();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	
	if (Proxy && ActorInfo)
	{
		return Proxy->GetTargetManagerComponent(ActorInfo);
	}

	return nullptr;	
}

UActorComponent* UNinjaCombatGameplayAbility::GetWeaponManagerComponentFromActorInfo() const
{
	const ICombatActorInfoProxyInterface* Proxy = GetActorInfoProxy();
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	
	if (Proxy && ActorInfo)
	{
		return Proxy->GetWeaponManagerComponent(ActorInfo);
	}

	return nullptr;
}

const ICombatActorInfoProxyInterface* UNinjaCombatGameplayAbility::GetActorInfoProxy()
{
	const UNinjaCombatSettings* Settings = GetDefault<UNinjaCombatSettings>();
	check(IsValid(Settings));
	return Settings->GetDefaultActorInfoProxy();
}

UAbilityTask_WaitGameplayEvent* UNinjaCombatGameplayAbility::InitializeEventTask(const FGameplayTag Event, const bool bOnlyMatchExact)
{
	UAbilityTask_WaitGameplayEvent* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, Event, nullptr, false, bOnlyMatchExact);
	checkf(IsValid(Task), TEXT("Unexpected invalid Event Task instance."));

	Task->EventReceived.AddDynamic(this, &ThisClass::HandleEventReceived);
	return Task;	
}

void UNinjaCombatGameplayAbility::FinishLatentTasks(const TArray<UAbilityTask*>& Tasks)
{
	for (TObjectPtr<UAbilityTask> Task : Tasks)
	{
		FinishLatentTask(Task);
	}	
}

void UNinjaCombatGameplayAbility::FinishLatentTask(UAbilityTask* Task)
{
	if (IsValid(Task))
	{
		Task->EndTask();
	}	
}

void UNinjaCombatGameplayAbility::AddDebugMessage(const FString& Message) const
{
#if WITH_EDITOR
	if (bEnableDebug)
	{
		UE_LOG(LogNinjaCombatAbility, Log, TEXT("%s"), *Message);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Emerald, Message);
		}
	}
#endif
}
