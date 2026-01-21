// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Effects/Executions/CombatExecution_Damage.h"

#include "NinjaCombatSubsystem.h"
#include "NinjaCombatTags.h"
#include "AbilitySystem/NinjaCombatAttributeSet.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/CombatDamageModifierInterface.h"

// ReSharper disable once CppUE4CodingStandardNamingViolationWarning
FGameplayTag UCombatExecution_Damage::EMPTY_CALLER_TAG = FGameplayTag::EmptyTag;

struct FDamageExecutionCaptures
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(BaseDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitMultiplier);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Stamina);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PendingDamage);
    
	/** Provides the proper instance for this struct. */
	static FDamageExecutionCaptures& Get()
	{
		static FDamageExecutionCaptures Singleton;
		return Singleton;
	}

private:
	
	FDamageExecutionCaptures()
	{
		// --- Source Attributes ------------
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(UNinjaCombatAttributeSet, BaseDamage, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UNinjaCombatAttributeSet, CriticalHitChance, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UNinjaCombatAttributeSet, CriticalHitMultiplier, Source, true);

		// --- Target Attributes ------------

		DEFINE_ATTRIBUTE_CAPTUREDEF(UNinjaCombatAttributeSet, Health, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UNinjaCombatAttributeSet, Stamina, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UNinjaCombatAttributeSet, PendingDamage, Target, true);
	}
};

UCombatExecution_Damage::UCombatExecution_Damage()
{
	RelevantAttributesToCapture.Add(FDamageExecutionCaptures::Get().BaseDamageDef);
	RelevantAttributesToCapture.Add(FDamageExecutionCaptures::Get().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(FDamageExecutionCaptures::Get().CriticalHitMultiplierDef);
	RelevantAttributesToCapture.Add(FDamageExecutionCaptures::Get().HealthDef);
	RelevantAttributesToCapture.Add(FDamageExecutionCaptures::Get().StaminaDef);
	RelevantAttributesToCapture.Add(FDamageExecutionCaptures::Get().PendingDamageDef);
}

void UCombatExecution_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const TObjectPtr<UAbilitySystemComponent> SourceAbilities = ExecutionParams.GetSourceAbilitySystemComponent();
	const TObjectPtr<UAbilitySystemComponent> TargetAbilities = ExecutionParams.GetTargetAbilitySystemComponent();
	
	if (IsValid(SourceAbilities) && IsValid(TargetAbilities))
	{
		const FGameplayEffectSpec& EffectSpec = ExecutionParams.GetOwningSpec();
		FAggregatorEvaluateParameters EvaluationParams;
		EvaluationParams.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
		EvaluationParams.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

		float PendingDamage = 0.f;
		CalculateBaseDamage(ExecutionParams, EvaluationParams, PendingDamage);
		CalculateCriticalDamage(ExecutionParams, EvaluationParams, PendingDamage);

		if (PendingDamage > 0.f)
		{
			FGameplayModifierEvaluatedData Data;
			Data.Attribute = FDamageExecutionCaptures::Get().PendingDamageProperty;
			Data.ModifierOp = EGameplayModOp::Additive;
			Data.Magnitude = PendingDamage;
			
			OutExecutionOutput.AddOutputModifier(Data);
			OutExecutionOutput.MarkConditionalGameplayEffectsToTrigger();
		}
	}
}

void UCombatExecution_Damage::CalculateBaseDamage(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	const FAggregatorEvaluateParameters& EvaluateParams, float& Damage) const
{
	Capture(Damage, ExecutionParams, EvaluateParams, FDamageExecutionCaptures::Get().BaseDamageDef, Tag_Combat_Data_Damage);		
	if (Damage > 0.f)
	{
		const AActor* SourceActor = ExecutionParams.GetSourceAbilitySystemComponent()->GetAvatarActor();
		const AActor* TargetActor = ExecutionParams.GetTargetAbilitySystemComponent()->GetAvatarActor();

		const AGameStateBase* GameState = SourceActor->GetWorld()->GetGameState();
		if (IsValid(GameState) && GameState->Implements<UCombatDamageModifierInterface>())
		{
			const FGameplayTagContainer& SourceTags = EvaluateParams.SourceTags ? *EvaluateParams.SourceTags : FGameplayTagContainer::EmptyContainer;
			const FGameplayTagContainer& TargetTags = EvaluateParams.TargetTags ? *EvaluateParams.TargetTags : FGameplayTagContainer::EmptyContainer;
			
			Damage = ICombatDamageModifierInterface::Execute_ModifyDamage(GameState, Damage, SourceActor,
				TargetActor, SourceTags, TargetTags);
		}
	}
}

void UCombatExecution_Damage::CalculateCriticalDamage(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	const FAggregatorEvaluateParameters& EvaluateParams, float& Damage) const
{
	if (Damage == 0.f) return;
	
	const UWorld* World = ExecutionParams.GetOwningSpec().GetContext().GetSourceObject()->GetWorld();
	const float CriticalChanceRoll = Roll(World);

	float CriticalHitChance = 0.f;
	Capture(CriticalHitChance, ExecutionParams, EvaluateParams, FDamageExecutionCaptures::Get().CriticalHitChanceDef, Tag_Combat_Data_CriticalHitChance);	
	if (CriticalHitChance == 0.f) return;
	
	if (CriticalChanceRoll <= CriticalHitChance)
	{
		float CriticalHitMultiplier = 0.f;
		Capture(CriticalHitMultiplier, ExecutionParams, EvaluateParams, FDamageExecutionCaptures::Get().CriticalHitMultiplierDef, Tag_Combat_Data_CriticalHitMultiplier);
		if (CriticalHitMultiplier == 0.f) return;
		
		FGameplayEffectSpec* MutableSpec = ExecutionParams.GetOwningSpecForPreExecuteMod();
		MutableSpec->AddDynamicAssetTag(Tag_Combat_Effect_Damage_Critical);
		Damage *= CriticalHitMultiplier;
	}
}

float UCombatExecution_Damage::Roll(const UWorld* World, const float Min, const float Max)
{
	const UNinjaCombatSubsystem* CombatSubsystem = World ? World->GetGameInstance()->GetSubsystem<UNinjaCombatSubsystem>() : nullptr;
	check(IsValid(CombatSubsystem));
	return CombatSubsystem->GetRandomFloatInRange(Min, Max);
}

void UCombatExecution_Damage::Capture(float& InOutValue,
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	const FAggregatorEvaluateParameters& EvaluateParams, const FGameplayEffectAttributeCaptureDefinition& InCaptureDef,
	const FGameplayTag CallerTag)
{
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(InCaptureDef, EvaluateParams, InOutValue);

	if (CallerTag.IsValid())
	{
		constexpr bool bIgnoreMagnitudeNotFound = false;
		InOutValue = ExecutionParams.GetOwningSpec().GetSetByCallerMagnitude(CallerTag, bIgnoreMagnitudeNotFound, InOutValue);
	}
}
