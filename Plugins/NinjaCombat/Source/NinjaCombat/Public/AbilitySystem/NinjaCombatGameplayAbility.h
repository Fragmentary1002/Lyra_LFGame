// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Interfaces/CombatActorInfoProxyInterface.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "NinjaCombatGameplayAbility.generated.h"

class UAbilityTask_WaitGameplayEvent;
class UNinjaCombatManagerComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogNinjaCombatAbility, Log, All);

/**
 * Base Gameplay Ability that provides access to the Combat Component.
 */

//  在Ninja Combat中所有的Ability都需要继承自这个类
//  这边更改到ULyraGameplayAbility 为了适配Lyra 框架
UCLASS(Abstract)
class NINJACOMBAT_API UNinjaCombatGameplayAbility : public UGameplayAbility
{
	
	GENERATED_BODY()

public:

	UNinjaCombatGameplayAbility(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/**
	 * Provides the Combo Manager Component from the Actor Info.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	UActorComponent* GetComboManagerComponentFromActorInfo() const;

	/**
	 * Provides the Damage Manager Component from the Actor Info.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	UActorComponent* GetDamageManagerComponentFromActorInfo() const;

	/**
	 * Provides the Defense Manager Component from the Actor Info.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	UActorComponent* GetDefenseManagerComponentFromActorInfo() const;
	
	/**
	 * Provides the Motion Warping Component from the Actor Info.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	UActorComponent* GetMotionWarpingComponentFromActorInfo() const;

	/**
	 * Provides the Movement Manager Component from the Actor Info.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	UActorComponent* GetMovementManagerComponentFromActorInfo() const;
	
	/**
	 * Provides the Target Manager Component from the Actor Info.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	UActorComponent* GetTargetManagerComponentFromActorInfo() const;
	
	/**
	 * Provides the Weapon Manager Component from the Actor Info.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Abilities")
	UActorComponent* GetWeaponManagerComponentFromActorInfo() const;	
	
protected:

	/** Toggles debug mode on and off. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Ability|Debug")
	bool bEnableDebug;

	static const ICombatActorInfoProxyInterface* GetActorInfoProxy();

	/**
	 * Handles the initialization of the event task. It won't be active yet.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	UAbilityTask_WaitGameplayEvent* InitializeEventTask(FGameplayTag Event, bool bOnlyMatchExact = true);

	/**
	 * Helper method that can finish an array of latent tasks.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	static void FinishLatentTasks(const TArray<UAbilityTask*>& Tasks);

	/**
	 * Helper method that can finish a latent task.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	static void FinishLatentTask(UAbilityTask* Task);

	/**
	 * Adds a debug message, if debugging is activated.
	 * 
	 * @param Message
	 *		Message that will be displayed to the log and on-screen.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Abilities")
	virtual void AddDebugMessage(const FString& Message) const;

	/**
	 * Convenience method to handle event payloads.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Ability")
	void HandleEventReceived(FGameplayEventData Payload);
	virtual void HandleEventReceived_Implementation(FGameplayEventData Payload) { }
	
};
