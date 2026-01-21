// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "MotionWarpingComponent.h"
#include "Interfaces/Components/CombatMotionWarpingInterface.h"
#include "NinjaCombatMotionWarpingComponent.generated.h"

/**
 * Specialized version of the Motion Warping Component, integrated with the Combat System.
 */
UCLASS(Blueprintable, ClassGroup = NinjaCombat, meta = (BlueprintSpawnableComponent))
class NINJACOMBAT_API UNinjaCombatMotionWarpingComponent : public UMotionWarpingComponent, public ICombatMotionWarpingInterface
{
	
	GENERATED_BODY()

public:

	UNinjaCombatMotionWarpingComponent(const FObjectInitializer& ObjectInitializer);

	// -- Begin Motion Warping implementation
	virtual void SetCombatWarpTarget_Implementation(const FName WarpName, const AActor* Target, float Offset = 0.f) override;
	virtual void ClearCombatWarpTarget_Implementation(const FName WarpName) override;
	// -- End Motion Warping implementation
	
protected:
	
	/**
	 * Calculates the Warp Location for a target.
	 * 
	 * @param Target		Actor used as a reference for the Warp Location calculation. 
	 * @param Offset		Optional positive or negative offset added to the distance.
	 * @return				A Vector representing the desired location for the warp.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Motion Warping")
	FVector CalculateWarpLocation(const AActor* Target, float Offset = 0.f) const;

	/**
	 * Calculates the Warp Rotation for a target.
	 * 
	 * @param Target		Actor used as a reference for the Warp Rotation calculation. 
	 * @return				A Rotator representing the desired direction for the warp.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Motion Warping")
	FRotator CalculateWarpRotation(const AActor* Target) const;
	
};
