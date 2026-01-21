// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "Blueprint/UserWidget.h"
#include "UI/Interfaces/CombatWidgetInterface.h"
#include "NinjaCombatBaseWidget.generated.h"

class UAbilitySystemComponent;
class UNinjaCombatManagerComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogNinjaCombatWidget, Log, All);

/**
 * Base combat widget with common functionality.
 */
UCLASS(Abstract)
class NINJACOMBATUI_API UNinjaCombatBaseWidget : public UUserWidget, public ICombatWidgetInterface
{
	
	GENERATED_BODY()

public:

	UNinjaCombatBaseWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Combat Widget implementation
	virtual void SetCombatActor_Implementation(AActor* NewCombatActor) override;
	virtual void HandleGameplayCue_Implementation(const FGameplayCueParameters& CueParameters) override;
	// -- End Combat Widget implementation

	/** 
	 * Provides the Combat Actor for this Widget.
	 *
	 * It could be the usual owning pawn or, if set differently via the Combat Widget Interface,
	 * a custom actor, such as the one that owns the hosting Widget Component.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Widgets")
	virtual AActor* GetCombatActor() const;

protected:

	/**
	 * Provides the Ability Component assigned to this widget's Combat Actor.
	 * 
	 * Please note that, for replicated actors with an Ability Component in the player state,
	 * it may take a moment before the Ability Component becomes available for the Widget.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Widgets")
	UAbilitySystemComponent* GetAbilityComponent() const;
	
private:

	/** Combat Actor that should be considered the Combat Owner. */
	UPROPERTY()
	TObjectPtr<AActor> CombatActor;	
	
};
