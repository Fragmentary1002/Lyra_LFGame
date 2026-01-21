// Ninja Bear Studio Inc., all rights reserved.
#include "UI/Widgets/NinjaCombatBaseOverheadBarWidget.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NinjaCombatFunctionLibrary.h"
#include "NinjaCombatTags.h"
#include "TimerManager.h"
#include "Components/ProgressBar.h"
#include "GameFramework/Pawn.h"
#include "Interfaces/Components/CombatDamageManagerInterface.h"

UNinjaCombatBaseOverheadBarWidget::UNinjaCombatBaseOverheadBarWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bHideOnFatalHit = true;
	CurrentValue = 0.f;
	MaximumValue = 0.f;
	MaximumAddValue = 0.f;
	MaximumPercentValue = 0.f;
	MaximumTotalValue = 0.f;
	DisplayDuration = 3.f;
	CurrentValueAttribute = FGameplayAttribute();
	MaximumValueAttribute = FGameplayAttribute();
	MaximumAddValueAttribute = FGameplayAttribute();
	MaximumPercentValueAttribute = FGameplayAttribute();
	MaximumTotalValueAttribute = FGameplayAttribute();
}

void UNinjaCombatBaseOverheadBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BindToAbilityComponent();	
}

void UNinjaCombatBaseOverheadBarWidget::NativeDestruct()
{
	UnbindFromAbilityComponent();
	Super::NativeDestruct();
}

void UNinjaCombatBaseOverheadBarWidget::BindToAbilityComponent()
{
	CachedAbilityComponent = GetAbilityComponent();
	if (IsValid(CachedAbilityComponent))
	{
		CurrentValue = CachedAbilityComponent->GetNumericAttribute(CurrentValueAttribute);
		MaximumValue = CachedAbilityComponent->GetNumericAttribute(MaximumValueAttribute);
		MaximumAddValue = CachedAbilityComponent->GetNumericAttribute(MaximumAddValueAttribute);
		MaximumPercentValue = CachedAbilityComponent->GetNumericAttribute(MaximumPercentValueAttribute);
		MaximumTotalValue = CachedAbilityComponent->GetNumericAttribute(MaximumTotalValueAttribute);
		
		UpdateStatus(CurrentValue, MaximumTotalValue);
		
		CurrentValueDelegate = CachedAbilityComponent->GetGameplayAttributeValueChangeDelegate(CurrentValueAttribute).AddUObject(this, &ThisClass::HandleCurrentAttributeChanged);
		MaximumValueDelegate = CachedAbilityComponent->GetGameplayAttributeValueChangeDelegate(MaximumValueAttribute).AddUObject(this, &ThisClass::HandleMaximumAttributeChanged);
		MaximumAddValueDelegate = CachedAbilityComponent->GetGameplayAttributeValueChangeDelegate(MaximumValueAttribute).AddUObject(this, &ThisClass::HandleMaximumAddAttributeChanged);
		MaximumPercentValueDelegate = CachedAbilityComponent->GetGameplayAttributeValueChangeDelegate(MaximumValueAttribute).AddUObject(this, &ThisClass::HandleMaximumPercentAttributeChanged);
	}
	else
	{
		// It may take a moment for the Ability Component to replicate. Try again on next tick.
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &ThisClass::BindToAbilityComponent);	
	}
}

void UNinjaCombatBaseOverheadBarWidget::UnbindFromAbilityComponent()
{
	if (IsValid(CachedAbilityComponent))
	{
		CachedAbilityComponent->GetGameplayAttributeValueChangeDelegate(CurrentValueAttribute).Remove(CurrentValueDelegate);
		CachedAbilityComponent->GetGameplayAttributeValueChangeDelegate(MaximumValueAttribute).Remove(MaximumValueDelegate);
		CachedAbilityComponent->GetGameplayAttributeValueChangeDelegate(MaximumValueAttribute).Remove(MaximumAddValueDelegate);
		CachedAbilityComponent->GetGameplayAttributeValueChangeDelegate(MaximumValueAttribute).Remove(MaximumPercentValueDelegate);

		CurrentValueDelegate.Reset();
		MaximumValueDelegate.Reset();
		MaximumAddValueDelegate.Reset();
		MaximumPercentValueDelegate.Reset();
		
		CachedAbilityComponent = nullptr;
	}
}

void UNinjaCombatBaseOverheadBarWidget::HandleCurrentAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (IsValid(CachedAbilityComponent))
	{
		CurrentValue = OnAttributeChangeData.NewValue;
		MaximumTotalValue = CachedAbilityComponent->GetNumericAttribute(MaximumTotalValueAttribute);
		UpdateStatus(CurrentValue, MaximumTotalValue);	
	}	
}

void UNinjaCombatBaseOverheadBarWidget::HandleMaximumAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (IsValid(CachedAbilityComponent))
	{
		MaximumValue = OnAttributeChangeData.NewValue;
		MaximumTotalValue = CachedAbilityComponent->GetNumericAttribute(MaximumTotalValueAttribute);
		UpdateStatus(CurrentValue, MaximumTotalValue);	
	}
}

void UNinjaCombatBaseOverheadBarWidget::HandleMaximumAddAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (IsValid(CachedAbilityComponent))
	{
		MaximumAddValue = OnAttributeChangeData.NewValue;
		MaximumTotalValue = CachedAbilityComponent->GetNumericAttribute(MaximumTotalValueAttribute);
		UpdateStatus(CurrentValue, MaximumTotalValue);	
	}	
}

void UNinjaCombatBaseOverheadBarWidget::HandleMaximumPercentAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData)
{
	if (IsValid(CachedAbilityComponent))
	{
		MaximumPercentValue = OnAttributeChangeData.NewValue;
		MaximumTotalValue = CachedAbilityComponent->GetNumericAttribute(MaximumTotalValueAttribute);
		UpdateStatus(CurrentValue, MaximumTotalValue);	
	}	
}

void UNinjaCombatBaseOverheadBarWidget::HandleGameplayCue_Implementation(const FGameplayCueParameters& CueParameters)
{
	Super::HandleGameplayCue_Implementation(CueParameters);
	
	const bool bFatalDamage = CueParameters.AggregatedSourceTags.HasTagExact(Tag_Combat_Effect_Damage_Fatal);
	if (bFatalDamage)
	{
		if (bHideOnFatalHit)
		{
			HideWidget();	
		}
	}
	else
	{
		const UActorComponent* DamageManager = UNinjaCombatFunctionLibrary::GetDamageManagerComponent(GetCombatActor());
		if (IsValid(DamageManager) && !ICombatDamageManagerInterface::Execute_IsDead(DamageManager))
		{
			ShowWidget();
		}
	}
}

void UNinjaCombatBaseOverheadBarWidget::UpdateStatus_Implementation(const float CurrentValueReceived, const float MaximumValueReceived)
{
	if (IsValid(OverheadProgressBar))
	{
		const float Percent = MaximumValueReceived > 0.f ? CurrentValueReceived / MaximumValueReceived : 0.f;
		OverheadProgressBar->SetPercent(Percent);
	}
}
