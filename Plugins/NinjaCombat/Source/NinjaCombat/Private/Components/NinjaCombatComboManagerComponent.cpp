// Ninja Bear Studio Inc., all rights reserved.
#include "Components/NinjaCombatComboManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NinjaCombatTags.h"
#include "StateTree.h"
#include "Data/NinjaCombatComboSetupData.h"
#include "Net/UnrealNetwork.h"

UNinjaCombatComboManagerComponent::UNinjaCombatComboManagerComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bStartLogicAutomatically = false;
	bInComboWindow = false;
	ComboCount = 0;
	SetIsReplicatedByDefault(true);
}

void UNinjaCombatComboManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ComboCount);
	DOREPLIFETIME_CONDITION(ThisClass, bInComboWindow, COND_SkipOwner);
}

void UNinjaCombatComboManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	BindToAbilitySystemComponent();
	OnStateTreeRunStatusChanged.AddUniqueDynamic(this, &ThisClass::HandleTreeStatusChanged);
}

void UNinjaCombatComboManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromAbilitySystemComponent();
	OnStateTreeRunStatusChanged.RemoveAll(this);
	Super::EndPlay(EndPlayReason);
}

void UNinjaCombatComboManagerComponent::BindToComboWindowChangedDelegate_Implementation(const FComboWindowChangedDelegate& Delegate)
{
	OnComboWindowChanged.AddUnique(Delegate);
}

void UNinjaCombatComboManagerComponent::BindToComboFinishedDelegate_Implementation(const FComboFinishedDelegate& Delegate)
{
	OnComboFinished.AddUnique(Delegate);
}

void UNinjaCombatComboManagerComponent::UnbindFromComboWindowChangedDelegate_Implementation(const UObject* Source)
{
	OnComboWindowChanged.RemoveAll(Source);
}

void UNinjaCombatComboManagerComponent::UnbindFromComboFinishedDelegate_Implementation(const UObject* Source)
{
	OnComboFinished.RemoveAll(Source);
}

void UNinjaCombatComboManagerComponent::OpenComboWindow_Implementation()
{
	if (!bInComboWindow)
	{
		const APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (GetOwner()->HasAuthority() || (OwnerPawn && OwnerPawn->IsLocallyControlled()))
		{
			const bool bWasInComboWindow = bInComboWindow;
			bInComboWindow = true;
			OnRep_InComboWindow(bWasInComboWindow);
		}
	}	
}

void UNinjaCombatComboManagerComponent::CloseComboWindow_Implementation()
{
	if (bInComboWindow)
	{
		const APawn* OwnerPawn = Cast<APawn>(GetOwner());
		if (GetOwner()->HasAuthority() || (OwnerPawn && OwnerPawn->IsLocallyControlled()))
		{
			const bool bWasInComboWindow = bInComboWindow;
			bInComboWindow = false;
			OnRep_InComboWindow(bWasInComboWindow);
		}
	}		
}

void UNinjaCombatComboManagerComponent::SetComboCount_Implementation(const int32 NewComboCount)
{
	if (NewComboCount >= 0)
	{
		ComboCount = NewComboCount;	
	}
}

void UNinjaCombatComboManagerComponent::StartCombo_Implementation(const UNinjaCombatComboSetupData* NewComboData)
{
	check(IsValid(NewComboData));
	check(IsValid(NewComboData->ComboTree));

	if (NewComboData != ComboData)
	{
		Execute_ResetCombo(this);
		ComboData = NewComboData;
		
		StateTreeRef.SetStateTree(ComboData->ComboTree);
		StartLogic();	
	}
}

void UNinjaCombatComboManagerComponent::AdvanceCombo_Implementation(const UInputAction* Action)
{
	check(IsValid(ComboData));
	
	if (IsValid(Action))
	{
		const FGameplayTag EventTag = ComboData->GetComboEventTag(Action);
		if (EventTag.IsValid())
		{
			SendStateTreeEvent(EventTag);
			Execute_CloseComboWindow(this);
		}
	}
}

void UNinjaCombatComboManagerComponent::ResetCombo_Implementation()
{
	if (IsRunning())
	{
		const static FString Reason = TEXT("Reseting the Combo.");
		StopLogic(Reason);
	}

	bInComboWindow = false;
	ComboCount = 0;
	ComboData = nullptr;	
}

void UNinjaCombatComboManagerComponent::HandleTreeStatusChanged(const EStateTreeRunStatus StateTreeRunStatus)
{
	if (IsFinished(StateTreeRunStatus))
	{
		const bool bSucceeded = StateTreeRunStatus == EStateTreeRunStatus::Succeeded;
		
		if (OnComboFinished.IsBound())
		{
			OnComboFinished.Broadcast(ComboData, bSucceeded);	
		}

		Execute_ResetCombo(this);
	}
}

void UNinjaCombatComboManagerComponent::BindToAbilitySystemComponent()
{
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn->HasAuthority() || OwnerPawn->IsLocallyControlled())
	{
		UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
		if (IsValid(AbilitySystemComponent))
		{
			ComboWindowTagHandle = AbilitySystemComponent->RegisterGameplayTagEvent(Tag_Combat_State_ComboWindow, EGameplayTagEventType::NewOrRemoved)
				.AddUObject(this, &ThisClass::HandleTagChanged);			
		}
		else
		{
			// Try again on next tick. It may not have been replicated yet.
			GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::BindToAbilitySystemComponent);
		}
	}
}

void UNinjaCombatComboManagerComponent::UnbindFromAbilitySystemComponent()
{
	UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
	if (IsValid(AbilitySystemComponent))
	{
		if (ComboWindowTagHandle.IsValid())
		{
			AbilitySystemComponent->UnregisterGameplayTagEvent(ComboWindowTagHandle, Tag_Combat_State_ComboWindow, EGameplayTagEventType::NewOrRemoved);
			ComboWindowTagHandle.Reset();			
		}
	}
}

void UNinjaCombatComboManagerComponent::HandleTagChanged(const FGameplayTag CallbackTag, const int32 NewCount)
{
	if (CallbackTag == Tag_Combat_State_ComboWindow)
	{
		if (NewCount > 0)
		{
			Execute_OpenComboWindow(this);
		}
		else
		{
			Execute_CloseComboWindow(this);
		}
	}	
}

bool UNinjaCombatComboManagerComponent::IsFinished(const EStateTreeRunStatus& StateTreeRunStatus)
{
	return StateTreeRunStatus == EStateTreeRunStatus::Failed || StateTreeRunStatus == EStateTreeRunStatus::Succeeded;
}

void UNinjaCombatComboManagerComponent::OnRep_InComboWindow_Implementation(bool bWasInComboWindow)
{
	if (OnComboWindowChanged.IsBound())
	{
		OnComboWindowChanged.Broadcast(ComboData, bInComboWindow);
	}
}
