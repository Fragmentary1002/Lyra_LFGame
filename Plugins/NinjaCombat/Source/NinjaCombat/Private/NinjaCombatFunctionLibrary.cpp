// Ninja Bear Studio Inc., all rights reserved.
#include "NinjaCombatFunctionLibrary.h"

#include "AbilitySystemGlobals.h"
#include "GameplayCueManager.h"
#include "NiagaraSystem.h"
#include "NinjaCombatSettings.h"
#include "AbilitySystem/Interfaces/CombatEffectContextProxyInterface.h"
#include "Components/DecalComponent.h"
#include "Interfaces/CombatSystemInterface.h"
#include "Interfaces/Components/CombatComboManagerInterface.h"
#include "Interfaces/Components/CombatDamageManagerInterface.h"
#include "Interfaces/Components/CombatDefenseManagerInterface.h"
#include "Interfaces/Components/CombatMotionWarpingInterface.h"
#include "Interfaces/Components/CombatMovementManagerInterface.h"
#include "Interfaces/Components/CombatPhysicalAnimationInterface.h"
#include "Interfaces/Components/CombatTargetManagerInterface.h"
#include "Interfaces/Components/CombatWeaponManagerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

UActorComponent* UNinjaCombatFunctionLibrary::GetComboManagerComponent(const AActor* Owner)
{
	if (IsValid(Owner))
	{
		// Attempt to retrieve the component via the interface first.
		if (Owner->Implements<UCombatSystemInterface>())
		{
			UActorComponent* Component = ICombatSystemInterface::Execute_GetComboManagerComponent(Owner);
			if (IsValid(Component))
			{
				check(Component->Implements<UCombatComboManagerInterface>());
				return Component;
			}
		}

		// Retrieve the component directly.
		return Owner->FindComponentByInterface(UCombatComboManagerInterface::StaticClass());
	}
	
	return nullptr;
}

UActorComponent* UNinjaCombatFunctionLibrary::GetDamageManagerComponent(const AActor* Owner)
{
	if (IsValid(Owner))
	{
		// Attempt to retrieve the component via the interface first.
		if (Owner->Implements<UCombatSystemInterface>())
		{
			UActorComponent* Component = ICombatSystemInterface::Execute_GetDamageManagerComponent(Owner);
			if (IsValid(Component))
			{
				check(Component->Implements<UCombatDamageManagerInterface>());
				return Component;
			}			
		}

		// Retrieve the component directly.
		return Owner->FindComponentByInterface(UCombatDamageManagerInterface::StaticClass());
	}
	
	return nullptr;
}

UActorComponent* UNinjaCombatFunctionLibrary::GetDefenseManagerComponent(const AActor* Owner)
{
	if (IsValid(Owner))
	{
		// Attempt to retrieve the component via the interface first.
		if (Owner->Implements<UCombatSystemInterface>())
		{
			UActorComponent* Component = ICombatSystemInterface::Execute_GetDefenseManagerComponent(Owner);
			if (IsValid(Component))
			{
				check(Component->Implements<UCombatDefenseManagerInterface>());
				return Component;
			}			
		}

		// Retrieve the component directly.
		return Owner->FindComponentByInterface(UCombatDefenseManagerInterface::StaticClass());
	}
	
	return nullptr;
}

UActorComponent* UNinjaCombatFunctionLibrary::GetMotionWarpingComponent(const AActor* Owner)
{
	if (IsValid(Owner))
	{
		// Attempt to retrieve the component via the interface first.
		if (Owner->Implements<UCombatSystemInterface>())
		{
			UActorComponent* Component = ICombatSystemInterface::Execute_GetMotionWarpingComponent(Owner);
			if (IsValid(Component))
			{
				check(Component->Implements<UCombatMotionWarpingInterface>());
				return Component;
			}				
		}

		// Retrieve the component directly.
		return Owner->FindComponentByInterface(UCombatMotionWarpingInterface::StaticClass());
	}
	
	return nullptr;	
}

UActorComponent* UNinjaCombatFunctionLibrary::GetMovementManagerComponent(const AActor* Owner)
{
	if (IsValid(Owner))
	{
		// Attempt to retrieve the component via the interface first.
		if (Owner->Implements<UCombatSystemInterface>())
		{
			UActorComponent* Component = ICombatSystemInterface::Execute_GetMovementManagerComponent(Owner);
			if (IsValid(Component))
			{
				check(Component->Implements<UCombatMovementManagerInterface>());
				return Component;
			}				
		}

		// Retrieve the component directly.
		return Owner->FindComponentByInterface(UCombatMovementManagerInterface::StaticClass());
	}
	
	return nullptr;	
}

UActorComponent* UNinjaCombatFunctionLibrary::GetPhysicalAnimationComponent(const AActor* Owner)
{
	if (IsValid(Owner))
	{
		// Attempt to retrieve the component via the interface first.
		if (Owner->Implements<UCombatSystemInterface>())
		{
			UActorComponent* Component = ICombatSystemInterface::Execute_GetPhysicalAnimationComponent(Owner);
			if (IsValid(Component))
			{
				check(Component->Implements<UCombatPhysicalAnimationInterface>());
				return Component;
			}				
		}

		// Retrieve the component directly.
		return Owner->FindComponentByInterface(UCombatPhysicalAnimationInterface::StaticClass());
	}
	
	return nullptr;
}

UActorComponent* UNinjaCombatFunctionLibrary::GetTargetManagerComponent(const AActor* Owner)
{
	if (IsValid(Owner))
	{
		// Attempt to retrieve the component via the interface first.
		if (Owner->Implements<UCombatSystemInterface>())
		{
			UActorComponent* Component = ICombatSystemInterface::Execute_GetTargetManagerComponent(Owner);
			if (IsValid(Component))
			{
				check(Component->Implements<UCombatTargetManagerInterface>());
				return Component;
			}				
		}

		// Retrieve the component directly.
		return Owner->FindComponentByInterface(UCombatTargetManagerInterface::StaticClass());
	}
	
	return nullptr;
}

UActorComponent* UNinjaCombatFunctionLibrary::GetWeaponManagerComponent(const AActor* Owner)
{
	if (IsValid(Owner))
	{
		// Attempt to retrieve the component via the interface first.
		if (Owner->Implements<UCombatSystemInterface>())
		{
			UActorComponent* Component = ICombatSystemInterface::Execute_GetWeaponManagerComponent(Owner);
			if (IsValid(Component))
			{
				check(Component->Implements<UCombatWeaponManagerInterface>());
				return Component;
			}			
		}

		// Retrieve the component directly.
		return Owner->FindComponentByInterface(UCombatWeaponManagerInterface::StaticClass());
	}
	
	return nullptr;
}

TScriptInterface<ICombatMeleeInterface> UNinjaCombatFunctionLibrary::GetMeleeScript(UObject* Target)
{
	if (IsValid(Target) && Target->Implements<UCombatMeleeInterface>())
	{
		const TScriptInterface<ICombatMeleeInterface> MeleeScript(Target);
		return MeleeScript;
	}

	return nullptr;
}

TScriptInterface<ICombatRangedInterface> UNinjaCombatFunctionLibrary::GetRangedScript(UObject* Target)
{
	if (IsValid(Target) && Target->Implements<UCombatRangedInterface>())
	{
		 TScriptInterface<ICombatRangedInterface> RangedScript(Target);

		return RangedScript;
	}

	return nullptr;
}

void UNinjaCombatFunctionLibrary::BreakDamageStruct(const FDamageEntry& Entry, AActor*& Causer,
	AActor*& Instigator, bool& bIsCriticalHit, bool& bIsBreakerHit, bool& bIsStaggerHit, bool& bIsFatalHit, float& Timestamp,
	float& DamageApplied, float& MitigationCost, float& PreviousHealth, float& PreviousStamina,
	FGameplayTagContainer& SourceTags, FGameplayTagContainer& TargetTags)
{
	const ICombatEffectContextProxyInterface* Proxy = GetDefault<UNinjaCombatSettings>()->GetDefaultEffectContextProxy();
	const FGameplayEffectContextHandle& EffectContextHandle = Entry.EffectContext;
	const FGameplayEffectContext* EffectContext = EffectContextHandle.Get();
	
	Causer = EffectContextHandle.GetEffectCauser();
	Instigator = EffectContextHandle.GetInstigator();
	bIsCriticalHit = Entry.IsCriticalHit();
	bIsBreakerHit = Entry.IsBreakerHit();
	bIsStaggerHit = Entry.IsStaggerHit();
	bIsFatalHit = Entry.IsFatalHit();
	Timestamp = Entry.Timestamp;
	DamageApplied = Proxy->GetDamageApplied(EffectContext);
	MitigationCost = Proxy->GetMitigationCost(EffectContext);
	PreviousHealth = Proxy->GetPreviousHealthOnTarget(EffectContext);
	PreviousStamina = Proxy->GetPreviousStaminaOnTarget(EffectContext);
	SourceTags = Entry.CapturedSourceTags;
	TargetTags = Entry.CapturedTargetTags;
}

void UNinjaCombatFunctionLibrary::ExecuteGameplayCueLocally(AActor* Owner, const FGameplayTag GameplayCueTag,
	const FGameplayCueParameters& GameplayCueParameters)
{
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	CueManager->HandleGameplayCue(Owner, GameplayCueTag, EGameplayCueEvent::Type::Executed, GameplayCueParameters);
}

void UNinjaCombatFunctionLibrary::AddGameplayCueLocally(AActor* Owner, const FGameplayTag GameplayCueTag,
	const FGameplayCueParameters& GameplayCueParameters)
{
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	CueManager->HandleGameplayCue(Owner, GameplayCueTag, EGameplayCueEvent::Type::OnActive, GameplayCueParameters);
	CueManager->HandleGameplayCue(Owner, GameplayCueTag, EGameplayCueEvent::Type::WhileActive, GameplayCueParameters);
}

void UNinjaCombatFunctionLibrary::RemoveGameplayCueLocally(AActor* Owner, const FGameplayTag GameplayCueTag,
	const FGameplayCueParameters& GameplayCueParameters)
{
	UGameplayCueManager* CueManager = UAbilitySystemGlobals::Get().GetGameplayCueManager();
	CueManager->HandleGameplayCue(Owner, GameplayCueTag, EGameplayCueEvent::Type::Removed, GameplayCueParameters);
}

FGameplayCueParameters UNinjaCombatFunctionLibrary::CreateGameplayCueFromEffectContext(
	const FGameplayEffectContextHandle& EffectContextHandle)
{
	FGameplayCueParameters CueParameters = FGameplayCueParameters();
	
	CueParameters.SourceObject = EffectContextHandle.GetSourceObject();
	CueParameters.Instigator = EffectContextHandle.GetInstigator();
	CueParameters.EffectCauser = EffectContextHandle.GetEffectCauser();
	CueParameters.AbilityLevel = EffectContextHandle.GetAbilityLevel();
	CueParameters.EffectContext = EffectContextHandle;

	if (EffectContextHandle.GetHitResult())
	{
		CueParameters.Location = EffectContextHandle.GetHitResult()->ImpactPoint;
		CueParameters.Normal = EffectContextHandle.GetHitResult()->Normal;
		CueParameters.PhysicalMaterial = EffectContextHandle.GetHitResult()->PhysMaterial;
	}
    
	return CueParameters;
}

UDecalComponent* UNinjaCombatFunctionLibrary::SpawnBloodDecal(const UObject* WorldContextObject,
	const TArray<FBasicParticleData>& Data, const UNiagaraSystem* NiagaraSystem,
	const FVector& SimulationPositionOffset, const FVector& BaseSize, UMaterialInterface* DecalMaterial,
	const float DecalChance, const float ScreenSize, const float DecalLifeSpan, const float FadeOutDuration, const float FixedXSize,
	const FVector2D SizeModifierRange, const FVector2D LifespanModifierRange)
{
	check(WorldContextObject);

	UDecalComponent* DecalComponent = nullptr;
	
	if (!UKismetSystemLibrary::IsDedicatedServer(WorldContextObject) && IsValid(NiagaraSystem) && IsValid(DecalMaterial))
	{
		for (const FBasicParticleData& ParticleData : Data)
		{
			if (FMath::FRandRange(0.f, 1.f) <= DecalChance)
			{
				const FVector ImpactPosition = ParticleData.Position;
				const FVector ImpactVelocity = ParticleData.Velocity;
			
				FVector DecalSize = BaseSize * FMath::FRandRange(SizeModifierRange.X, SizeModifierRange.Y);
				DecalSize.X = FixedXSize;

				FRotator DecalRotation = ImpactVelocity.ToOrientationRotator();
				DecalRotation.Roll = FMath::FRandRange(0.f, 360.f);

				const float ActualLifeSpan = DecalLifeSpan * FMath::FRandRange(LifespanModifierRange.X, LifespanModifierRange.Y);
				
				DecalComponent = UGameplayStatics::SpawnDecalAtLocation(WorldContextObject, DecalMaterial,
					DecalSize, ImpactPosition, DecalRotation, ActualLifeSpan);

				if (IsValid(DecalComponent))
				{
					DecalComponent->SetFadeScreenSize(ScreenSize);
					DecalComponent->SetFadeIn(0.f, 0.2f);
					DecalComponent->SetFadeOut(ActualLifeSpan, FadeOutDuration, false);
				}
			}
		}
	}

	return DecalComponent;
}
