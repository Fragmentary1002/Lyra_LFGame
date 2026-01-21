// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/CombatAbility_Evade.h"
#include "AbilitySystem/Providers/NinjaCombatAnimationProvider.h"
#include "Kismet/KismetMathLibrary.h"
#include "AnimationProvider_DirectionalEvade.generated.h"

/** Configuration Class for the Evade Direction. */
USTRUCT(BlueprintType)
struct FEvadeDirection
{
	GENERATED_BODY()

	/** Range for this direction. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evade Direction")
	FVector2D Range;

	/** Name of the montage section. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Evade Direction")
	FName SectionName;
	
	FEvadeDirection()
	{
		Range = FVector2D::ZeroVector;
		SectionName = NAME_None;
	}

	FEvadeDirection(const float Min, const float Max, const FName SectionName) :
		Range(Min, Max), SectionName(SectionName)
	{
	}

	/** Checks if the provided angle matches this configuration range. */
	bool InRange(const float Angle) const
	{
		return UKismetMathLibrary::InRange_FloatFloat(Angle, Range.X, Range.Y);		
	}
};

/**
 * Provides appropriate section names for their respective evade angles. 
 */
UCLASS(meta = (DisplayName = "Directional Evades"))
class NINJACOMBAT_API UAnimationProvider_DirectionalEvade : public UNinjaCombatAnimationProvider
{
	
	GENERATED_BODY()

public:

	UAnimationProvider_DirectionalEvade();
	
	// -- Begin Animation Provider implementation
	virtual FName GetSectionName_Implementation(UNinjaCombatGameplayAbility* CombatAbility) const override;
	// -- End Animation Provider implementation

protected:

	/**
	 * Maps the Evade Directions to the appropriate Animation Montage sections.
	 *
	 * This setup expects that the default Animation Montage has multiple sections,
	 * each representing a proper direction of the evade movement.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation Provider", meta = (TitleProperty = "{SectionName} {Range}"))
	TArray<FEvadeDirection> EvadeDirections;

	/**
	 * Calculates the Evade Angle, based on any logic applicable for the game.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Animation Provider")
	bool CalculateEvadeAngle(UNinjaCombatGameplayAbility* CombatAbility, float& OutAngle) const;

private:
	
	/** Helper method to initialize an array with default directions, during construction. */
	static void InitializeDirections(TArray<FEvadeDirection>& DirectionsArray);

};
