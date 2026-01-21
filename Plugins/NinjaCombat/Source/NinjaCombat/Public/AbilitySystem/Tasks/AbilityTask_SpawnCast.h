// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_SpawnCast.generated.h"

/**
 * Executes a cast.
 */
UCLASS()
class NINJACOMBAT_API UAbilityTask_SpawnCast : public UAbilityTask
{

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCastReady, AActor*, CastActor);

	GENERATED_BODY()

public:

	/** Broadcasts when . */
	UPROPERTY(BlueprintAssignable)
	FCastReady OnCastReady;

	UAbilityTask_SpawnCast();

	// -- Begin Ability Task implementation
	virtual void Activate() override;
	// -- End Ability Task implementation

	/**
	 * Creates the appropriate Ability Task, based on the provided data.
	 */
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"), Category = "Ability|Tasks")
	static UAbilityTask_SpawnCast* CreateTask(UGameplayAbility* OwningAbility,
		UPARAM(meta = (MustImplement = "/Script/NinjaCombat.CombatCastInterface")) TSubclassOf<AActor> CastClass,
		FTransform Transform, bool bEnableDebug = false);

protected:

	/** Class that will be spawned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Cast")
	TSubclassOf<AActor> CastClass;

	/** Transform used for the spawn. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawn Cast")
	FTransform CastTransform; 

	virtual void SpawnCastActor();

private:

	/** Sets this task to debug mode. */
	bool bEnableDebug;	
	
};
