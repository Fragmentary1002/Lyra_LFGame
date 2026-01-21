// Ninja Bear Studio Inc., all rights reserved.
#include "Abilitysystem/Providers/Target/TargetProvider_TargetingSystem.h"

#include "AbilitySystem/NinjaCombatGameplayAbility.h"
#include "AbilitySystem/Interfaces/CombatTargetReceiverInterface.h"
#include "Interfaces/Components/CombatTargetManagerInterface.h"
#include "TargetingSystem/TargetingSubsystem.h"
#include "Types/TargetingSystemTypes.h"

UTargetProvider_TargetingSystem::UTargetProvider_TargetingSystem()
{
	bExecuteAsync = true;
	bPrioritizeStoredTarget = false;
}

void UTargetProvider_TargetingSystem::RequestWarpTarget_Implementation(UNinjaCombatGameplayAbility* CombatAbility) const
{
	if (bPrioritizeStoredTarget)
	{
		const UActorComponent* TargetManager = CombatAbility->GetTargetManagerComponentFromActorInfo();
		check(IsValid(TargetManager) && TargetManager->Implements<UCombatTargetManagerInterface>());
	
		AActor* CombatTarget = ICombatTargetManagerInterface::Execute_GetCombatTarget(TargetManager);
		ICombatTargetReceiverInterface::Execute_ReceiveCombatTarget(CombatAbility, CombatTarget);
		
		if (IsValid(CombatTarget))
		{
			ICombatTargetReceiverInterface::Execute_ReceiveCombatTarget(CombatAbility, CombatTarget);
			return;
		}
	}
	
	AActor* Avatar = CombatAbility->GetAvatarActorFromActorInfo();
	check(IsValid(Avatar));

	UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::Get(Avatar->GetWorld());
	check(IsValid(TargetingSubsystem));

	FTargetingSourceContext SourceContext;
	SourceContext.SourceActor = Avatar;
	SourceContext.SourceLocation = Avatar->GetActorLocation();
	
	const FTargetingRequestHandle TargetingHandle = UTargetingSubsystem::MakeTargetRequestHandle(TargetingPreset, SourceContext);

	const FTargetingRequestDelegate Delegate = FTargetingRequestDelegate::CreateWeakLambda(this, [this, CombatAbility](const FTargetingRequestHandle& Handle)
		{ HandleTargetReceived(CombatAbility, Handle); });
	
	if (bExecuteAsync)
	{
		FTargetingAsyncTaskData& AsyncTaskData = FTargetingAsyncTaskData::FindOrAdd(TargetingHandle);
		AsyncTaskData.bReleaseOnCompletion = true;
		TargetingSubsystem->StartAsyncTargetingRequestWithHandle(TargetingHandle, Delegate);
	}
	else
	{
		FTargetingImmediateTaskData& ImmediateTaskData = FTargetingImmediateTaskData::FindOrAdd(TargetingHandle);
		ImmediateTaskData.bReleaseOnCompletion = true;
		TargetingSubsystem->ExecuteTargetingRequestWithHandle(TargetingHandle, Delegate);
	}
}

void UTargetProvider_TargetingSystem::HandleTargetReceived(UNinjaCombatGameplayAbility* CombatAbility, const FTargetingRequestHandle& TargetingHandle) const
{
	const UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::Get(CombatAbility->GetWorld());
	check(IsValid(TargetingSubsystem));

	TArray<AActor*> TargetsFound;
	TargetingSubsystem->GetTargetingResultsActors(TargetingHandle, TargetsFound);

	AActor* Target = TargetsFound.IsValidIndex(0) ? TargetsFound[0] : nullptr;
	ICombatTargetReceiverInterface::Execute_ReceiveCombatTarget(CombatAbility, Target);
}
