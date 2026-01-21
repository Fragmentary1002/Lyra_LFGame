// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatActorInfoProxyInterface.generated.h"

struct FGameplayAbilityActorInfo;

class UActorComponent;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UCombatActorInfoProxyInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Basic definition of a proxy that provides actor info for the ability system.
 */
class NINJACOMBAT_API ICombatActorInfoProxyInterface
{
	
	GENERATED_BODY()

public:

	/** Provides the Combo Manager Component from the Actor Info. */
	virtual UActorComponent* GetComboManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const = 0;

	/** Provides the Damage Manager Component from the Actor Info. */
	virtual UActorComponent* GetDamageManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const = 0;

	/** Provides the Defense Manager Component from the Actor Info. */
	virtual UActorComponent* GetDefenseManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const = 0;
	
	/** Provides the Motion Warping Component from the Actor Info. */
	virtual UActorComponent* GetMotionWarpingComponent(const FGameplayAbilityActorInfo* ActorInfo) const = 0;

	/** Provides the Movement Manager Component from the Actor Info. */
	virtual UActorComponent* GetMovementManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const = 0;
	
	/** Provides the Physical Animation Component from the Actor Info. */
	virtual UActorComponent* GetPhysicalAnimationComponent(const FGameplayAbilityActorInfo* ActorInfo) const = 0;

	/** Provides the Target Manager Component from the Actor Info. */
	virtual UActorComponent* GetTargetManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const = 0;
	
	/** Provides the Weapon Manager Component from the Actor Info. */
	virtual UActorComponent* GetWeaponManagerComponent(const FGameplayAbilityActorInfo* ActorInfo) const = 0;
	
};