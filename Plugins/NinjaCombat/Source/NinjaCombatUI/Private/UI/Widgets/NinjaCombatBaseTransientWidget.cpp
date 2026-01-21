// Ninja Bear Studio Inc., all rights reserved.
#include "UI/Widgets/NinjaCombatBaseTransientWidget.h"

#include "TimerManager.h"

UNinjaCombatBaseTransientWidget::UNinjaCombatBaseTransientWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bHideWhenUnchanged = false;
	DisplayDuration = 3.f;
	VisibilityWhenShown = ESlateVisibility::HitTestInvisible;
	VisibilityWhenHidden = ESlateVisibility::Hidden;
}

void UNinjaCombatBaseTransientWidget::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	Super::NativeDestruct();
}

void UNinjaCombatBaseTransientWidget::ShowWidget_Implementation()
{
	SetVisibility(VisibilityWhenShown);

	if (bHideWhenUnchanged)
	{
		GetWorld()->GetTimerManager().SetTimer(HideTimerHandle, this, &ThisClass::HideWidget,
			DisplayDuration, false, DisplayDuration);
	}	
}

void UNinjaCombatBaseTransientWidget::HideWidget_Implementation()
{
	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);
	SetVisibility(VisibilityWhenHidden);
}
