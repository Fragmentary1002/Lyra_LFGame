// Ninja Bear Studio Inc., all rights reserved.
#include "Components/NinjaCombatWeaponManagerComponent.h"

#include "Interfaces/CombatWeaponInterface.h"
#include "Net/UnrealNetwork.h"

UNinjaCombatWeaponManagerComponent::UNinjaCombatWeaponManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetIsReplicatedByDefault(true);
}

void UNinjaCombatWeaponManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, Weapons);
}

void UNinjaCombatWeaponManagerComponent::BeginPlay()
{
	Super::BeginPlay();
	LoadAttachedWeapons();
	InitializeWeapons();
}

AActor* UNinjaCombatWeaponManagerComponent::GetWeapon_Implementation(const FGameplayTagQuery& WeaponQuery) const
{
	for (AActor* WeaponInstance : Weapons)
	{
		FGameplayTagContainer WeaponTags = ICombatWeaponInterface::Execute_GetWeaponTags(WeaponInstance);
		if (WeaponQuery.Matches(WeaponTags))
		{
			return WeaponInstance;
		}
	}
	
	return nullptr;
}

void UNinjaCombatWeaponManagerComponent::LoadAttachedWeapons_Implementation()
{
	if (OwnerHasAuthority())
	{
		TArray<AActor*> AttachedActors;
		GetOwner()->GetAttachedActors(AttachedActors);
		
		for (AActor* AttachedActor : AttachedActors)
		{
			if (AttachedActor->Implements<UCombatWeaponInterface>())
			{
				Weapons.AddUnique(AttachedActor);
				OnRep_Weapons();
			}
		}
	}	
}

void UNinjaCombatWeaponManagerComponent::InitializeWeapons_Implementation()
{
	if (OwnerHasAuthority())
	{
		for (TSubclassOf<AActor>& WeaponClass : DefaultWeaponClasses)
		{
			if (ensure(WeaponClass && WeaponClass->ImplementsInterface(UCombatWeaponInterface::StaticClass())))
			{
				AActor* NewWeapon = NewObject<AActor>(GetOwner(), WeaponClass);
				Weapons.AddUnique(NewWeapon);
				OnRep_Weapons();
			}
		}
	}
}

void UNinjaCombatWeaponManagerComponent::OnRep_Weapons_Implementation()
{
}
