// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameFramework/Actor.h"
#include "Interfaces/CombatProjectileInterface.h"
#include "Types/FDamageEntry.h"
#include "Types/TargetingSystemTypes.h"
#include "NinjaCombatProjectileActor.generated.h"

class UNiagaraSystem;
class UProjectileMovementComponent;
class USphereComponent;
class USoundBase;
class UTargetingPreset;

/**
 * Base implementation of a projectile.
 */
UCLASS(Abstract)
class NINJACOMBAT_API ANinjaCombatProjectileActor : public AActor, public ICombatProjectileInterface
{
	
	GENERATED_BODY()

public:

	ANinjaCombatProjectileActor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// -- Begin Actor implementation
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	// -- End Actor implementation

	// -- Begin Projectile implementation
	virtual TSubclassOf<UGameplayEffect> GetImpactEffectClass_Implementation() const override;
	virtual TArray<AActor*> GetActorsHit_Implementation() const override;
	virtual float GetProjectileImpactStrength_Implementation() const override;
	virtual void SetImpactEffectHandle_Implementation(FGameplayEffectSpecHandle& Handle) override;
	virtual void Launch_Implementation() override;
	virtual void HandleImpactCosmetics_Implementation(const FHitResult& HitResult) const override;
	virtual void HandleDestructionCosmetics_Implementation() const override;
	// -- End Projectile implementation

	/**
	 * Provides the Capsule Component that's the main projectile collision.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Combat Projectile")
	FORCEINLINE USphereComponent* GetCapsuleComponent() const { return SphereComponent; }

	/**
	 * Provides the Projectile Movement Component.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Combat Projectile")
	FORCEINLINE UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }	
	
	/**
	 * Provides the amount of bounces that happened in this projectile.
	 * Bounciness must be enabled for this to be counted.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Combat Projectile")
	FORCEINLINE int32 GetBounceCount() const { return BounceCount; }
	
protected:

	/** Impact Strength for physical animations. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile")
	float ImpactStrength;
	
	/** Maximum bounces the projectile can do. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile")
	int32 MaxBounces;

	/** Impact effect used by this projectile. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile")
	TSubclassOf<UGameplayEffect> ImpactEffectClass;

	/** Particle Effect played when the projectile hits something. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile")
	TSoftObjectPtr<UNiagaraSystem> ImpactFX;

	/** Particle Effect played when the projectile is being destroyed. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile")
	TSoftObjectPtr<UNiagaraSystem> DestructionFX;
	
	/** Sound Effect played when the projectile hits something. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile")
	TSoftObjectPtr<USoundBase> ImpactSound;

	/**
	 * Trace Channel used for Projectile Traces.
	 * It's recommended to create a dedicated Channel for Projectile Traces.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile")
	TEnumAsByte<ECollisionChannel> TraceChannel;

	/** If set to true, it will use the targeting system to collect targets for launch and bounces. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile")
	bool bUsesTargetingSystem;
	
	/** Defines if the targeting will execute asynchronously. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile", meta = (EditCondition = "bUsesTargetingSystem", EditConditionHides))
	bool bExecuteAsync;
	
	/**
	 * The Targeting Preset used to collect a target and every other target after a bounce.
	 * Can be left empty, in which case the projectile will simply follow the initial trajectory.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat Projectile", meta = (EditCondition = "bUsesTargetingSystem", EditConditionHides))
	TObjectPtr<UTargetingPreset> TargetingPreset;

	/**
	 * Determines if the Debug Mode is activated for this projectile.
	 * This will display trace adjustments and impacts.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Projectile")
	bool bEnableDebug;

	/**
	 * Calculates the Initial Rotation for a forward-orienting projectile.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Projectile")
	FRotator GetInitialRotation() const;
	
	/**
	 * Launches the projectile forward, considering the Initial Rotation.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Projectile")
	void LaunchForward();

	/**
	 * Launches towards the next available target, removing it from the array.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Projectile")
	void LaunchToTarget(const AActor* Target);

	/**
	 * Adjusts a Hit Result in case you need to trace from an impact on the Capsule to one on the Mesh.
	 *
	 * @param ImpactResult		The Impact Result from a collision. 
	 * @param AdjustedResult	The Adjusted Result used in the Gameplay Effect Context.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Projectile")
	void AdjustHitResult(const FHitResult& ImpactResult, FHitResult& AdjustedResult);
	
	/**
	 * Handles a collision with an actor.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Projectile")
	void HandleActorHit(const FHitResult& ImpactResult);
	
	/**
	 * Handles the projectile in the end of its lifecycle.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Combat Projectile")
	void HandleProjectileExhausted();

	/**
	 * Handles a target that has been collected by the targeting system.
	 */  
	virtual void HandleTargetReceived(const FTargetingRequestHandle& TargetingHandle);

	/**
	 * Plays a Niagara System, from a loaded Soft Object Pointer. 
	 */
	static void PlayNiagaraSystem(const UWorld* World, UNiagaraSystem* System,
		const FVector& Location, const FRotator& Rotation);

	/**
	 * Plays a Sound Asset, from a loaded Soft Object Pointer. 
	 */	
	static void PlaySound(const UWorld* World, USoundBase* Sound, const FVector& Location,
		const FRotator& Rotation);
	
private:

	/** Checks if this projectile has been exhausted. */
	bool bHasExhausted;

	/** Target we are chasing after. */
	int32 TargetIndex;
	
	/** Current amount of bounces that happened. */
	int32 BounceCount;

	/** Velocity from last bounce, if any. */
	FVector LastBounceVelocity;

	/** Handle to be used when the projectile hits. */
	FGameplayEffectSpecHandle ImpactEffectHandle;
	
	/** Sphere component used as root for the projectile. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess))
	TObjectPtr<USphereComponent> SphereComponent;
	
	/** Movement Component for projectiles. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess))
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;
	
	/** List of all actors that were hit by this projectile. */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> ActorsHit;

	/** Requests a sync/async targeting. */
	void ExecuteTargeting();
	
	/** Hidden handler to ensure internal logic is correct. */
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);
	
	/** Hidden handler to ensure internal logic is correct. */
	UFUNCTION()
	void OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	// -- Start Debug Helpers
	void PrintMessage(const FString& Message) const;
	void DrawScanLine(const FHitResult& NewHit) const;
	void DrawNewTarget(const FHitResult& NewHit, const FColor& Color) const;
	// -- End Debug Helpers
	
};
