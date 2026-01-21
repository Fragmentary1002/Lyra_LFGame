// Ninja Bear Studio Inc., all rights reserved.
#include "Components/NinjaCombatDefenseManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NinjaCombatTags.h"
#include "TimerManager.h"
#include "AbilitySystem/NinjaCombatAttributeSet.h"
#include "AbilitySystem/Effects/CombatEffect_LastStand.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/CombatDamageModifierInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

UNinjaCombatDefenseManagerComponent::UNinjaCombatDefenseManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bBindToAbilityComponent = true;
	
	BlockAngleAttribute = UNinjaCombatAttributeSet::GetBlockAngleAttribute();
	LastStandCountAttribute = UNinjaCombatAttributeSet::GetLastStandCountAttribute();
	LastStandHealthPercentAttribute = UNinjaCombatAttributeSet::GetLastStandHealthPercentAttribute();
	LastStandEffectClass = UCombatEffect_LastStand::StaticClass();
	
	SetIsReplicatedByDefault(true);
}

void UNinjaCombatDefenseManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, bIsBlocking, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, bIsInvulnerable, COND_SkipOwner);
}

bool UNinjaCombatDefenseManagerComponent::IsBlocking_Implementation() const
{
	return bIsBlocking || HasGameplayTag(Tag_Combat_State_Blocking);
}

bool UNinjaCombatDefenseManagerComponent::CanBlock_Implementation(const AActor* DamageInstigator) const
{
	if (Execute_IsBlocking(this) && IsValid(DamageInstigator))
	{
		const FVector MyLocation = GetOwner()->GetActorLocation();
		const FVector EnemyLocation = DamageInstigator->GetActorLocation();
		const FVector EnemyForward = DamageInstigator->GetActorForwardVector();

		FVector A = MyLocation - EnemyLocation;
		A.Normalize();
		const float DotProduct = A.Dot(EnemyForward);
		const float Angle = UKismetMathLibrary::DegAcos(DotProduct);
		
		return FMath::Abs(Angle) <= GetBlockAngle();
	}

	return false;
}

bool UNinjaCombatDefenseManagerComponent::IsInvulnerable_Implementation() const
{
	return bIsInvulnerable || HasGameplayTag(Tag_Combat_State_Invulnerable);
}

void UNinjaCombatDefenseManagerComponent::OnAbilitySystemComponentReceived()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	if (IsValid(AbilitySystemComponent))
	{
		BlockingTagHandle = AbilitySystemComponent->RegisterGameplayTagEvent(Tag_Combat_State_Blocking, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ThisClass::HandleTagChanged);

		InvulnerabilityTagHandle = AbilitySystemComponent->RegisterGameplayTagEvent(Tag_Combat_State_Invulnerable, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ThisClass::HandleTagChanged);

		if (OwnerHasAuthority() || OwnerIsLocallyControlled())
		{
			bIsBlocking = AbilitySystemComponent->HasMatchingGameplayTag(Tag_Combat_State_Blocking);
			OnRep_IsBlocking(false);

			bIsInvulnerable = AbilitySystemComponent->HasMatchingGameplayTag(Tag_Combat_State_Invulnerable);
			OnRep_IsInvulnerable(false);
		}
	}
}

void UNinjaCombatDefenseManagerComponent::OnAbilitySystemComponentReset()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	if (IsValid(AbilitySystemComponent))
	{
		if (BlockingTagHandle.IsValid())
		{
			AbilitySystemComponent->UnregisterGameplayTagEvent(BlockingTagHandle, Tag_Combat_State_Blocking, EGameplayTagEventType::NewOrRemoved);
			BlockingTagHandle.Reset();
			bIsBlocking = false;
		}

		if (InvulnerabilityTagHandle.IsValid())
		{
			AbilitySystemComponent->UnregisterGameplayTagEvent(BlockingTagHandle, Tag_Combat_State_Invulnerable, EGameplayTagEventType::NewOrRemoved);
			InvulnerabilityTagHandle.Reset();
			bIsInvulnerable = false;
		}		
	}
}

void UNinjaCombatDefenseManagerComponent::HandleTagChanged(const FGameplayTag CallbackTag, const int32 NewCount)
{
	// There is no external access to the blocking and invulnerability properties. They are updated from
	// the Gameplay Ability System, by the presence of specific Gameplay Tags. We are deliberately allowing
	// local players to predict their blocking and invulnerability states, to improve local feedback.
	
	if (OwnerHasAuthority() || OwnerIsLocallyControlled())
	{
		if (CallbackTag == Tag_Combat_State_Blocking)
		{
			const bool bWasBlocking = bIsBlocking; 
			bIsBlocking = NewCount > 0;
			OnRep_IsBlocking(bWasBlocking);
		}
		else if (CallbackTag == Tag_Combat_State_Invulnerable)
		{
			const bool bWasInvulnerable = bIsInvulnerable;
			bIsInvulnerable = NewCount > 0;
			OnRep_IsInvulnerable(bWasInvulnerable);
		}
	}
}

float UNinjaCombatDefenseManagerComponent::GetBlockAngle_Implementation() const
{
	const UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	check(IsValid(AbilitySystemComponent));
	return AbilitySystemComponent->GetNumericAttribute(BlockAngleAttribute);
}

bool UNinjaCombatDefenseManagerComponent::CheckLastStand_Implementation() const
{
	// The default implementation for a last stand simply looks at the owner's attributes.
	//
	// You can actually go a long way with this implementation. Items in your inventory that provide a Last
	// Stand can simply add to these attributes when equipped. Abilities can execute a similar approach.
	//
	// Of course, you can fully rewrite this method if you needed, to implement more specific needs.
	
	const UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	if (IsValid(AbilitySystemComponent) && OwnerHasAuthority())
	{
		const float LastStandCount = AbilitySystemComponent->GetNumericAttribute(LastStandCountAttribute);
		const float LastStandHealth = AbilitySystemComponent->GetNumericAttribute(LastStandHealthPercentAttribute);
		return LastStandCount > 0.f && LastStandHealth > 0.f;
	}

	return false;
}

void UNinjaCombatDefenseManagerComponent::CommitLastStand_Implementation()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	if (IsValid(LastStandEffectClass) && IsValid(AbilitySystemComponent) && OwnerHasAuthority())
	{
		const FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		const UGameplayEffect* GameplayEffect = LastStandEffectClass->GetDefaultObject<UGameplayEffect>();

		static constexpr float EffectLevel = 1.f;
		AbilitySystemComponent->ApplyGameplayEffectToSelf(GameplayEffect, EffectLevel, EffectContext);
	}
}

void UNinjaCombatDefenseManagerComponent::DefendDamage_Implementation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const
{
	const AActor* TargetActor = Input.GetTargetActor();
	check(IsValid(TargetActor));

	Output.OriginalDamage = Input.Damage;
	Output.bInvulnerable = Execute_IsInvulnerable(this); 
	
	if (!Output.bInvulnerable)
	{
		ModifyIncomingDamage(Input, Output);
		ApplyBlockMitigation(Input, Output);
		ApplyDefenseMitigation(Input, Output);
		ApplyArmorMitigation(Input, Output);
		
		const float Damage = Output.GetAppliedDamage();
		if (Damage > 0.f)
		{
			const float PreviousHealth = Input.CurrentHealth;
			const float CurrentHealth = FMath::Max(PreviousHealth - Damage, 0.f);
			
			Output.Health = CurrentHealth;
			Output.bIsFatal = PreviousHealth > 0.f && CurrentHealth == 0.f;

			if (Output.bIsFatal)
			{
				ApplyLastStand(Input, Output);
			}
		}
	}
}

void UNinjaCombatDefenseManagerComponent::ModifyIncomingDamage(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const
{
	float Damage = Input.Damage;

	const AActor* Instigator = Input.GetDamageInstigator();
	const AActor* Target = Input.GetTargetActor();

	const FGameplayTagContainer SourceTags = Input.SourceTags;
	const FGameplayTagContainer TargetTags = Input.TargetTags;
	
	const AGameModeBase* GameMode = GetWorld()->GetAuthGameMode();
	if (IsValid(GameMode) && GameMode->Implements<UCombatDamageModifierInterface>())
	{
		Damage = ICombatDamageModifierInterface::Execute_ModifyDamage(GameMode, Damage, Instigator, Target, SourceTags, TargetTags);
		Output.ModifiedDamage = Damage;
	}

	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (IsValid(GameState) && GameState->Implements<UCombatDamageModifierInterface>())
	{
		Damage = ICombatDamageModifierInterface::Execute_ModifyDamage(GameState, Damage, Instigator, Target, SourceTags, TargetTags);
		Output.ModifiedDamage = Damage;
	}
}

void UNinjaCombatDefenseManagerComponent::ApplyBlockMitigation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const
{
	if (Execute_CanBlock(this, Input.GetDamageInstigator()))
	{
		const float Damage = Output.GetAppliedDamage();
		const float BlockRoll = Roll();
		const float BlockChance = Input.BlockChance;
		const float BlockMitigation = Input.BlockMitigation;
		const float BlockStaminaCostRate = Input.BlockStaminaCostRate;
	
		if (Damage > 0.f && BlockChance > 0.f && BlockMitigation > 0.f && BlockRoll <= BlockChance)
		{
			const float AvailableStamina = Input.CurrentStamina - Output.GetMitigationCost();
			float DamageMitigatedFromBlocking;
			
			if (BlockStaminaCostRate == 0.f)
			{
				// Blocking this damage is free. Just apply the block reduction to the incoming damage.
				DamageMitigatedFromBlocking = Damage * BlockMitigation;
			}
			else
			{
				// The amount damage we can actually block is constrained by both our current Block attribute
				// and the amount of available Stamina. Whichever is the smallest is used for the mitigation.
				DamageMitigatedFromBlocking = FMath::Min(Damage * BlockMitigation, AvailableStamina / BlockStaminaCostRate);
			}

			const float BlockLimit = Input.BlockLimit;
			if (BlockLimit > 0.f)
			{
				// We have a block limit, so apply to the mitigated the damage, before calculating the cost.
				DamageMitigatedFromBlocking = FMath::Min(DamageMitigatedFromBlocking, BlockLimit);
			}
			
			// Stamina cost to mitigate this damage, via blocking.
			Output.DamageMitigatedFromBlocking = DamageMitigatedFromBlocking;
			const float MitigationCost = Output.DamageMitigatedFromBlocking * BlockStaminaCostRate;

			Output.BlockStaminaCost = MitigationCost;
			Output.Stamina = FMath::Max(0.f, AvailableStamina - MitigationCost);
			Output.bIsBlocked = DamageMitigatedFromBlocking > 0.f;
			Output.bIsBreaker = AvailableStamina > 0.f && Output.Stamina == 0.f; 
		}
	}
}

void UNinjaCombatDefenseManagerComponent::ApplyDefenseMitigation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const
{
	const float DefenseRoll = Roll();
	const float Damage = Output.GetAppliedDamage();
	const float DefenseChance = Input.DefenseChance;
	const float DefenseMitigation = Input.DefenseMitigation;
	const float DefenseStaminaCostRate = Input.DefenseStaminaCostRate;
	
	if (Damage > 0.f && DefenseChance > 0.f && DefenseMitigation > 0.f && DefenseRoll <= DefenseChance)
	{
		const float AvailableStamina = Input.CurrentStamina - Output.GetMitigationCost();
		float DamageMitigatedFromDefense;
		
		if (DefenseStaminaCostRate == 0.f)
		{
			// Mitigating damage is free. Just apply the defense reduction to the incoming damage.
			DamageMitigatedFromDefense = Damage * DefenseMitigation;
		}
		else
		{
			// The amount damage we can actually mitigate is constrained by both our current Defense attribute
			// and the amount of available Stamina. Whichever is the smallest is used for the mitigation.
			DamageMitigatedFromDefense = FMath::Min(Damage * DefenseMitigation, AvailableStamina / DefenseStaminaCostRate);
		}

		const float DefenseLimit = Input.DefenseLimit;
		if (DefenseLimit > 0.f)
		{
			// We have a defense limit, so apply to the mitigated the damage, before calculating the cost.
			DamageMitigatedFromDefense = FMath::Min(DamageMitigatedFromDefense, DefenseLimit);
		}
		
		// Stamina cost to mitigate this damage, via generic mitigation.
		Output.DamageMitigatedFromDefense = DamageMitigatedFromDefense;
		const float MitigationCost = Output.DamageMitigatedFromDefense * DefenseStaminaCostRate;

		Output.DefenseStaminaCost = MitigationCost;
		Output.Stamina = FMath::Max(0.f, AvailableStamina - MitigationCost);
		Output.bIsDefended = DamageMitigatedFromDefense > 0.f;
	}	
}

void UNinjaCombatDefenseManagerComponent::ApplyArmorMitigation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const
{
	const float Damage = Output.GetAppliedDamage();
	const float ArmorReduction = Input.ArmorReduction;
	
	if (Damage > 0.f && ArmorReduction > 0.f)
	{
		const float ArmorMitigation = FMath::Min(Damage, ArmorReduction);
		Output.DamageMitigatedFromArmor = FMath::Max(0.f, Damage - ArmorMitigation);
	}	
}

void UNinjaCombatDefenseManagerComponent::ApplyLastStand(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const
{
	if (Execute_CheckLastStand(this))
	{
		Output.bIsFatal = false;
		Output.bIsLastStand = true;
	}
}

void UNinjaCombatDefenseManagerComponent::OnRep_IsBlocking_Implementation(const bool bWasBlocking)
{
	if (OnBlockingStateChanged.IsBound())
	{
		OnBlockingStateChanged.Broadcast(bIsBlocking);
	}	
}

void UNinjaCombatDefenseManagerComponent::OnRep_IsInvulnerable_Implementation(const bool bWasInvulnerable)
{
	if (OnInvulnerabilityStateChanged.IsBound())
	{
		OnInvulnerabilityStateChanged.Broadcast(bIsInvulnerable);
	}
}

void UNinjaCombatDefenseManagerComponent::BindToBlockingStateChangedDelegate_Implementation(const FBlockingStateChangedDelegate& Delegate)
{
	OnBlockingStateChanged.AddUnique(Delegate);
}

void UNinjaCombatDefenseManagerComponent::BindToInvulnerabilityStateChangedDelegate_Implementation(const FInvulnerabilityStateChangedDelegate& Delegate)
{
	OnInvulnerabilityStateChanged.AddUnique(Delegate);
}

void UNinjaCombatDefenseManagerComponent::UnbindFromBlockingStateChangedDelegate_Implementation(const UObject* Source)
{
	OnBlockingStateChanged.RemoveAll(Source);
}

void UNinjaCombatDefenseManagerComponent::UnbindFromInvulnerabilityStateChangedDelegate_Implementation(const UObject* Source)
{
	OnInvulnerabilityStateChanged.RemoveAll(Source);
}
