// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameplayEffectTypes.h"
#include "NinjaCombatBaseTransientWidget.h"
#include "NinjaCombatBaseOverheadBarWidget.generated.h"

class UProgressBar;

/**
 * Base widget used to display attributes (current/maximum) as a progress bar.
 */
UCLASS(Abstract)
class NINJACOMBATUI_API UNinjaCombatBaseOverheadBarWidget : public UNinjaCombatBaseTransientWidget
{
	
	GENERATED_BODY()

public:

	UNinjaCombatBaseOverheadBarWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// -- Begin Widget implementation
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// -- End Widget implementation

	// -- Begin Combat Widget implementation
	virtual void HandleGameplayCue_Implementation(const FGameplayCueParameters& CueParameters) override;
	// -- End Combat Widget implementation
	
protected:

	/** Progress Bar used to represent the status. */
	UPROPERTY(BlueprintReadWrite, Category = "Combat Widget|Overhead Bar", meta = (BindWidget))
	TObjectPtr<UProgressBar> OverheadProgressBar;

	/** Determines if this widget hides after receiving fatal damage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Overhead Bar")
	bool bHideOnFatalHit;
	
	/** Attribute that backs the Progress Bar's current value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Overhead Bar")
	FGameplayAttribute CurrentValueAttribute;

	/** Attribute that backs the Progress Bar's maximum value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Overhead Bar")
	FGameplayAttribute MaximumValueAttribute;

	/** Attribute that backs the Progress Bar's maximum/add value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Overhead Bar")
	FGameplayAttribute MaximumAddValueAttribute;

	/** Attribute that backs the Progress Bar's maximum/percent value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Overhead Bar")
	FGameplayAttribute MaximumPercentValueAttribute;

	/** Attribute that backs the Progress Bar's absolute (meta) value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Overhead Bar")
	FGameplayAttribute MaximumTotalValueAttribute;
	
	/** Connects to the owner's ASC. */
	UFUNCTION()
	virtual void BindToAbilityComponent();

	/** Unbinds delegates from the owner's ASC. */
	virtual void UnbindFromAbilityComponent();

	/** Tracks changes in the current attribute. */
	void HandleCurrentAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	/** Tracks changes in the maximum attribute. */
	void HandleMaximumAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	/** Tracks changes in the maximum (add) attribute. */
	void HandleMaximumAddAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData);

	/** Tracks changes in the maximum (percent) attribute. */
	void HandleMaximumPercentAttributeChanged(const FOnAttributeChangeData& OnAttributeChangeData);
	
	/**
	 * Reacts to an update in the status, with current and maximum values.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Status Bar Widget")
	void UpdateStatus(float CurrentValueReceived, float MaximumValueReceived);

private:

	float CurrentValue;
	float MaximumValue;
	float MaximumAddValue;
	float MaximumPercentValue;
	float MaximumTotalValue;

	FDelegateHandle CurrentValueDelegate;
	FDelegateHandle MaximumValueDelegate;
	FDelegateHandle MaximumAddValueDelegate;
	FDelegateHandle MaximumPercentValueDelegate;

	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedAbilityComponent;
	
};
