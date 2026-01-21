// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_Cast.h"

#include "NinjaCombatTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tasks/AbilityTask_SpawnCast.h"
#include "AbilityTasks/AbilityTask_PerformTargeting.h"
#include "Interfaces/CombatCastInterface.h"
#include "TargetingSystem/TargetingPreset.h"
#include "TargetingSystem/TargetingSubsystem.h"

UCombatAbility_Cast::UCombatAbility_Cast(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bAppliesToSelf = true;
	bAppliesToTargets = false;
	Targeting = ECombatCastTargeting::TargetingSystem; 
	bPerformAsyncTargeting = true;

	SelfEffectClass = nullptr;
	TargetEffectClass = nullptr;
	TargetingPreset = nullptr;
	TargetingActorClass = nullptr;
	CosmeticsGameplayCue = FGameplayTag::EmptyTag;
}

bool UCombatAbility_Cast::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)
		&& (bAppliesToSelf || bAppliesToTargets)
		&& (bAppliesToSelf && IsValid(SelfEffectClass) || !bAppliesToSelf)
		&& (bAppliesToTargets && IsValid(TargetEffectClass) || !bAppliesToTargets);
}

void UCombatAbility_Cast::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	CastEventTask = InitializeEventTask(Tag_Combat_Event_Cast);
	CastEventTask->ReadyForActivation();
}

void UCombatAbility_Cast::HandleEventReceived_Implementation(const FGameplayEventData Payload)
{
	Super::HandleEventReceived_Implementation(Payload);

	if (Payload.EventTag == Tag_Combat_Event_Cast)
	{
		if (bAppliesToSelf)
		{
			CastToSelf();
		}

		if (bAppliesToTargets)
		{
			switch (Targeting)
			{
			case ECombatCastTargeting::TargetingSystem:
				CollectTargetsFromTargetingSystem();
				break;
			case ECombatCastTargeting::SpawnActor:
				SpawnTargetingActor();
				break;
			}
		}
	}
}

void UCombatAbility_Cast::CollectTargetsFromTargetingSystem()
{
	if (ensure(IsValid(TargetingPreset)))
	{
		TargetingTask = UAbilityTask_PerformTargeting::PerformTargetingRequest(this, TargetingPreset, bPerformAsyncTargeting);
		TargetingTask->OnTargetReady.AddUniqueDynamic(this, &ThisClass::OnTargetsReady);
		TargetingTask->ReadyForActivation();	
	}
}

FTransform UCombatAbility_Cast::GetCastLocation() const
{
	return GetAvatarActorFromActorInfo()->GetActorTransform();
}

void UCombatAbility_Cast::SpawnTargetingActor()
{
	if (IsValid(TargetingActorClass) && ensure(TargetingActorClass->ImplementsInterface(UCombatCastInterface::StaticClass())))
	{
		const FTransform CastTransform = GetCastLocation();
		SpawnActorTask = UAbilityTask_SpawnCast::CreateTask(this, TargetingActorClass, CastTransform, bEnableDebug);
		SpawnActorTask->OnCastReady.AddUniqueDynamic(this, &ThisClass::OnCastActorReady);
		SpawnActorTask->ReadyForActivation();
	}
}

void UCombatAbility_Cast::OnCastActorReady(AActor* SpawnedActor)
{
	if (IsValid(SpawnedActor) && SpawnedActor->Implements<UCombatCastInterface>())
	{
		ICombatCastInterface::Execute_SetCastSource(SpawnedActor, GetAvatarActorFromActorInfo());

		if (IsValid(SelfEffectClass))
		{
			FGameplayEffectSpecHandle SelfEffectSpecHandle = MakeOutgoingGameplayEffectSpec(SelfEffectClass);
			SelfEffectSpecHandle.Data.Get()->GetContext().AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
			ICombatCastInterface::Execute_SetOwnerEffectHandle(SpawnedActor, SelfEffectSpecHandle);	
		}
		
		if (IsValid(TargetEffectClass))
		{
			FGameplayEffectSpecHandle TargetEffectSpecHandle = MakeOutgoingGameplayEffectSpec(TargetEffectClass);
			TargetEffectSpecHandle.Data.Get()->GetContext().AddInstigator(GetAvatarActorFromActorInfo(), GetAvatarActorFromActorInfo());
			ICombatCastInterface::Execute_SetTargetEffectHandle(SpawnedActor, TargetEffectSpecHandle);	
		}
		
		ICombatCastInterface::Execute_StartCast(SpawnedActor);
	}
}

void UCombatAbility_Cast::OnTargetsReady(const FTargetingRequestHandle TargetingRequestHandle)
{
	const UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::Get(GetWorld());
	check(IsValid(TargetingSubsystem));

	TArray<AActor*> TargetsFound;
	TargetingSubsystem->GetTargetingResultsActors(TargetingRequestHandle, TargetsFound);
	CastToTargets_Implementation(TargetsFound);
}

void UCombatAbility_Cast::CastToSelf_Implementation()
{
	const FGameplayEffectSpecHandle Handle = MakeOutgoingGameplayEffectSpec(SelfEffectClass);
	SelfCastEffectHandle = K2_ApplyGameplayEffectSpecToOwner(Handle);
}

void UCombatAbility_Cast::CastToTargets_Implementation(const TArray<AActor*>& TargetsFound)
{
	const int32 TargetsCount = TargetsFound.Num();
	if (TargetsCount > 0)
	{
		TArray<TWeakObjectPtr<AActor>> WeakTargetsFound;
		WeakTargetsFound.Reserve(TargetsFound.Num());
		for (AActor* TargetFound : TargetsFound) { WeakTargetsFound.Add(TargetFound); } 		

		FGameplayAbilityTargetingLocationInfo TargetingSource;
		TargetingSource.LocationType = EGameplayAbilityTargetingLocationType::ActorTransform;
		TargetingSource.SourceActor = GetAvatarActorFromActorInfo();
		TargetingSource.SourceAbility = this;

		FGameplayEffectSpecHandle TargetHandle = MakeOutgoingGameplayEffectSpec(TargetEffectClass);
		const FGameplayAbilityTargetDataHandle TargetDataHandle = TargetingSource.MakeTargetDataHandleFromActors(WeakTargetsFound);
		const FGameplayEffectSpecHandle EffectSpecHandle = MakeOutgoingGameplayEffectSpec(TargetEffectClass);
		
		if (TargetDataHandle.Num() > 0 && EffectSpecHandle.IsValid())
		{
			EffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(Tag_Combat_Data_CastHits.GetTag(), TargetsCount);
			K2_ApplyGameplayEffectSpecToTarget(EffectSpecHandle, TargetDataHandle);
		}
	}
}

void UCombatAbility_Cast::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	FinishLatentTasks({ CastEventTask, SpawnActorTask, TargetingTask });

	if (SelfCastEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(SelfCastEffectHandle);
		SelfCastEffectHandle.Invalidate();
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

