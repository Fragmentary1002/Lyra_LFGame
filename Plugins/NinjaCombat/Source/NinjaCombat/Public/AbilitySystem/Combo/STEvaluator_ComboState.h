// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/StateTreeEvaluatorBlueprintBase.h"
#include "STEvaluator_ComboState.generated.h"

/**
 * Exposes the current combo state in the backing character's Combat Component.
 */
UCLASS(DisplayName = "Combo State", Category = "Combat")
class NINJACOMBAT_API USTEvaluator_ComboState : public UStateTreeEvaluatorBlueprintBase
{
	
	GENERATED_BODY()

public:
	
	explicit USTEvaluator_ComboState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:

	/** Reference to the Actor owning the Combat Component. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	TObjectPtr<AActor> CombatActor;

	/** Informs if the Combo Window is currently open. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Output)
	bool bInComboWindow;

	/** Current count for the combo. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Output)
	int32 ComboCount;

	// -- Begin State Tree Evaluator implementation
	virtual void Tick(FStateTreeExecutionContext& Context, const float DeltaTime) override;
	// -- End State Tree Evaluator implementation
	
};
