// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "NinjaCombatBaseWidget.h"
#include "NinjaCombatBaseTransientWidget.generated.h"

/**
 * Base widget that will be shown due to a certain event and hidden after a certain amount of time.
 */
UCLASS(Abstract)
class NINJACOMBATUI_API UNinjaCombatBaseTransientWidget : public UNinjaCombatBaseWidget
{
	
	GENERATED_BODY()

public:

	UNinjaCombatBaseTransientWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Widget implementation.
	virtual void NativeDestruct() override;
	// -- End Widget implementation.
	
protected:

	/** Visibility applied by the "Show Widget" event. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Transient Settings")
	ESlateVisibility VisibilityWhenShown;

	/** Visibility applied by the "Hide Widget" event. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Transient Settings")
	ESlateVisibility VisibilityWhenHidden;
	
	/** Determines if this widget hides after being unchanged for a certain amount of time. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat Widget|Transient Settings")
	bool bHideWhenUnchanged;

	/**
	 * Amount of time that the widget will be displayed for, if unchanged.
	 * Can be set from a value defined on the parent widget, down to all transient children.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Widget|Transient Settings", meta = (EditCondition = "bHideWhenUnchanged"))
	float DisplayDuration;

	/**
	 * Shows the widget, applying the visibility rule and starting the timer if needed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Ninja Combat|Widgets")
	void ShowWidget();
	
	/**
	 * Hides the Widget, applying the visibility rule and cancelling the timer if needed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintCosmetic, Category = "Ninja Combat|Widgets")
	void HideWidget();

private:

	FTimerHandle HideTimerHandle;
	
};
