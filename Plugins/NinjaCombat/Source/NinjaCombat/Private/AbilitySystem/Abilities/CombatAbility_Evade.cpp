// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_Evade.h"

#include "NinjaCombatTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Effects/CombatEffect_Invulnerability.h"
#include "AbilitySystem/Providers/Animation/AnimationProvider_DirectionalEvade.h"

UCombatAbility_Evade::UCombatAbility_Evade(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		.SetDefaultSubobjectClass<UAnimationProvider_DirectionalEvade>(AnimationProviderName))
{
	AbilityTags.AddTag(Tag_Combat_Ability_Evade);
	InvulnerabilityEffectClass = UCombatEffect_Invulnerability::StaticClass();
}

void UCombatAbility_Evade::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	static constexpr bool bExactOnly = false;
	InvulnerabilityEventTask = InitializeEventTask(Tag_Combat_Event_Invulnerability, bExactOnly);
	InvulnerabilityEventTask->ReadyForActivation();
}

void UCombatAbility_Evade::HandleEventReceived_Implementation(const FGameplayEventData Payload)
{
	Super::HandleEventReceived_Implementation(Payload);
	
	if (Payload.EventTag == Tag_Combat_Event_Invulnerability_Begin)
	{
		GrantInvulnerabilityEffect();
	}
	else if (Payload.EventTag == Tag_Combat_Event_Invulnerability_End)
	{
		RevokeInvulnerabilityEffect();
	}
}

void UCombatAbility_Evade::GrantInvulnerabilityEffect()
{
	if (IsValid(InvulnerabilityEffectClass))
	{
		const FGameplayEffectSpecHandle Handle = MakeOutgoingGameplayEffectSpec(InvulnerabilityEffectClass);
		InvulnerabilityEffectHandle = K2_ApplyGameplayEffectSpecToOwner(Handle);

		if (InvulnerabilityEffectHandle.IsValid())
		{
			if (bEnableDebug)
			{
				const FString& DebugMessage = FString::Printf(TEXT("Granted Invulnerability Effect."));
				AddDebugMessage(DebugMessage);	
			}
		}
	}
}

void UCombatAbility_Evade::RevokeInvulnerabilityEffect()
{
	if (InvulnerabilityEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(InvulnerabilityEffectHandle);
		InvulnerabilityEffectHandle.Invalidate();

		if (bEnableDebug)
		{
			const FString& DebugMessage = FString::Printf(TEXT("Revoked Invulnerability Effect."));
			AddDebugMessage(DebugMessage);	
		}
	}
}

void UCombatAbility_Evade::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	FinishLatentTasks({ InvulnerabilityEventTask });
	RevokeInvulnerabilityEffect();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
