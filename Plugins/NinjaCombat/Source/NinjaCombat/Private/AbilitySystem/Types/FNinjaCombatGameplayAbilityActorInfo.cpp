// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Types/FNinjaCombatGameplayAbilityActorInfo.h"

#include "NinjaCombatFunctionLibrary.h"

void FNinjaCombatGameplayAbilityActorInfo::InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent)
{
    // Using typedef Super from header.
    Super::InitFromActor(InOwnerActor, InAvatarActor, InAbilitySystemComponent);

    if (InAvatarActor != nullptr)
    {
    	ComboManagerComponent = UNinjaCombatFunctionLibrary::GetComboManagerComponent(InAvatarActor);
    	DamageManagerComponent = UNinjaCombatFunctionLibrary::GetDamageManagerComponent(InAvatarActor);
    	DefenseManagerComponent = UNinjaCombatFunctionLibrary::GetDefenseManagerComponent(InAvatarActor);
    	MotionWarpingComponent = UNinjaCombatFunctionLibrary::GetMotionWarpingComponent(InAvatarActor);
    	MovementManagerComponent = UNinjaCombatFunctionLibrary::GetMovementManagerComponent(InAvatarActor);
    	PhysicalAnimationComponent = UNinjaCombatFunctionLibrary::GetPhysicalAnimationComponent(InAvatarActor);
    	TargetManagerComponent = UNinjaCombatFunctionLibrary::GetTargetManagerComponent(InAvatarActor);
    	WeaponManagerComponent = UNinjaCombatFunctionLibrary::GetWeaponManagerComponent(InAvatarActor);
    }
}

void FNinjaCombatGameplayAbilityActorInfo::ClearActorInfo()
{
    // Using typedef Super from header.
    Super::ClearActorInfo();
	
	ComboManagerComponent.Reset();
	DamageManagerComponent.Reset();
	DefenseManagerComponent.Reset();
	MotionWarpingComponent.Reset();
	MovementManagerComponent.Reset();
	PhysicalAnimationComponent.Reset();
	TargetManagerComponent.Reset();
	WeaponManagerComponent.Reset();	
}
