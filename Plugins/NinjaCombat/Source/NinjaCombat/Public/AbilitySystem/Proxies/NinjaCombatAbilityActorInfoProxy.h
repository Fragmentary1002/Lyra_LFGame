// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Interfaces/CombatActorInfoProxyInterface.h"
#include "UObject/Object.h"
#include "NinjaCombatAbilityActorInfoProxy.generated.h"

/**
 * Default Proxy for an Ability Actor Info.
 */
UCLASS()
class NINJACOMBAT_API UNinjaCombatAbilityActorInfoProxy : public UObject, public ICombatActorInfoProxyInterface
{
	
	GENERATED_BODY()

public:

	// -- Begin Ability Actor Info Proxy implementation
	virtual UActorComponent* GetComboManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual UActorComponent* GetDamageManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual UActorComponent* GetDefenseManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual UActorComponent* GetMotionWarpingComponent(const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual UActorComponent* GetMovementManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual UActorComponent* GetPhysicalAnimationComponent(const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual UActorComponent* GetTargetManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const override;
	virtual UActorComponent* GetWeaponManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const override;
	// -- End Ability Actor Info Proxy implementation
	
};
