// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Proxies/NinjaCombatAbilityActorInfoProxy.h"

#include "AbilitySystem/Types/FNinjaCombatGameplayAbilityActorInfo.h"
#include "Interfaces/Components/CombatComboManagerInterface.h"
#include "Interfaces/Components/CombatDamageManagerInterface.h"
#include "Interfaces/Components/CombatDefenseManagerInterface.h"
#include "Interfaces/Components/CombatMotionWarpingInterface.h"
#include "Interfaces/Components/CombatMovementManagerInterface.h"
#include "Interfaces/Components/CombatPhysicalAnimationInterface.h"
#include "Interfaces/Components/CombatTargetManagerInterface.h"
#include "Interfaces/Components/CombatWeaponManagerInterface.h"

UActorComponent* UNinjaCombatAbilityActorInfoProxy::GetComboManagerComponent(
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	check(ActorInfo != nullptr);
    
	const FNinjaCombatGameplayAbilityActorInfo* CombatActorInfo = static_cast<const FNinjaCombatGameplayAbilityActorInfo*>(ActorInfo);
	if (CombatActorInfo != nullptr && CombatActorInfo->ComboManagerComponent.IsValid())
	{
		check(CombatActorInfo->ComboManagerComponent->Implements<UCombatComboManagerInterface>());
		return CombatActorInfo->ComboManagerComponent.Get();
	}

	return nullptr;	
}

UActorComponent* UNinjaCombatAbilityActorInfoProxy::GetDamageManagerComponent(
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	check(ActorInfo != nullptr);
    
	const FNinjaCombatGameplayAbilityActorInfo* CombatActorInfo = static_cast<const FNinjaCombatGameplayAbilityActorInfo*>(ActorInfo);
	if (CombatActorInfo != nullptr && CombatActorInfo->DamageManagerComponent.IsValid())
	{
		check(CombatActorInfo->DamageManagerComponent->Implements<UCombatDamageManagerInterface>());
		return CombatActorInfo->DamageManagerComponent.Get();
	}

	return nullptr;		
}

UActorComponent* UNinjaCombatAbilityActorInfoProxy::GetDefenseManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const
{
	check(ActorInfo != nullptr);
    
	const FNinjaCombatGameplayAbilityActorInfo* CombatActorInfo = static_cast<const FNinjaCombatGameplayAbilityActorInfo*>(ActorInfo);
	if (CombatActorInfo != nullptr && CombatActorInfo->MotionWarpingComponent.IsValid())
	{
		check(CombatActorInfo->DefenseManagerComponent->Implements<UCombatDefenseManagerInterface>());
		return CombatActorInfo->DefenseManagerComponent.Get();
	}

	return nullptr;	
}

UActorComponent* UNinjaCombatAbilityActorInfoProxy::GetMotionWarpingComponent(
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	check(ActorInfo != nullptr);
    
	const FNinjaCombatGameplayAbilityActorInfo* CombatActorInfo = static_cast<const FNinjaCombatGameplayAbilityActorInfo*>(ActorInfo);
	if (CombatActorInfo != nullptr && CombatActorInfo->MotionWarpingComponent.IsValid())
	{
		check(CombatActorInfo->MotionWarpingComponent->Implements<UCombatMotionWarpingInterface>());
		return CombatActorInfo->MotionWarpingComponent.Get();
	}

	return nullptr;		
}

UActorComponent* UNinjaCombatAbilityActorInfoProxy::GetMovementManagerComponent(
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	check(ActorInfo != nullptr);
    
	const FNinjaCombatGameplayAbilityActorInfo* CombatActorInfo = static_cast<const FNinjaCombatGameplayAbilityActorInfo*>(ActorInfo);
	if (CombatActorInfo != nullptr && CombatActorInfo->MovementManagerComponent.IsValid())
	{
		check(CombatActorInfo->MovementManagerComponent->Implements<UCombatMovementManagerInterface>());
		return CombatActorInfo->MovementManagerComponent.Get();
	}

	return nullptr;	
}

UActorComponent* UNinjaCombatAbilityActorInfoProxy::GetPhysicalAnimationComponent(
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	check(ActorInfo != nullptr);
    
	const FNinjaCombatGameplayAbilityActorInfo* CombatActorInfo = static_cast<const FNinjaCombatGameplayAbilityActorInfo*>(ActorInfo);
	if (CombatActorInfo != nullptr && CombatActorInfo->PhysicalAnimationComponent.IsValid())
	{
		check(CombatActorInfo->PhysicalAnimationComponent->Implements<UCombatPhysicalAnimationInterface>());
		return CombatActorInfo->PhysicalAnimationComponent.Get();
	}

	return nullptr;		
}

UActorComponent* UNinjaCombatAbilityActorInfoProxy::GetTargetManagerComponent(
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	check(ActorInfo != nullptr);
    
	const FNinjaCombatGameplayAbilityActorInfo* CombatActorInfo = static_cast<const FNinjaCombatGameplayAbilityActorInfo*>(ActorInfo);
	if (CombatActorInfo != nullptr && CombatActorInfo->TargetManagerComponent.IsValid())
	{
		check(CombatActorInfo->TargetManagerComponent->Implements<UCombatTargetManagerInterface>());
		return CombatActorInfo->TargetManagerComponent.Get();
	}

	return nullptr;	
}

UActorComponent* UNinjaCombatAbilityActorInfoProxy::GetWeaponManagerComponent(
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	check(ActorInfo != nullptr);
    
	const FNinjaCombatGameplayAbilityActorInfo* CombatActorInfo = static_cast<const FNinjaCombatGameplayAbilityActorInfo*>(ActorInfo);
	if (CombatActorInfo != nullptr && CombatActorInfo->WeaponManagerComponent.IsValid())
	{
		check(CombatActorInfo->WeaponManagerComponent->Implements<UCombatWeaponManagerInterface>());
		return CombatActorInfo->WeaponManagerComponent.Get();
	}

	return nullptr;		
}
