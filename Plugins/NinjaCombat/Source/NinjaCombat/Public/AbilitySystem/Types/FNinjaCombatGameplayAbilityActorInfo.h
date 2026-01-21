// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "Abilities/GameplayAbilityTypes.h"
#include "FNinjaCombatGameplayAbilityActorInfo.generated.h"

class UActorComponent;
class UNinjaCombatManagerComponent;

/**
 * Enhances the default actor info so it will also resolve combat objects.
 * 
 * This is not directly used by the Combat/Ability System. Instead, a proxy object is used so you
 * can use any other Actor Info struct as long as you provide an appropriate proxy for it.
 */
USTRUCT(BlueprintType)
struct FNinjaCombatGameplayAbilityActorInfo : public FGameplayAbilityActorInfo
{
    
    GENERATED_BODY()

    typedef Super FGameplayAbilityActorInfo;

    virtual ~FNinjaCombatGameplayAbilityActorInfo() override {}

	/** Combo Manager Component assigned to the actor. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat Info")
	TWeakObjectPtr<UActorComponent> ComboManagerComponent;

	/** Damage Manager Component assigned to the actor. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat Info")
	TWeakObjectPtr<UActorComponent> DamageManagerComponent;

	/** Defense Manager Component assigned to the actor. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat Info")
	TWeakObjectPtr<UActorComponent> DefenseManagerComponent;
	
	/** Motion Warping Component assigned to the actor. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat Info")
	TWeakObjectPtr<UActorComponent> MotionWarpingComponent;

	/** Movement Manager Component assigned to the actor. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat Info")
	TWeakObjectPtr<UActorComponent> MovementManagerComponent;
	
	/** Physical Animation Component assigned to the actor. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat Info")
	TWeakObjectPtr<UActorComponent> PhysicalAnimationComponent;

	/** Target Manager Component assigned to the actor. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat Info")
	TWeakObjectPtr<UActorComponent> TargetManagerComponent;
	
	/** Weapon Manager Component assigned to the actor. */
	UPROPERTY(BlueprintReadOnly, Category = "Combat Info")
	TWeakObjectPtr<UActorComponent> WeaponManagerComponent;
	
    // -- Begin Actor Info implementation
    virtual void InitFromActor(AActor* InOwnerActor, AActor* InAvatarActor, UAbilitySystemComponent* InAbilitySystemComponent) override;
    virtual void ClearActorInfo() override;
    // -- End Actor Info implementation
    
};