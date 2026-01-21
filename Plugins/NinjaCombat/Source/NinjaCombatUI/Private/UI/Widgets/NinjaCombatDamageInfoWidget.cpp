// Ninja Bear Studio Inc., all rights reserved.
#include "UI/Widgets/NinjaCombatDamageInfoWidget.h"

#include "NinjaCombatFunctionLibrary.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "NinjaCombatSettings.h"
#include "NinjaCombatTags.h"
#include "AbilitySystem/Interfaces/CombatEffectContextProxyInterface.h"
#include "Components/TextBlock.h"
#include "Kismet/KismetTextLibrary.h"

UNinjaCombatDamageInfoWidget::UNinjaCombatDamageInfoWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bAccumulateDamage = true;
	AccumulatedDamage = 0.f;
}

void UNinjaCombatDamageInfoWidget::HandleGameplayCue_Implementation(const FGameplayCueParameters& CueParameters)
{
	Super::HandleGameplayCue_Implementation(CueParameters);
	
	const ICombatEffectContextProxyInterface* Proxy = GetDefault<UNinjaCombatSettings>()->GetDefaultEffectContextProxy();
	check(Proxy != nullptr);
	float Damage = Proxy->GetDamageApplied(CueParameters.EffectContext.Get());

	const bool bIsBreakerHit = CueParameters.AggregatedSourceTags.HasTagExact(Tag_Combat_Effect_Damage_Critical);
	const bool bIsCriticalHit = CueParameters.AggregatedSourceTags.HasTagExact(Tag_Combat_Effect_Damage_Critical);
	const bool bIsFatalHit = CueParameters.AggregatedSourceTags.HasTagExact(Tag_Combat_Effect_Damage_Fatal);
	
	const FHitResult* HitResult = CueParameters.EffectContext.Get()->GetHitResult();
	const FGameplayTagContainer SourceTags = CueParameters.AggregatedSourceTags;

	if (bAccumulateDamage)
	{
		AccumulatedDamage += Damage;
		Damage = AccumulatedDamage;
	}

	FVector DamageLocation;
	
	if (HitResult && HitResult->HasValidHitObjectHandle())
	{
		DamageLocation = HitResult->GetActor()->GetActorLocation();
		if (HitResult && HitResult->IsValidBlockingHit())
		{
			DamageLocation = HitResult->ImpactPoint;
		}
	}
	else
	{
		DamageLocation = GetCombatActor()->GetActorLocation();
	}

	UpdateDamage(Damage, bIsBreakerHit, bIsCriticalHit, bIsFatalHit, DamageLocation, SourceTags);
}

void UNinjaCombatDamageInfoWidget::UpdateDamage_Implementation(const float Damage, bool bIsBreakerHit, bool bIsCriticalHit,
	bool bIsFatalHit, const FVector DamageLocation, const FGameplayTagContainer& SourceTags)
{
	if (IsValid(DamageTextBlock))
	{
		const FText TextValue = UKismetTextLibrary::Conv_DoubleToText(Damage, ToZero);
		DamageTextBlock->SetText(TextValue);
		ShowWidget();
	}
}

void UNinjaCombatDamageInfoWidget::HideWidget_Implementation()
{
	Super::HideWidget_Implementation();
	AccumulatedDamage = 0.f;
}

