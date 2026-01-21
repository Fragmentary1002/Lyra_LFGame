// Ninja Bear Studio Inc., all rights reserved.
#include "UI/Components/NinjaCombatWidgetComponent.h"

#include "UI/Interfaces/CombatWidgetInterface.h"

UNinjaCombatWidgetComponent::UNinjaCombatWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNinjaCombatWidgetComponent::InitWidget()
{
	Super::InitWidget();

	UUserWidget* MyWidgetInstance = GetWidget();
	if (IsValid(MyWidgetInstance) && MyWidgetInstance->Implements<UCombatWidgetInterface>())
	{
		ICombatWidgetInterface::Execute_SetCombatActor(MyWidgetInstance, GetOwner());
	}
}

void UNinjaCombatWidgetComponent::HandleGameplayCue(const FGameplayCueParameters& CueParameters) const
{
	UUserWidget* MyWidgetInstance = GetWidget();
	if (IsValid(MyWidgetInstance) && MyWidgetInstance->Implements<UCombatWidgetInterface>())
	{
		ICombatWidgetInterface::Execute_HandleGameplayCue(MyWidgetInstance, CueParameters);
	}
}
