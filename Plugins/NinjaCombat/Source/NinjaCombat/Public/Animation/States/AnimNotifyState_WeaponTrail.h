// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/NinjaCombatAnimNotifyState.h"
#include "AnimNotifyState_WeaponTrail.generated.h"

class UNiagaraComponent;

/**
 * Renders a weapon trail for a given weapon instance. These won't render in a Dedicated Server.
 */
UCLASS(meta = (DisplayName = "Weapon Trail"))
class NINJACOMBAT_API UAnimNotifyState_WeaponTrail : public UNinjaCombatAnimNotifyState
{
	
	GENERATED_BODY()
	
public:

	/** Query used to retrieve the backing weapon. */
	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FGameplayTagQuery WeaponQuery;

	/** Socket in the weapon where the trail starts. */
	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FName StartSocket;

	/** Socket in the weapon where the trail ends. */
	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FName EndSocket;
	
	/** Parameter in the VFX for the trail start position. */
	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FName TrailBeginParameterName;

	/** Parameter in the VFX for the trail end position. */
	UPROPERTY(EditAnywhere, Category = "Weapon Trail")
	FName TrailEndParameterName;

	UAnimNotifyState_WeaponTrail();

	// -- Begin Anim Notify State implementation
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	// -- End Anim Notify State implementation

private:

	/** Weapon mesh used for reference. */
	UPROPERTY()
	TObjectPtr<UMeshComponent> WeaponMeshComponent;
	
	/** All components active during this trail. */
	UPROPERTY()
	// Fragmentary 修改5.6适配
	TArray<	TObjectPtr<UNiagaraComponent>> TrailComponents;
	
};
