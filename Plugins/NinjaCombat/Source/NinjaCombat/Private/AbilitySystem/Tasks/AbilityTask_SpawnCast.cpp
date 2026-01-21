// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Tasks/AbilityTask_SpawnCast.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"

UAbilityTask_SpawnCast::UAbilityTask_SpawnCast()
{
	bEnableDebug = false;
}

UAbilityTask_SpawnCast* UAbilityTask_SpawnCast::CreateTask(UGameplayAbility* OwningAbility,
	const TSubclassOf<AActor> CastClass, const FTransform Transform, const bool bEnableDebug)
{
	UAbilityTask_SpawnCast* NewTask = NewAbilityTask<UAbilityTask_SpawnCast>(OwningAbility);
	NewTask->CastClass = CastClass;
	NewTask->CastTransform = Transform;
	NewTask->bEnableDebug = bEnableDebug;
	return NewTask;	
}

void UAbilityTask_SpawnCast::Activate()
{
	Super::Activate();
	SpawnCastActor();
}

void UAbilityTask_SpawnCast::SpawnCastActor()
{
	UWorld* World = GetWorld();
	if (IsValid(World))
	{
		AActor* Owner = GetAvatarActor();
		APawn* Instigator = Cast<APawn>(Owner);

		FActorSpawnParameters Parameters;
		Parameters.Instigator = Instigator;
		Parameters.Owner = Owner;
		Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
		
		AActor* CastActor = World->SpawnActor<AActor>(CastClass, CastTransform, Parameters);
		if (IsValid(CastActor) && ShouldBroadcastAbilityTaskDelegates())
		{
			OnCastReady.Broadcast(CastActor);
		}
	}

	EndTask();
}
