// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CombatAnimationMontageProviderInterface.generated.h"

class UAnimMontage;

UINTERFACE(MinimalAPI)
class UCombatAnimationMontageProviderInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Defines an object that can receive a combat target.
 */
class NINJACOMBAT_API ICombatAnimationMontageProviderInterface
{
	
	GENERATED_BODY()

public:

	/**
	 * Provides an Animation Montage to be used by a combat ability.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Animation Montage Provider Interface")
	UAnimMontage* GetAnimationMontage() const;
	virtual UAnimMontage* GetAnimationMontage_Implementation() const { return nullptr; }

	/**
	 * Provides the Section Name to be used by a combat ability.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Animation Montage Provider Interface")
	FName GetSectionName() const;
	virtual FName GetSectionName_Implementation() const { return NAME_None; }
	
};