// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "NiagaraDataInterfaceExport.h"
#include "Interfaces/CombatMeleeInterface.h"
#include "Interfaces/CombatRangedInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/FDamageEntry.h"
#include "NinjaCombatFunctionLibrary.generated.h"

class UNinjaCombatManagerComponent;

/**
 * Support functions for the Combat System. 
 */
UCLASS()
class NINJACOMBAT_API UNinjaCombatFunctionLibrary : public UBlueprintFunctionLibrary
{
	
	GENERATED_BODY()

public:
	
	/**
	 * Provides the Combo Tracker Component assigned to an actor, via the Combat System Interface.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Combo Manager"))
	static UActorComponent* GetComboManagerComponent(const AActor* Owner);
	
	/**
	 * Provides the Damage Manager Component assigned to an actor, via the Combat System Interface. 
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Damage Manager"))
	static UActorComponent* GetDamageManagerComponent(const AActor* Owner);

	/**
	 * Provides the Defense Manager Component assigned to an actor, via the Combat System Interface. 
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Defense Manager"))
	static UActorComponent* GetDefenseManagerComponent(const AActor* Owner);
	
	/**
	 * Provides the Motion Warping Component assigned to an actor, via the Combat System Interface.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Motion Warping"))
	static UActorComponent* GetMotionWarpingComponent(const AActor* Owner);

	/**
	 * Provides the Movement Manager Component assigned to an actor, via the Combat System Interface.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Motion Warping"))
	static UActorComponent* GetMovementManagerComponent(const AActor* Owner);
	
	/**
	 * Provides the Physical Animation Component assigned to an actor, via the Combat System Interface.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Physical Animation"))
	static UActorComponent* GetPhysicalAnimationComponent(const AActor* Owner);	

	/**
	 * Provides the Target Manager Component assigned to an actor, via the Combat System Interface.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Target Manager"))
	static UActorComponent* GetTargetManagerComponent(const AActor* Owner);
	
	/**
	 * Provides the Weapon Manager Component assigned to an actor, via the Combat System Interface. 
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Weapon Manager"))
	static UActorComponent* GetWeaponManagerComponent(const AActor* Owner);
	
	/**
	 * Provides the Melee Script for a given target, implementing the Melee Interface.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Melee Script"))
	static TScriptInterface<ICombatMeleeInterface> GetMeleeScript(UObject* Target);

	/**
	 * Provides the Ranged Script for a given target, implementing the Melee Interface.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (ReturnDisplayName = "Ranged Script"))
	static TScriptInterface<ICombatRangedInterface> GetRangedScript(UObject* Target);
	
	/**
	 * Breaks a Damage Entry into its values so Blueprints can easily access them.
	 * 
	 * @param Entry				The Entry that is going to be exposed by this function.
	 * @param Causer			Actor that caused the damage.
	 * @param Instigator		Source actor that caused the damage.
	 * @param bIsCriticalHit	Informs if this is a critical hit.
	 * @param bIsBreakerHit		Informs if this is a breaker hit.
	 * @param bIsStaggerHit		Informs if this is a stagger hit.
	 * @param bIsFatalHit		Informs if this is a fatal hit.
	 * @param Timestamp			Moment when the damage happened. 
	 * @param DamageApplied		Total damage applied after mitigation.
	 * @param MitigationCost	Total damage avoided due to mitigation.
	 * @param PreviousHealth	Health on target before the damage.
	 * @param PreviousStamina	Previous stamina on target before mitigation.
	 * @param SourceTags		Gameplay tags present in the source.
	 * @param TargetTags		Gameplay tags present in the target.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (BlueprintThreadSafe))
	static void BreakDamageStruct(const FDamageEntry& Entry, AActor*& Causer, AActor*& Instigator,
		bool& bIsCriticalHit, bool& bIsBreakerHit, bool& bIsStaggerHit, bool& bIsFatalHit, float& Timestamp, float& DamageApplied,
		float& MitigationCost, float& PreviousHealth, float& PreviousStamina, FGameplayTagContainer& SourceTags,
		FGameplayTagContainer& TargetTags);
	
	/**
	 * Executes a Gameplay Cue locally, avoiding the Multicast RPC.
	 * 
	 * @param Owner						Owner of the Ability System Component handling the Cue. 
	 * @param GameplayCueTag			Gameplay Tag representing the Cue.
	 * @param GameplayCueParameters		Parameters used by the Gameplay Cue.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	static void ExecuteGameplayCueLocally(AActor* Owner, const FGameplayTag GameplayCueTag,
		const FGameplayCueParameters& GameplayCueParameters);

	/**
	 * Adds a Gameplay Cue locally, avoiding the Multicast RPC.
	 * 
	 * @param Owner						Owner of the Ability System Component handling the Cue. 
	 * @param GameplayCueTag			Gameplay Tag representing the Cue.
	 * @param GameplayCueParameters		Parameters used by the Gameplay Cue.
	 */	
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	static void AddGameplayCueLocally(AActor* Owner, const FGameplayTag GameplayCueTag,
		const FGameplayCueParameters& GameplayCueParameters);

	/**
	 * Removes a Gameplay Cue locally, avoiding the Multicast RPC.
	 * 
	 * @param Owner						Owner of the Ability System Component handling the Cue. 
	 * @param GameplayCueTag			Gameplay Tag representing the Cue.
	 * @param GameplayCueParameters		Parameters used by the Gameplay Cue.
	 */	
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", meta = (AutoCreateRefTerm = "GameplayCueParameters", GameplayTagFilter = "GameplayCue"))
	static void RemoveGameplayCueLocally(AActor* Owner, const FGameplayTag GameplayCueTag,
		const FGameplayCueParameters& GameplayCueParameters);

	/**
	 * Creates parameters for a Gameplay Cue, from an Context Handle.
	 * 
	 * @param EffectContextHandle		Source Effect Context Handle. 
	 * @return							Gameplay Cue with the Effect Data.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat")
	static FGameplayCueParameters CreateGameplayCueFromEffectContext(const FGameplayEffectContextHandle& EffectContextHandle);
	
	/**
	 * Creates a blood decal from a Niagara Particle Collision.
	 * 
	 * @param WorldContextObject			World context used to spawn the decal. 
	 * @param Data							Niagara: Information about the particle that generated the collision.
	 * @param NiagaraSystem					Niagara: The system that is generating collisions.
	 * @param SimulationPositionOffset		Niagara: Offset for the simulation
	 * @param BaseSize						Decal: The base size for the decal, before any random modifications.
	 * @param DecalMaterial					Decal: The material used for the blood decal.
	 * @param DecalChance					Decal: Chance to spawn the decal in the [0, 1] range.
	 * @param ScreenSize					Decal: Screen size required for the decal to be visible. Zero means "always".
	 * @param DecalLifeSpan					Decal: How long the decal will live in the game world.
	 * @param FadeOutDuration				Decal: Duration of the fade-out when the decal is about to be destroyed.
	 * @param FixedXSize					Tweaks: Fixed size for the local X coordinate (height) of the decal. 
	 * @param SizeModifierRange				Tweaks: A range that allows random modifications in the decal's size.
	 * @param LifespanModifierRange			Tweaks: A range that allows random modifications in the decal's lifespan.
	 * 
	 * @return
	 *		The Decal Component fully configured and spawned in the game world. Since this is a cosmetic function,
	 *		it will always be skipped and return null if called in a Dedicated Server.
	 */
	UFUNCTION(BlueprintPure, Category = "Ninja Combat", BlueprintCosmetic, meta = (WorldContext = "WorldContextObject", ReturnDisplayName = "Decal Component"))
	static UDecalComponent* SpawnBloodDecal(
		const UObject* WorldContextObject,
		const TArray<FBasicParticleData>& Data, const UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset,
		const FVector& BaseSize, UMaterialInterface* DecalMaterial,
		float DecalChance = 1.f, float ScreenSize = 0.f, float DecalLifeSpan = 20.f, float FadeOutDuration = 5.f, float FixedXSize = 3.5f,
		FVector2D SizeModifierRange = FVector2D(0.75, 1.25f), FVector2D LifespanModifierRange = FVector2D(0.8, 1.2f));
};
