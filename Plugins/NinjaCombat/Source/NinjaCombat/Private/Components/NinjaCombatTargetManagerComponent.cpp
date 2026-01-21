// Ninja Bear Studio Inc., all rights reserved.
#include "Components/NinjaCombatTargetManagerComponent.h"

#include "NinjaCombatFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

UNinjaCombatTargetManagerComponent::UNinjaCombatTargetManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
	
	bCanPredictTarget = true;
}

void UNinjaCombatTargetManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, CombatTarget);
}

void UNinjaCombatTargetManagerComponent::BindToCombatTargetChangedDelegate_Implementation(const FCombatTargetChangedDelegate& Delegate)
{
	OnCombatTargetChanged.AddUnique(Delegate);
}

void UNinjaCombatTargetManagerComponent::UnbindFromTargetChangedDelegate_Implementation(const UObject* Source)
{
	OnCombatTargetChanged.RemoveAll(Source);
}

AActor* UNinjaCombatTargetManagerComponent::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void UNinjaCombatTargetManagerComponent::SetCombatTarget_Implementation(AActor* NewCombatTarget)
{
	const bool bCanPredictLocally = bCanPredictTarget && OwnerIsLocallyControlled();
	
	if (OwnerHasAuthority() || bCanPredictLocally)
	{
		AActor* OldCombatTarget = CombatTarget;
		CombatTarget = NewCombatTarget;
		OnRep_CombatTarget(OldCombatTarget);
	}
	
	if (!OwnerHasAuthority() && OwnerIsLocallyControlled())
	{
		// Local players without network authority must replicate the value from server.
		// This needs to happen regardless of local prediction of the value.
		//
		Server_SetCombatTarget(NewCombatTarget);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UNinjaCombatTargetManagerComponent::OnRep_CombatTarget(AActor* OldCombatTarget)
{
	// Make sure that we are only broadcasting actual changes between current and old targets.
	// Otherwise, if we are predicting the local client, we could potentially broadcast twice.
	//
	if (CombatTarget != OldCombatTarget)
	{
		UActorComponent* OldTargetComponent = UNinjaCombatFunctionLibrary::GetTargetManagerComponent(OldCombatTarget);
		if (IsValid(OldTargetComponent))
		{
			Execute_ClearTargetingSource(OldTargetComponent, GetOwner());
		}

		UActorComponent* NewTargetComponent = UNinjaCombatFunctionLibrary::GetTargetManagerComponent(CombatTarget);
		if (IsValid(NewTargetComponent))
		{
			Execute_ReceiveTargetingSource(NewTargetComponent, GetOwner());
		}
		
		if (OnCombatTargetChanged.IsBound())
		{
			OnCombatTargetChanged.Broadcast(CombatTarget, OldCombatTarget);
		}	
	}
}

void UNinjaCombatTargetManagerComponent::ReceiveTargetingSource_Implementation(AActor* TargetingSource)
{
}

void UNinjaCombatTargetManagerComponent::ClearTargetingSource_Implementation(AActor* TargetingSource)
{
}

void UNinjaCombatTargetManagerComponent::Server_SetCombatTarget_Implementation(AActor* NewCombatTarget)
{
	Execute_SetCombatTarget(this, NewCombatTarget);
}

bool UNinjaCombatTargetManagerComponent::Server_SetCombatTarget_Validate(AActor* NewCombatTarget)
{
	return true;
}
