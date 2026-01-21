// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NinjaCombatDamageHandlerData.generated.h"

class UNinjaCombatDamageHandler;

/**
 * Configures handlers for received damage.
 */
UCLASS()
class NINJACOMBAT_API UNinjaCombatDamageHandlerData : public UPrimaryDataAsset
{
	
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Damage Handler")
	TArray<TObjectPtr<UNinjaCombatDamageHandler>> DamageHandlers;
	
	// -- Begin Primary Data Asset implementation
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	// -- End Primary Data Asset implementation
	
};
