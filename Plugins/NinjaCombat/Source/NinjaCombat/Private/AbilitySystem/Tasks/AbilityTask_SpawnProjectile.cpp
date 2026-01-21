// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Tasks/AbilityTask_SpawnProjectile.h"

#include "GameFramework/NinjaCombatProjectileRequest.h"

UAbilityTask_SpawnProjectile::UAbilityTask_SpawnProjectile()
{
	bEnableDebug = false;
}

UAbilityTask_SpawnProjectile* UAbilityTask_SpawnProjectile::CreateTask(UGameplayAbility* OwningAbility, const bool bEnableDebug)
{
	UAbilityTask_SpawnProjectile* NewTask = NewAbilityTask<UAbilityTask_SpawnProjectile>(OwningAbility);
	NewTask->bEnableDebug = bEnableDebug;
	return NewTask;	
}

void UAbilityTask_SpawnProjectile::Join(const UNinjaCombatProjectileRequest* Request)
{
	if (!ProjectileRequests.Contains(Request))
	{
		UNinjaCombatProjectileRequest* NonConstRequest = const_cast<UNinjaCombatProjectileRequest*>(Request);
		ProjectileRequests.Add(NonConstRequest);
	}
}

void UAbilityTask_SpawnProjectile::Activate()
{
	Super::Activate();

	for (auto It = ProjectileRequests.CreateIterator(); It; ++It)
	{
		const UNinjaCombatProjectileRequest* Request = *It;

		AActor* Projectile = Request->SpawnProjectile();
		if (IsValid(Projectile) && ShouldBroadcastAbilityTaskDelegates())
		{
			OnProjectileLaunched.Broadcast(Request, Projectile);
		}
		
		It.RemoveCurrent();
	}
}
