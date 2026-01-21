// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "NinjaCombatProjectileRequest.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNinjaCombatProjectileRequest, Log, All);


/**
 * Represents a request to launch a projectile.
 */
UCLASS()
class NINJACOMBAT_API UNinjaCombatProjectileRequest : public UObject
{
	
	GENERATED_BODY()

public:

	UNinjaCombatProjectileRequest();
	
	/**
	 * Creates a new Projectile Request, with required data.
	 *
	 * @param RequestClass		Optional class used for the request, in case the default one is being extended.
	 * @param Owner				Owner for this request.
	 * @param Instigator		Instigator used for the projectile.
	 * @param SourceMesh		Source mesh from where the projectile will be spawned.
	 * @param SocketName		Socket in the mesh to use as a launch origin.
	 * @param ProjectileClass	Class representing the projectile that will be spawned.
	 * @return					The projectile request, ready to be used.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Projectile Request", DisplayName = "New Projectile Request Instance")
	static UNinjaCombatProjectileRequest* NewInstance(TSubclassOf<UNinjaCombatProjectileRequest> RequestClass,
		AActor* Owner, AActor* Instigator, UMeshComponent* SourceMesh, FName SocketName,
		UPARAM(meta = (MustImplement = "/Script/NinjaCombat.CombatProjectileInterface")) TSubclassOf<AActor> ProjectileClass);
	
	/**
	 * Checks if this instance has valid parameters to request a projectile launch.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Projectile Request")
	virtual bool HasValidRequestData() const;	

	/**
	 * Provides the owner for this request.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Projectile Request")
	FORCEINLINE AActor* GetRequestOwner() const { return RequestOwner; }
	
	/**
	 * Provides the instigator for this request, which can be used by Gameplay Effects.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Projectile Request")
	FORCEINLINE APawn* GetInstigator() const { return Instigator; }

	/**
	 * Provides the socket name used as an origin for the projectile.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Projectile Request")
	FORCEINLINE FName GetSocketName() const { return SocketName; }

	/**
	 * Calculates and provides the Source Rotation that can be used.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Projectile Request")
	virtual FVector GetSourceLocation() const;
	
	/**
	 * Calculates and provides the Source Rotation that can be used.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat|Projectile Request")
	virtual FRotator GetSourceRotation() const;

	/**
	 * Sets an instigator for this request, overriding the default one.
	 * 
	 * @param NewInstigator
	 *		New instigator used for this request.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Projectile Request")
	void SetInstigator(APawn* NewInstigator);

	/**
	 * Sets a target location to aim the projectile at. Optional.
	 * 
	 * @param NewTargetLocation
	 *		Location used as target.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Projectile Request")
	void SetTargetLocation(const FVector& NewTargetLocation);

	/**
	 * Sets a fallback class to the projectile class set in this request.
	 * Only effective if the internal class is not set yet.
	 *
	 * @param ProjectileClassFallback
	 *		Fallback class for the projectile. Must implement CombatProjectileInterface.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Projectile Request")
	void SetProjectileClassFallback(UPARAM(meta = (MustImplement = "/Script/NinjaCombat.CombatProjectileInterface")) TSubclassOf<AActor> ProjectileClassFallback);
	
	/**
	 * Spawns the projectile, based on the internal definition.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Projectile Request")
	AActor* SpawnProjectile() const;

protected:

	/** Actor that has triggered the projectile. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile Request")
	TObjectPtr<AActor> RequestOwner;

	/** Instigator for this projectile, which can be used by Gameplay Effects. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile Request")
	TObjectPtr<APawn> Instigator;
	
	/** The Mesh Component from which the projectile will spawn. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile Request")
	TObjectPtr<UMeshComponent> SourceMesh;	

	/** Socket used to retrieve the origin location. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile Request")
	FName SocketName;

	/** Class representing the projectile that will be spawned. */
	UPROPERTY(BlueprintReadOnly, Category = "Projectile Request")
	TSubclassOf<AActor> ProjectileClass;

	/** Modifies the projectile before invoking "Finish Spawning". */
	UFUNCTION(BlueprintNativeEvent, Category = "Projectile Request")
	void ModifyProjectile(AActor* Projectile) const;

private:

	/** Informs if there is a target location set to this request. */
	bool bHasTargetLocation;	

	/** An optional location that can be set to aim the projectile at. */
	FVector TargetLocation;
	
};
