// Ninja Bear Studio Inc., all rights reserved.
#include "Damage/Handlers/DamageHandler_Cosmetics.h"

#include "Interfaces/CombatMeleeInterface.h"
#include "Interfaces/CombatProjectileInterface.h"

UDamageHandler_Cosmetics::UDamageHandler_Cosmetics()
{
	bHandleMeleeDamage = true;
	bHandleProjectileImpact = true;
}

void UDamageHandler_Cosmetics::HandleDamage_Implementation(UNinjaCombatDamageManagerComponent* DamageManager, const FGameplayCueParameters& Parameters) const
{
	const FHitResult* HitResult = Parameters.EffectContext.GetHitResult();

	const AActor* EffectCauser = Parameters.GetEffectCauser();
	if (IsValid(EffectCauser) && HitResult != nullptr)
	{
		if (IsMeleeDamage(Parameters) && EffectCauser->Implements<UCombatMeleeInterface>())
		{
			ICombatMeleeInterface::Execute_HandleMeleeDamageCosmetics(EffectCauser, *HitResult);	
		}
			
		if (IsRangedDamage(Parameters) && EffectCauser->Implements<UCombatProjectileInterface>())
		{
			ICombatProjectileInterface::Execute_HandleImpactCosmetics(EffectCauser, *HitResult);
		}
	}
}
