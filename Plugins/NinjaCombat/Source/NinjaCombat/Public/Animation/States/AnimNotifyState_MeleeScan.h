// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/NinjaCombatAnimNotifyState.h"
#include "Types/ECombatAbilitySource.h"
#include "Types/EMeleeScanMode.h"
#include "AnimNotifyState_MeleeScan.generated.h"

class UNinjaCombatMeleeScan;

/**
 * Sends Gameplay Events to start and stop melee scans.
 */
UCLASS(meta = (DisplayName = "Melee Scan"))
class UAnimNotifyState_MeleeScan : public UNinjaCombatAnimNotifyState
{
	
	GENERATED_BODY()

public:

	/**
	 * Determines if this scan uses the owner mesh that hosts this notify or a mesh from
	 * a weapon that must be obtained from the owner's Combat Manager, by a Weapon Query.
	 */
	UPROPERTY(EditAnywhere, Category = "Melee Scan")
	ECombatAbilitySource Source;

	/** Query used to retrieve the backing weapon. */
	UPROPERTY(EditAnywhere, Category = "Melee Scan", meta = (EditCondition = "bUsesWeapon", EditConditionHides))
	FGameplayTagQuery WeaponQuery;
	
	/** Sockets used for the Melee Scan. */
	UPROPERTY(EditAnywhere, Category = "Melee Scan")
	TArray<FName> ScanSocketNames;

	/** Channel used for the scan. */
	UPROPERTY(EditAnywhere, Category = "Melee Scan")
	TEnumAsByte<ECollisionChannel> ScanChannel;
	
	/** Scan Mode to be used. */
	UPROPERTY(EditAnywhere, Category = "Melee Scan")
	EMeleeScanMode ScanMode;

	/** Half extent for a box sweep. */
	UPROPERTY(EditAnywhere, Category = "Melee Scan", meta = (EditCondition = "ScanMode == EMeleeScanMode::BoxSweep", EditConditionHides))	
	FVector BoxHalfExtent;

	/** Extent for a capsule sweep. Max(X, Y) is used for radius, Z is used for Half-Height. */
	UPROPERTY(EditAnywhere, Category = "Melee Scan", meta = (EditCondition = "ScanMode == EMeleeScanMode::CapsuleSweep", EditConditionHides))	
	FVector CapsuleExtent;
	
	/** Radius for a sphere sweep. */
	UPROPERTY(EditAnywhere, Category = "Melee Scan", meta = (EditCondition = "ScanMode == EMeleeScanMode::SphereSweep", EditConditionHides))
	float SphereRadius;

	/** Class used to represent the Melee Scan. */
	UPROPERTY(EditAnywhere, Category = "Melee Scan", NoClear)
	TSubclassOf<UNinjaCombatMeleeScan> MeleeScanClass;
	
	UAnimNotifyState_MeleeScan();
	
	// -- Begin Anim Notify State implementation
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;
	// -- End Anim Notify State implementation

protected:

	/**
	 * Provides the Weapon Mesh for this scan, whenever is configured to do so.
	 *
	 * @param Owner		Owner of the weapon component that can be used to retrieve the weapon.
	 * @return			The Weapon's Mesh Component used to perform this scan.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ninja Combat|Melee Scan Anim Notify State")
	static UMeshComponent* GetMeleeScanMesh(const AActor* Owner);
	
	/**
	 * Creates the Scan Instance from the parameters in this class.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Melee Scan")
	UNinjaCombatMeleeScan* CreateMeleeScanInstance(AActor* Owner, AActor* Causer, UMeshComponent* SourceMesh) const;

};
