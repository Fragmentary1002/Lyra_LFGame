// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Effects/CombatEffect_InitializeAttributes.h"

#include "AbilitySystem/NinjaCombatAttributeSet.h"

UCombatEffect_InitializeAttributes::UCombatEffect_InitializeAttributes()
{
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxHealthAttribute(), 100.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxHealthAddAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxHealthPercentAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetHealthRegenRateAttribute(), 0.f);

	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxStaminaAttribute(), 100.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxStaminaAddAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxStaminaPercentAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetStaminaRegenRateAttribute(), 0.f);

	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxMagicAttribute(), 100.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxMagicAddAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxMagicPercentAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMagicRegenRateAttribute(), 0.f);

	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetBaseDamageAttribute(), 10.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetCriticalHitChanceAttribute(), 0.1f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetCriticalHitMultiplierAttribute(), 2.f);

	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetBlockChanceAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetBlockMitigationAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetBlockAngleAttribute(), 75.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetBlockLimitAttribute(), 100.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetBlockStaminaCostRateAttribute(), 2.f);

	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetDefenseChanceAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetDefenseMitigationAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetDefenseLimitAttribute(), 100.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetDefenseStaminaCostRateAttribute(), 2.f);

	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetArmorReductionAttribute(), 0.f);

	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetLastStandCountAttribute(), 0.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetLastStandHealthPercentAttribute(), 0.1f);

	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetMaxPoiseAttribute(), 100.f);
	SetupAttributeWithFloat(UNinjaCombatAttributeSet::GetPoiseAttribute(), 100.f);
	
	FConditionalGameplayEffect ConditionalHealthEffect;
	ConditionalHealthEffect.EffectClass = UCombatEffect_InitializeHealthAttribute::StaticClass();

	FConditionalGameplayEffect ConditionalStaminaEffect;
	ConditionalStaminaEffect.EffectClass = UCombatEffect_InitializeStaminaAttribute::StaticClass();
	
	FConditionalGameplayEffect ConditionalMagicEffect;
	ConditionalMagicEffect.EffectClass = UCombatEffect_InitializeMagicAttribute::StaticClass();	
	
	FGameplayEffectExecutionDefinition ConditionalExecutions;
	ConditionalExecutions.ConditionalGameplayEffects.Add(ConditionalHealthEffect);
	ConditionalExecutions.ConditionalGameplayEffects.Add(ConditionalStaminaEffect);
	ConditionalExecutions.ConditionalGameplayEffects.Add(ConditionalMagicEffect);
	Executions.Add(ConditionalExecutions);
}

void UCombatEffect_InitializeAttributes::SetupAttributeWithFloat(const FGameplayAttribute& Attribute, const float Value)
{
	const FScalableFloat FloatValue(Value);
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = Attribute;
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(FloatValue);
	ModifierInfo.ModifierOp = EGameplayModOp::Override;
	Modifiers.Add(ModifierInfo);
}

UCombatEffect_InitializeHealthAttribute::UCombatEffect_InitializeHealthAttribute()
{
	FAttributeBasedFloat MagnitudeAttribute;
	MagnitudeAttribute.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
		UNinjaCombatAttributeSet::GetMaxHealthTotalAttribute(),
		EGameplayEffectAttributeCaptureSource::Source, false);
	
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UNinjaCombatAttributeSet::GetHealthAttribute();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(MagnitudeAttribute);
	ModifierInfo.ModifierOp = EGameplayModOp::Override;
	Modifiers.Add(ModifierInfo);
}

UCombatEffect_InitializeStaminaAttribute::UCombatEffect_InitializeStaminaAttribute()
{
	FAttributeBasedFloat MagnitudeAttribute;
	MagnitudeAttribute.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
		UNinjaCombatAttributeSet::GetMaxStaminaTotalAttribute(),
		EGameplayEffectAttributeCaptureSource::Source, false);
	
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UNinjaCombatAttributeSet::GetStaminaAttribute();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(MagnitudeAttribute);
	ModifierInfo.ModifierOp = EGameplayModOp::Override;
	Modifiers.Add(ModifierInfo);	
}

UCombatEffect_InitializeMagicAttribute::UCombatEffect_InitializeMagicAttribute()
{
	FAttributeBasedFloat MagnitudeAttribute;
	MagnitudeAttribute.BackingAttribute = FGameplayEffectAttributeCaptureDefinition(
		UNinjaCombatAttributeSet::GetMaxMagicTotalAttribute(),
		EGameplayEffectAttributeCaptureSource::Source, false);
	
	FGameplayModifierInfo ModifierInfo;
	ModifierInfo.Attribute = UNinjaCombatAttributeSet::GetMagicAttribute();
	ModifierInfo.ModifierMagnitude = FGameplayEffectModifierMagnitude(MagnitudeAttribute);
	ModifierInfo.ModifierOp = EGameplayModOp::Override;
	Modifiers.Add(ModifierInfo);	
}
