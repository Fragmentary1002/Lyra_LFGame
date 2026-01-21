// Ninja Bear Studio Inc., all rights reserved.
#include "UI/Widgets/NinjaCombatBaseWidget.h"

#include "AbilitySystemGlobals.h"
#include "NinjaCombatFunctionLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"

DEFINE_LOG_CATEGORY(LogNinjaCombatWidget);

UNinjaCombatBaseWidget::UNinjaCombatBaseWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UNinjaCombatBaseWidget::SetCombatActor_Implementation(AActor* NewCombatActor)
{
	if (NewCombatActor != nullptr && NewCombatActor != CombatActor)
	{
		CombatActor = NewCombatActor;
		UE_LOG(LogNinjaCombatWidget, Verbose, TEXT("Assigned owner %s to Widget %s"), *GetNameSafe(CombatActor), *GetNameSafe(this));
		
		WidgetTree->ForEachWidget([&] (UWidget* Widget)
		{
			if (Widget->Implements<UCombatWidgetInterface>())
			{
				Execute_SetCombatActor(Widget, CombatActor);
			}
		});
	}
}

void UNinjaCombatBaseWidget::HandleGameplayCue_Implementation(const FGameplayCueParameters& CueParameters)
{
	WidgetTree->ForEachWidget([&] (UWidget* Widget)
	{
		if (Widget->Implements<UCombatWidgetInterface>())
		{
			Execute_HandleGameplayCue(Widget, CueParameters);
		}
	});
}

AActor* UNinjaCombatBaseWidget::GetCombatActor() const
{
	return IsValid(CombatActor) ? CombatActor : GetOwningPlayerPawn(); 
}

UAbilitySystemComponent* UNinjaCombatBaseWidget::GetAbilityComponent() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetCombatActor());
}
