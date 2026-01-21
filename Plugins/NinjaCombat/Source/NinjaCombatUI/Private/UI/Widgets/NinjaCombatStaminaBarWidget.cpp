// Ninja Bear Studio Inc., all rights reserved.
#include "UI/Widgets/NinjaCombatStaminaBarWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/NinjaCombatAttributeSet.h"

UNinjaCombatStaminaBarWidget::UNinjaCombatStaminaBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CurrentValueAttribute = UNinjaCombatAttributeSet::GetStaminaAttribute();
	MaximumValueAttribute = UNinjaCombatAttributeSet::GetMaxStaminaAttribute();
	MaximumAddValueAttribute = UNinjaCombatAttributeSet::GetMaxStaminaAddAttribute();
	MaximumPercentValueAttribute = UNinjaCombatAttributeSet::GetMaxStaminaPercentAttribute();
	MaximumTotalValueAttribute = UNinjaCombatAttributeSet::GetMaxStaminaTotalAttribute();	
}
