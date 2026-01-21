// Ninja Bear Studio Inc., all rights reserved.
#include "Components/NinjaCombatMotionWarpingComponent.h"

#include "Kismet/KismetMathLibrary.h"

UNinjaCombatMotionWarpingComponent::UNinjaCombatMotionWarpingComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UNinjaCombatMotionWarpingComponent::SetCombatWarpTarget_Implementation(const FName WarpName, const AActor* Target, const float Offset)
{
	if (WarpName != NAME_None && IsValid(Target))
	{
		const FVector CalculatedWarpLocation = CalculateWarpLocation(Target, Offset);
		const FRotator CalculatedWarpRotation = CalculateWarpRotation(Target);
		AddOrUpdateWarpTargetFromLocationAndRotation(WarpName, CalculatedWarpLocation, CalculatedWarpRotation);
	}
}

void UNinjaCombatMotionWarpingComponent::ClearCombatWarpTarget_Implementation(const FName WarpName)
{
	RemoveWarpTarget(WarpName);
}

FVector UNinjaCombatMotionWarpingComponent::CalculateWarpLocation_Implementation(const AActor* Target, const float Offset) const
{
	check(IsValid(Target));

	const FVector AvatarLocation = GetOwner()->GetActorLocation();
	const FVector EnemyLocation = Target->GetActorLocation();

	FVector DirectionToEnemy = EnemyLocation - AvatarLocation;
	DirectionToEnemy.Normalize();

	FVector WarpLocation = EnemyLocation - DirectionToEnemy * Offset;
	WarpLocation.Z = EnemyLocation.Z;

	return WarpLocation;
}

FRotator UNinjaCombatMotionWarpingComponent::CalculateWarpRotation_Implementation(const AActor* Target) const
{
	check(IsValid(Target));

	const FVector AvatarLocation = GetOwner()->GetActorLocation();
	const FVector EnemyLocation = Target->GetActorLocation();

	return UKismetMathLibrary::FindLookAtRotation(AvatarLocation, EnemyLocation);
}
