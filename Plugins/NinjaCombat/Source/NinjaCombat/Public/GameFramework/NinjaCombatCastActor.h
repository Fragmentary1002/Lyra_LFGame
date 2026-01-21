// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/CombatCastInterface.h"
#include "NinjaCombatCastActor.generated.h"

class USphereComponent;

/**
 * Base implementation of a cast.
 */
UCLASS(Abstract)
class NINJACOMBAT_API ANinjaCombatCastActor : public AActor, public ICombatCastInterface
{
	
	GENERATED_BODY()

public:

	ANinjaCombatCastActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// -- Begin Actor implementation
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// -- End Actor implementation
	
	// -- Begin Combat Cast implementation
	virtual void SetCastSource_Implementation(AActor* SourceActor) override;
	virtual void SetOwnerEffectHandle_Implementation(FGameplayEffectSpecHandle& Handle) override;
	virtual void SetTargetEffectHandle_Implementation(FGameplayEffectSpecHandle& Handle) override;
	virtual void StartCast_Implementation() override;
	// -- End Combat Cast implementation

	/**
	 * Provides the Actor that originated this cast.
	 *
	 * The actor might be null as the cast can work independently from the owning actor
	 * still being alive/valid. Make sure to check that before performing any operations!
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Combat Cast")
	AActor* GetCastSource() const;

protected:

	/** Handle to the Gameplay Effect Spec applied to the owner. */
	FGameplayEffectSpecHandle OwnerHandle;

	/** Handle to the Gameplay Effect Spec applied to targets. */
	FGameplayEffectSpecHandle TargetHandle;

	/**
	 * Applies the Gameplay Effect to this cast's owner.
	 *
	 * Must be called in the authoritative version. It's safe to call this function if
	 * there's no Gameplay Effect Handle for the owner.
	 */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ninja Combat|Combat Cast")
	virtual void ApplyEffectToSelf();

	/**
	 * Applies the Gameplay Effect to the target.
	 *
	 * Must be called in the authoritative version. It's safe to call this function if
	 * there's no Gameplay Effect Handle for targets.
	 */	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Ninja Combat|Combat Cast")
	virtual void ApplyEffectToTarget(AActor* Target);
	
	UFUNCTION()
	virtual void OnRep_HasStarted();

	/**
	 * Checks if a captured target should receive the effect.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Cast")
	bool ShouldApplyToTarget(AActor* Target);
	
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Cast")
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
private:

	/** Informs if this cast has started. */
	UPROPERTY(ReplicatedUsing = OnRep_HasStarted)
	uint8 bHasStarted:1;
	
	/** Source, usually an ability, that originated this cast. */
	UPROPERTY(Replicated)
	TWeakObjectPtr<AActor> CastSourcePtr;

	/** Collision sphere for the cast. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Cast", meta = (AllowPrivateAccess))
	TObjectPtr<USphereComponent> CastSphere;
	
};