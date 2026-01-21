// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "NinjaCombatDelegates.h"
#include "Components/StateTreeComponent.h"
#include "Interfaces/Components/CombatComboManagerInterface.h"
#include "NinjaCombatComboManagerComponent.generated.h"

class UNinjaCombatComboSetupData;

/**
 * Manages a Combo, using a State Tree to determine each possible state and transition.
 */
UCLASS(Blueprintable, ClassGroup = NinjaCombat, meta = (BlueprintSpawnableComponent))
class NINJACOMBAT_API UNinjaCombatComboManagerComponent : public UStateTreeComponent, public ICombatComboManagerInterface
{
	
	GENERATED_BODY()

public:
	
	UNinjaCombatComboManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Actor Component implementation
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// -- End Actor Component implementation
	
	// -- Begin Combo Tracker implementation
	virtual void BindToComboWindowChangedDelegate_Implementation(const FComboWindowChangedDelegate& Delegate) override;
	virtual void BindToComboFinishedDelegate_Implementation(const FComboFinishedDelegate& Delegate) override;
	virtual void UnbindFromComboWindowChangedDelegate_Implementation(const UObject* Source) override;
	virtual void UnbindFromComboFinishedDelegate_Implementation(const UObject* Source) override;
	virtual bool InComboWindow_Implementation() const override { return bInComboWindow; }
	virtual void OpenComboWindow_Implementation() override;
	virtual void CloseComboWindow_Implementation() override;
	virtual int32 GetComboCount_Implementation() const override { return ComboCount; }
	virtual void SetComboCount_Implementation(int32 NewComboCount) override;
	virtual void StartCombo_Implementation(const UNinjaCombatComboSetupData* NewComboData) override;
	virtual void AdvanceCombo_Implementation(const UInputAction* Action) override;
	virtual void ResetCombo_Implementation() override;
	// -- End Combo Tracker implementation
	
protected:

	/** Notifies that the running combo is in the combo window. */
	UPROPERTY(BlueprintAssignable, Category = "Combo Manager")
	FComboWindowChangedMCDelegate OnComboWindowChanged;
	
	/** Notifies that the running combo has ended. */
	UPROPERTY(BlueprintAssignable, Category = "Combo Manager")
	FComboFinishedMCDelegate OnComboFinished;

	/** Stores the combo window state. */
	UPROPERTY(ReplicatedUsing = OnRep_InComboWindow)
	uint8 bInComboWindow:1;
	
	/** Counts how many iterations in the current combo. */
	UPROPERTY(Replicated)
	int32 ComboCount;	

	/**
	 * Timer-friendly function to connect to the Ability Component.
	 */
	UFUNCTION()
	void BindToAbilitySystemComponent();

	/**
	 * Unbinds from the Ability Component.
	 */
	void UnbindFromAbilitySystemComponent();

	/**
	 * Reacts to a change in the tracked tags.
	 */
	virtual void HandleTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	/**
	 * Reacts to the replication of the "Combo Window" property.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combo Manager")
	void OnRep_InComboWindow(bool bWasInComboWindow);

private:

	/** Delegate Handle for the Combo Window Tag event listener. */
	FDelegateHandle ComboWindowTagHandle;
	
	/** Current combo being executed. */
	UPROPERTY()
	TObjectPtr<const UNinjaCombatComboSetupData> ComboData;

	/** Helps determining if a status is final. */
	static bool IsFinished(const EStateTreeRunStatus& StateTreeRunStatus);
	
	/** Notifies a change in the tree status, so we can cancel/end the combo ability. */
	UFUNCTION()
	void HandleTreeStatusChanged(EStateTreeRunStatus StateTreeRunStatus);
	
};
