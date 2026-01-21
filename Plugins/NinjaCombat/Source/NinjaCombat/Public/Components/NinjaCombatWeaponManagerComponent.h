// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "NinjaCombatBaseComponent.h"
#include "Interfaces/Components/CombatWeaponManagerInterface.h"
#include "NinjaCombatWeaponManagerComponent.generated.h"

/**
 * Simple weapon manager for the combat system.
 */
UCLASS(Blueprintable, ClassGroup = NinjaCombat, meta = (BlueprintSpawnableComponent))
class NINJACOMBAT_API UNinjaCombatWeaponManagerComponent : public UNinjaCombatBaseComponent, public ICombatWeaponManagerInterface
{
	
	GENERATED_BODY()

public:

	UNinjaCombatWeaponManagerComponent();

	// -- Begin Actor Component implementation
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	// -- End Actor Component implementation

	// -- Begin Weapon Manager implementation
	virtual AActor* GetWeapon_Implementation(const FGameplayTagQuery& WeaponQuery) const override;
	// -- End Weapon Manager implementation
	
protected:

	/**
	 * A list of weapons currently maintained by this component.
	 *
	 * You can decide if you want to maintain this array using any additional logic that suit your needs
	 * (i.e. adding more methods to this component), or reacting via delegates to changes in a backing
	 * Inventory Manager.
	 *
	 * Regardless, keep in mind that this is still a pretty simple implementation and for more elaborate
	 * scenarios, you may want to create your own implementation of the Weapon Manager, using something
	 * more optimized like a Fast Array Replication.
	 */
	UPROPERTY(ReplicatedUsing = OnRep_Weapons)	
	TArray<TObjectPtr<AActor>> Weapons;
	
	/** All weapons granted to this repository. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon Component", meta = (MustImplement = "/Script/NinjaCombat.CombatWeaponInterface"))
	TArray<TSubclassOf<AActor>> DefaultWeaponClasses;

	/** Finds all actor components already attached to the character. */
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "Weapon Component")
	void LoadAttachedWeapons();
	
	/** Initializes the default weapons assigned to this component. */
	UFUNCTION(BlueprintNativeEvent, BlueprintAuthorityOnly, Category = "Weapon Component")
	void InitializeWeapons();
	
	/** Reacts to a change in the Default Weapons. */
	UFUNCTION(BlueprintNativeEvent, Category = "Weapon Component")
	void OnRep_Weapons();

};
