// Ninja Bear Studio Inc., all rights reserved.
#include "UI/Widgets/NinjaCombatMagicBarWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/NinjaCombatAttributeSet.h"

UNinjaCombatMagicBarWidget::UNinjaCombatMagicBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	CurrentValueAttribute = UNinjaCombatAttributeSet::GetMagicAttribute();
	MaximumValueAttribute = UNinjaCombatAttributeSet::GetMaxMagicAttribute();
	MaximumAddValueAttribute = UNinjaCombatAttributeSet::GetMaxMagicAddAttribute();
	MaximumPercentValueAttribute = UNinjaCombatAttributeSet::GetMaxMagicPercentAttribute();
	MaximumTotalValueAttribute = UNinjaCombatAttributeSet::GetMaxMagicTotalAttribute();
}
