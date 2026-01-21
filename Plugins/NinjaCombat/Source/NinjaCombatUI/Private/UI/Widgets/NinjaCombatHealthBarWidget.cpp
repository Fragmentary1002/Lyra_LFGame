// Ninja Bear Studio Inc., all rights reserved.
#include "UI/Widgets/NinjaCombatHealthBarWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/NinjaCombatAttributeSet.h"

UNinjaCombatHealthBarWidget::UNinjaCombatHealthBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CurrentValueAttribute = UNinjaCombatAttributeSet::GetHealthAttribute();
	MaximumValueAttribute = UNinjaCombatAttributeSet::GetMaxHealthAttribute();
	MaximumAddValueAttribute = UNinjaCombatAttributeSet::GetMaxHealthAddAttribute();
	MaximumPercentValueAttribute = UNinjaCombatAttributeSet::GetMaxHealthPercentAttribute();
	MaximumTotalValueAttribute = UNinjaCombatAttributeSet::GetMaxHealthTotalAttribute();
}
