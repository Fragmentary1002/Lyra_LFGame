// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_Combo.h"

#include "InputAction.h"
#include "NinjaCombatFunctionLibrary.h"
#include "NinjaCombatTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Effects/CombatEffect_ComboWindow.h"
#include "Data/NinjaCombatComboSetupData.h"
#include "Interfaces/Components/CombatComboManagerInterface.h"

UCombatAbility_Combo::UCombatAbility_Combo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ComboWindowEffectClass = UCombatEffect_ComboWindow::StaticClass();
}

bool UCombatAbility_Combo::InComboWindow() const
{
	check(IsValid(ComboManager));
	return ICombatComboManagerInterface::Execute_InComboWindow(ComboManager);
}

int32 UCombatAbility_Combo::GetComboCounter() const
{
	check(IsValid(ComboManager));
	return ICombatComboManagerInterface::Execute_GetComboCount(ComboManager);
}

bool UCombatAbility_Combo::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)
		&& ensureMsgf(IsValid(ComboData), TEXT("A Combo Data Asset is required to activate the ability."));
}

void UCombatAbility_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	ComboManager = GetComboManagerComponentFromActorInfo();
	if (IsValid(ComboManager))
	{
		static constexpr bool bExactOnly = false;
		ComboEventTask = InitializeEventTask(Tag_Combat_Event_Combo, bExactOnly);
		ComboEventTask->ReadyForActivation();

		FComboFinishedDelegate Delegate;
		Delegate.BindDynamic(this, &ThisClass::UCombatAbility_Combo::HandleComboFinished);
	
		ICombatComboManagerInterface::Execute_BindToComboFinishedDelegate(ComboManager, Delegate);
		ICombatComboManagerInterface::Execute_StartCombo(ComboManager, ComboData);
	}
	else
	{
		K2_CancelAbility();
	}
}

void UCombatAbility_Combo::HandleEventReceived_Implementation(const FGameplayEventData Payload)
{
	Super::HandleEventReceived_Implementation(Payload);

	if (Payload.EventTag == Tag_Combat_Event_Combo_Attack && InComboWindow())
	{
		const UInputAction* Action = GetInputActionFromEvent(Payload);
		if (ensure(IsValid(Action)))
		{
			ICombatComboManagerInterface::Execute_AdvanceCombo(ComboManager, Action);
		}
	}
	else if (Payload.EventTag == Tag_Combat_Event_Combo_Begin)
	{
		GrantComboWindowEffect();
	}
	else if (Payload.EventTag == Tag_Combat_Event_Combo_End)
	{
		RevokeComboWindowEffect();
	}
}

void UCombatAbility_Combo::HandleComboFinished(const UNinjaCombatComboSetupData* FinishedComboData, const bool bSucceeded)
{
	if (ComboData == FinishedComboData)
	{
		constexpr bool bReplicateEndAbility = false;
		const bool bWasCancelled = !bSucceeded;
		
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(),
			bReplicateEndAbility, bWasCancelled);
	}
}

const UInputAction* UCombatAbility_Combo::GetInputActionFromEvent_Implementation(
	const FGameplayEventData& Payload) const
{
	return Cast<UInputAction>(Payload.OptionalObject);
}

void UCombatAbility_Combo::GrantComboWindowEffect()
{
	if (IsValid(ComboWindowEffectClass) && !InComboWindow())
	{
		const FGameplayEffectSpecHandle Handle = MakeOutgoingGameplayEffectSpec(ComboWindowEffectClass);
		ComboWindowEffectHandle = K2_ApplyGameplayEffectSpecToOwner(Handle);

		if (ComboWindowEffectHandle.IsValid())
		{
			if (bEnableDebug)
			{
				const FString& DebugMessage = FString::Printf(TEXT("Granted Combo Window Effect."));
				AddDebugMessage(DebugMessage);	
			}
		}
	}	
}

void UCombatAbility_Combo::RevokeComboWindowEffect()
{
	if (ComboWindowEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(ComboWindowEffectHandle);
		ComboWindowEffectHandle.Invalidate();

		if (bEnableDebug)
		{
			const FString& DebugMessage = FString::Printf(TEXT("Revoked Combo Window Effect."));
			AddDebugMessage(DebugMessage);	
		}
	}	
}

void UCombatAbility_Combo::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	ICombatComboManagerInterface::Execute_UnbindFromComboFinishedDelegate(ComboManager, this);
	
	FinishLatentTasks({ ComboEventTask });
	RevokeComboWindowEffect();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

