// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "NinjaCombatComboSetupData.generated.h"

class UInputAction;
class UStateTree;

/**
 * Aggregates assets and data related to a combo.
 */
UCLASS()
class NINJACOMBAT_API UNinjaCombatComboSetupData : public UPrimaryDataAsset
{

	GENERATED_BODY()

public:

	/** State Tree representing the Combo. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TObjectPtr<UStateTree> ComboTree;

	/** All mappings between Input Actions and Event Gameplay Tags. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TMap<TObjectPtr<const UInputAction>, FGameplayTag> ComboEvents; 

	// -- Begin Primary Data Asset implementation
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	// -- End Primary Data Asset implementation
	
	/** Provides a Gameplay Tag for a given Input Action. */
	FGameplayTag GetComboEventTag(const UInputAction* Action) const;
	
};
