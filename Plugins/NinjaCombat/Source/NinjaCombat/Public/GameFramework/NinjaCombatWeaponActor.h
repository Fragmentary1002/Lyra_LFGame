// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/CombatProjectileProviderInterface.h"
#include "Interfaces/CombatWeaponInterface.h"
#include "NinjaCombatWeaponActor.generated.h"

/**
 * Base implementation for a Weapon.
 */
UCLASS(Abstract)
class NINJACOMBAT_API ANinjaCombatWeaponActor : public AActor, public ICombatWeaponInterface, public ICombatProjectileProviderInterface
{
	
	GENERATED_BODY()

public:

	ANinjaCombatWeaponActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Actor implementation
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// -- End Actor implementation
	
	// -- Begin Weapon implementation
	virtual FGameplayTagContainer GetWeaponTags_Implementation() const override;
	// -- End Weapon implementation

	// -- Begin Projectile Provider interface
	virtual bool GetProjectileClass_Implementation(TSubclassOf<AActor>& OutProjectileClass) const override;
	// -- End Projectile Provider interface

protected:

	/** Tags used to identify this weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", Replicated)
	FGameplayTagContainer WeaponTags;

	/** An optional projectile class that can be assigned to this weapon. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon", meta = (MustImplement = "/Script/NinjaCombat.CombatProjectileInterface"))
	TSubclassOf<AActor> ProjectileClass;

};
