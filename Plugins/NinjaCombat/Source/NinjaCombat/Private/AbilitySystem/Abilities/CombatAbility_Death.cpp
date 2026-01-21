// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_Death.h"

#include "AbilitySystemComponent.h"
#include "NinjaCombatTags.h"
#include "Interfaces/Components/CombatDamageManagerInterface.h"

UCombatAbility_Death::UCombatAbility_Death(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		.DoNotCreateDefaultSubobject(WarpTargetProviderName))
{
	bMatchAnimationEnd = true;
	bEnableMotionWarping = false;
	WarpTargetProvider = nullptr;

	AbilityTags.AddTag(Tag_Combat_Ability_Death);

	FAbilityTriggerData DeathTrigger;
	DeathTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	DeathTrigger.TriggerTag = Tag_Combat_Event_Death;
	AbilityTriggers.Add(DeathTrigger);
}

void UCombatAbility_Death::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (TriggerEventData && TriggerEventData->EventTag == Tag_Combat_Event_Death)
	{
		PlayAnimationMontage();
	}
	else
	{
		K2_CancelAbility();
	}
}

void UCombatAbility_Death::FinishDying_Implementation()
{
	UActorComponent* DamageManager = GetDamageManagerComponentFromActorInfo();
	if (IsValid(DamageManager) && ensure(DamageManager->Implements<UCombatDamageManagerInterface>()))
	{
		ICombatDamageManagerInterface::Execute_FinishDying(DamageManager);
	}
}

void UCombatAbility_Death::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	FinishDying();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


