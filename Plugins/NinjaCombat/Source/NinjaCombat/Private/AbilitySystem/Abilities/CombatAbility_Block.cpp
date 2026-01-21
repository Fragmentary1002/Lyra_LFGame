// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_Block.h"

#include "NinjaCombatTags.h"
#include "AbilitySystem/Effects/CombatEffect_Blocking.h"
#include "Interfaces/Components/CombatDefenseManagerInterface.h"

UCombatAbility_Block::UCombatAbility_Block(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	AbilityTags.AddTag(Tag_Combat_Ability_Block);
	BlockingEffectClass = UCombatEffect_Blocking::StaticClass();
}

bool UCombatAbility_Block::IsBlocking() const
{
	const UActorComponent* DefenseManager = GetDefenseManagerComponentFromActorInfo();
	check(IsValid(DefenseManager));
	return ICombatDefenseManagerInterface::Execute_IsBlocking(DefenseManager);
}

void UCombatAbility_Block::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const bool bWasBlocking = IsBlocking();
	
	// We'll check here if the avatar is blocking, instead of checking in "CanActivateAbility".
	// Reason for that is because we want to still activate the ability if the avatar is blocking,
	// but when the ability is deactivated, we'll clear both - the handle and pre-existing effect.

	if (IsValid(BlockingEffectClass) && !bWasBlocking)
	{
		const FGameplayEffectSpecHandle EffectHandle = MakeOutgoingGameplayEffectSpec(BlockingEffectClass);
		BlockingEffectHandle = K2_ApplyGameplayEffectSpecToOwner(EffectHandle);
	}

	if (IsBlocking())
	{
		if (bEnableDebug)
		{
			const FString& DebugMessage = FString::Printf(TEXT("Avatar is blocking (%s)."), bWasBlocking ? TEXT("had effect") : TEXT("applied effect"));
			AddDebugMessage(DebugMessage);
		}
	}
	else
	{
		K2_CancelAbility();
	}
}

void UCombatAbility_Block::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	// First, try to remove the effect from this ability.
	if (BlockingEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(BlockingEffectHandle);
		BlockingEffectHandle.Invalidate();

		if (bEnableDebug)
		{
			const FString& DebugMessage = FString::Printf(TEXT("Avatar stopped blocking (from applied effect)."));
			AddDebugMessage(DebugMessage);	
		}		
	}

	// If we are still blocking, then we had a pre-existing effect. Maybe a default one?
	if (IsBlocking())
	{
		FGameplayTagContainer TagContainer = FGameplayTagContainer::EmptyContainer;
		TagContainer.AddTagFast(Tag_Combat_State_Blocking);
		BP_RemoveGameplayEffectFromOwnerWithGrantedTags(TagContainer);

		if (bEnableDebug)
		{
			const FString& DebugMessage = FString::Printf(TEXT("Avatar stopped blocking (from previous effect)."));
			AddDebugMessage(DebugMessage);	
		}		
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
