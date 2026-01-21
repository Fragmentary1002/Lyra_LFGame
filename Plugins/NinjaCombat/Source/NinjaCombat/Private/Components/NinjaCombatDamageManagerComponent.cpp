// Ninja Bear Studio Inc., all rights reserved.
#include "Components/NinjaCombatDamageManagerComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NinjaCombatFunctionLibrary.h"
#include "NinjaCombatSettings.h"
#include "NinjaCombatTags.h"
#include "TimerManager.h"
#include "AbilitySystem/NinjaCombatAttributeSet.h"
#include "AbilitySystem/Interfaces/CombatEffectContextProxyInterface.h"
#include "Components/CapsuleComponent.h"
#include "Damage/NinjaCombatDamageHandler.h"
#include "Data/NinjaCombatDamageHandlerData.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/Components/CombatDefenseManagerInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

UNinjaCombatDamageManagerComponent::UNinjaCombatDamageManagerComponent()
	: DamageTakenList(this)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	bBindToAbilityComponent = true;
	SetIsReplicatedByDefault(true);	
}

void UNinjaCombatDamageManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ThisClass, DamageTakenList);
	DOREPLIFETIME_CONDITION(ThisClass, bIsDead, COND_SkipOwner);
}

void UNinjaCombatDamageManagerComponent::OnAbilitySystemComponentReceived()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	if (IsValid(AbilitySystemComponent))
	{
		DeathTagHandle = AbilitySystemComponent->RegisterGameplayTagEvent(Tag_Combat_State_Dead, EGameplayTagEventType::NewOrRemoved)
			.AddUObject(this, &ThisClass::HandleTagChanged);	
	}
}

void UNinjaCombatDamageManagerComponent::OnAbilitySystemComponentReset()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	if (IsValid(AbilitySystemComponent))
	{
		if (DeathTagHandle.IsValid())
		{
			AbilitySystemComponent->UnregisterGameplayTagEvent(DeathTagHandle, Tag_Combat_State_Dead, EGameplayTagEventType::NewOrRemoved);
			DeathTagHandle.Reset();
		}
	}
}

bool UNinjaCombatDamageManagerComponent::IsDead_Implementation() const
{
	return bIsDead || HasGameplayTag(Tag_Combat_State_Dead);
}

void UNinjaCombatDamageManagerComponent::HandleTagChanged(const FGameplayTag CallbackTag, const int32 NewCount)
{
	// Handles the scenario where death is determined by the presence of the tag from external reasons.
	if (CallbackTag == Tag_Combat_State_Dead)
	{
		bIsDead = NewCount > 0;
	}
}

void UNinjaCombatDamageManagerComponent::CalculateDamage_Implementation(const FDamageCalculationInput& Input, FDamageCalculationOutput& Output) const
{
	const UActorComponent* DefenseComponent = UNinjaCombatFunctionLibrary::GetDefenseManagerComponent(GetOwner());
	if (IsValid(DefenseComponent) && ensure(DefenseComponent->Implements<UCombatDefenseManagerInterface>()))
	{
		return ICombatDefenseManagerInterface::Execute_DefendDamage(DefenseComponent, Input, Output);
	}
}

void UNinjaCombatDamageManagerComponent::RegisterDamageReceived_Implementation(const FGameplayEffectSpec& EffectSpec)
{
	if (OwnerHasAuthority())
	{
		DamageTakenList.RegisterDamageTaken(EffectSpec);
	}
}

void UNinjaCombatDamageManagerComponent::HandleDamageReceived_Implementation(const FDamageEntry& DamageEntry)
{
	// Handles the scenario where death happened by damage applied.
	if (DamageEntry.IsFatalHit())
	{
		const TObjectPtr<APawn> MyPawn = Cast<APawn>(GetOwner());
		if (IsValid(MyPawn) && (MyPawn->HasAuthority() || MyPawn->IsLocallyControlled()))
		{
			bIsDead = true;
			OnRep_IsDead();
		}
	}
	
	BroadcastDamageGameplayEvent(DamageEntry);
	PlayDamageGameplayCue(DamageEntry);

	if (OnDamageReceived.IsBound())
	{
		AActor* EffectCauser = DamageEntry.GetDamageCauser();
		OnDamageReceived.Broadcast(EffectCauser, GetOwner(), DamageEntry);	
	}	
}

void UNinjaCombatDamageManagerComponent::BroadcastDamageGameplayEvent(const FDamageEntry& DamageEntry)
{
	UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	if (IsValid(AbilitySystemComponent) && (OwnerHasAuthority() || OwnerIsLocallyControlled()))
	{
		const ICombatEffectContextProxyInterface* Proxy = GetDefault<UNinjaCombatSettings>()->GetDefaultEffectContextProxy();
		check(Proxy != nullptr);

		const FGameplayEffectContext* EffectContext = DamageEntry.EffectContext.Get();
		const float Magnitude = Proxy->GetDamageApplied(EffectContext);
		
		const FGameplayTag EventTag = DamageEntry.IsFatalHit() ? Tag_Combat_Event_Death :
			DamageEntry.IsStaggerHit() ? Tag_Combat_Event_Damage_Stagger : Tag_Combat_Event_Damage_Received;
		
		FGameplayEventData* Payload = new FGameplayEventData();
		Payload->EventTag = EventTag;
		Payload->Instigator = DamageEntry.EffectContext.GetInstigator();
		Payload->Target = GetOwner();
		Payload->EventMagnitude = Magnitude;
		Payload->ContextHandle = DamageEntry.EffectContext;
		Payload->InstigatorTags = DamageEntry.CapturedSourceTags;
		Payload->TargetTags = DamageEntry.CapturedTargetTags;
		
		AbilitySystemComponent->HandleGameplayEvent(EventTag, Payload);
	}
}

void UNinjaCombatDamageManagerComponent::PlayDamageGameplayCue(const FDamageEntry& DamageEntry)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		const FGameplayCueParameters CueParameters = DamageEntry.GetGameplayCueParameters();
		UNinjaCombatFunctionLibrary::ExecuteGameplayCueLocally(GetOwner(), Tag_GameplayCue_Combat_Hit, CueParameters);
		
		if (DamageEntry.IsFatalHit())
		{
			UNinjaCombatFunctionLibrary::ExecuteGameplayCueLocally(GetOwner(), Tag_GameplayCue_Combat_Death, CueParameters);
		}
	}	
}

void UNinjaCombatDamageManagerComponent::BindToDamageReceivedDelegate_Implementation(const FDamageReceivedDelegate& Delegate)
{
	OnDamageReceived.Add(Delegate);
}

void UNinjaCombatDamageManagerComponent::BindToOwnerDiedDelegate_Implementation(const FOwnerDiedDelegate& Delegate)
{
	OnOwnerDeath.Add(Delegate);
}

void UNinjaCombatDamageManagerComponent::UnbindFromDamageReceivedDelegate_Implementation(const UObject* Source)
{
	OnDamageReceived.RemoveAll(Source);
}

void UNinjaCombatDamageManagerComponent::UnbindFromOwnerDiedDelegate_Implementation(const UObject* Source)
{
	OnOwnerDeath.RemoveAll(Source);
}

void UNinjaCombatDamageManagerComponent::OnRep_IsDead_Implementation()
{
	Execute_StartDying(this);
}

void UNinjaCombatDamageManagerComponent::StartDying_Implementation()
{
	UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->AddLooseGameplayTag(Tag_Combat_State_Dead);

		UGameplayAbility* DeathAbility = GetDeathAbility();
		AbilitySystemComponent->CancelAbilities(nullptr, nullptr, DeathAbility);
	}

	const ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	if (IsValid(CharacterOwner))
	{
		CharacterOwner->GetCapsuleComponent()->CanCharacterStepUpOn = ECB_Yes;
		CharacterOwner->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		CharacterOwner->GetCharacterMovement()->GravityScale = 0;
		CharacterOwner->GetMovementComponent()->StopMovementImmediately();
	}

	if (OnOwnerDeath.IsBound())
	{
		OnOwnerDeath.Broadcast();
	}
}

UGameplayAbility* UNinjaCombatDamageManagerComponent::GetDeathAbility() const
{
	const UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayTagContainer Tags = FGameplayTagContainer::EmptyContainer;
		Tags.AddTagFast(Tag_Combat_Ability_Death);

		TArray<FGameplayAbilitySpecHandle> SpecHandles;
		AbilitySystemComponent->FindAllAbilitiesWithTags(SpecHandles, Tags);

		if (SpecHandles.Num() > 0)
		{
			const FGameplayAbilitySpec* Spec = AbilitySystemComponent->FindAbilitySpecFromHandle(SpecHandles[0]);
			if (Spec != nullptr)
			{
				return Spec->Ability;
			}
		}
	}

	return nullptr;
}

void UNinjaCombatDamageManagerComponent::FinishDying_Implementation()
{
	AActor* OwnerActor = GetOwner();
	if (IsValid(OwnerActor))
	{
		if (OwnerHasAuthority())
		{
			UAbilitySystemComponent* AbilitySystemComponent = GetCachedAbilitySystemComponent();
			if (IsValid(AbilitySystemComponent))
			{
				AbilitySystemComponent->ClearActorInfo();
			}
	
			APawn* PawnOwner = Cast<APawn>(GetOwner());
			if (IsValid(PawnOwner))
			{
				PawnOwner->DetachFromControllerPendingDestroy();
			}
		}

		OwnerActor->Destroy();
	}
}

void UNinjaCombatDamageManagerComponent::GameplayCue_Combat_Hit_Implementation(const EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	if (EventType == EGameplayCueEvent::Executed)
	{
		ExecuteDamageHandlers(Parameters);
	}
}

void UNinjaCombatDamageManagerComponent::GameplayCue_Combat_Death_Implementation(const EGameplayCueEvent::Type EventType, const FGameplayCueParameters& Parameters)
{
	if (EventType == EGameplayCueEvent::Executed)
	{
		ExecuteDamageHandlers(Parameters);
	}
}

void UNinjaCombatDamageManagerComponent::ExecuteDamageHandlers(const FGameplayCueParameters& Parameters)
{
	if (IsValid(DamageHandlerData))
	{
		for (const UNinjaCombatDamageHandler* Handler : DamageHandlerData->DamageHandlers)
		{
			Handler->HandleDamage(this, Parameters);
		}
	}
}
