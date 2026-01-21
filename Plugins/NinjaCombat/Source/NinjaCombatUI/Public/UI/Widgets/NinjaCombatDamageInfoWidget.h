// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Types/FDamageEntry.h"
#include "NinjaCombatBaseTransientWidget.h"
#include "NinjaCombatDelegates.h"
#include "NinjaCombatDamageInfoWidget.generated.h"

class UTextBlock;
/**
 * Base widget class that receives damage information and displays details. 
 */
UCLASS(Abstract)
class NINJACOMBATUI_API UNinjaCombatDamageInfoWidget : public UNinjaCombatBaseTransientWidget
{
	
	GENERATED_BODY()

public:

	UNinjaCombatDamageInfoWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// -- Begin Widget implementation
	virtual void HideWidget_Implementation() override;
	// -- End Widget implementation

	// -- Begin Combat Widget implementation
	virtual void HandleGameplayCue_Implementation(const FGameplayCueParameters& CueParameters) override;
	// -- End Combat Widget implementation
	
protected:

	/** Damage Text Block storing the damage info. */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Widget|Damage Info", meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageTextBlock;
	
	/** Determines if this widget accumulates damage being displayed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Damage Info")
	bool bAccumulateDamage;

	/**
	 * Updates incoming damage, providing all the details from it.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Widget")
	void UpdateDamage(float Damage, bool bIsBreakerHit, bool bIsCriticalHit, bool bIsFatalHit,
		const FVector DamageLocation, const FGameplayTagContainer& SourceTags);

private:
	
	float AccumulatedDamage;

};
