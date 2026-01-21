// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Combo/STEvaluator_ComboState.h"

#include "NinjaCombatFunctionLibrary.h"
#include "StateTreeExecutionContext.h"
#include "Components/ActorComponent.h"
#include "Interfaces/Components/CombatComboManagerInterface.h"

USTEvaluator_ComboState::USTEvaluator_ComboState(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CombatActor = nullptr;
	bInComboWindow = false;
	ComboCount = 0;
}

void USTEvaluator_ComboState::Tick(FStateTreeExecutionContext& Context, const float DeltaTime)
{
	const AActor* ContextOwner = Cast<AActor>(Context.GetOwner());
	const UActorComponent* ComboComponent = UNinjaCombatFunctionLibrary::GetComboManagerComponent(ContextOwner);

	if (IsValid(ComboComponent))
	{
		bInComboWindow = ICombatComboManagerInterface::Execute_InComboWindow(ComboComponent);
		ComboCount = ICombatComboManagerInterface::Execute_GetComboCount(ComboComponent);
	}
	else
	{
		bInComboWindow = false;
		ComboCount = 0;
	}
}
