// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaCombatWeaponActor.h"

#include "Net/UnrealNetwork.h"

ANinjaCombatWeaponActor::ANinjaCombatWeaponActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	bReplicates = true;
	WeaponTags = FGameplayTagContainer::EmptyContainer;
}

void ANinjaCombatWeaponActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, WeaponTags);
}

FGameplayTagContainer ANinjaCombatWeaponActor::GetWeaponTags_Implementation() const
{
	return WeaponTags;
}

bool ANinjaCombatWeaponActor::GetProjectileClass_Implementation(TSubclassOf<AActor>& OutProjectileClass) const
{
	if (IsValid(ProjectileClass))
	{
		OutProjectileClass = ProjectileClass;
		return true;
	}

	return false;
}
