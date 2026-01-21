// Ninja Bear Studio Inc., all rights reserved.
#include "Interfaces/CombatSystemInterface.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NinjaCombatTags.h"
#include "Components/ArrowComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Interfaces/Components/CombatComboManagerInterface.h"
#include "Interfaces/Components/CombatDamageManagerInterface.h"
#include "Interfaces/Components/CombatDefenseManagerInterface.h"
#include "Interfaces/Components/CombatMotionWarpingInterface.h"
#include "Interfaces/Components/CombatMovementManagerInterface.h"
#include "Interfaces/Components/CombatPhysicalAnimationInterface.h"
#include "Interfaces/Components/CombatTargetManagerInterface.h"
#include "Interfaces/Components/CombatWeaponManagerInterface.h"

USkeletalMeshComponent* ICombatSystemInterface::GetCombatMesh_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	if (IsValid(SelfActor))
	{
		// First Attempt: Find a component with the combat Tag.
		//
		const FName CombatTag = Tag_Combat_Component_Mesh.GetTag().GetTagName();
		USkeletalMeshComponent* Mesh = SelfActor->FindComponentByTag<USkeletalMeshComponent>(CombatTag);

		// Second Attempt: Use the tag configured in the Ability System Component, if not "NONE".
		//
		if (!IsValid(Mesh))
		{
			const UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(SelfActor);
			if (IsValid(AbilityComponent) && AbilityComponent->AffectedAnimInstanceTag != NAME_None)
			{
				const FName AbilityTag = AbilityComponent->AffectedAnimInstanceTag;
				Mesh = SelfActor->FindComponentByTag<USkeletalMeshComponent>(AbilityTag);
			}
		}
	
		// Third Attempt: Cast to a character and return the main mesh.
		//
		if (!IsValid(Mesh))
		{
			const ACharacter* Character = Cast<ACharacter>(SelfActor);
			Mesh = IsValid(Character) ? Character->GetMesh() : nullptr;
		}
	
		return Mesh;
	}

	return nullptr;
}

UAnimInstance* ICombatSystemInterface::GetCombatAnimInstance_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	if (IsValid(SelfActor))
	{
		const USkeletalMeshComponent* Mesh = Execute_GetCombatMesh(SelfActor);
		return IsValid(Mesh) ? Mesh->GetAnimInstance() : nullptr;
	}

	return nullptr;
}

UArrowComponent* ICombatSystemInterface::GetCombatForwardReference_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	if (IsValid(SelfActor))
	{
		const FName ForwardTag = Tag_Combat_Component_ForwardReference.GetTag().GetTagName();
		return SelfActor->FindComponentByTag<UArrowComponent>(ForwardTag);
	}

	return nullptr;
}

UActorComponent* ICombatSystemInterface::GetComboManagerComponent_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	return IsValid(SelfActor) ? SelfActor->FindComponentByInterface(UCombatComboManagerInterface::StaticClass()) : nullptr;	
}

UActorComponent* ICombatSystemInterface::GetDamageManagerComponent_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	return IsValid(SelfActor) ? SelfActor->FindComponentByInterface(UCombatDamageManagerInterface::StaticClass()) : nullptr;	
}

UActorComponent* ICombatSystemInterface::GetDefenseManagerComponent_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	return IsValid(SelfActor) ? SelfActor->FindComponentByInterface(UCombatDefenseManagerInterface::StaticClass()) : nullptr;
}

UActorComponent* ICombatSystemInterface::GetMotionWarpingComponent_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	return IsValid(SelfActor) ? SelfActor->FindComponentByInterface(UCombatMotionWarpingInterface::StaticClass()) : nullptr;	
}

UActorComponent* ICombatSystemInterface::GetMovementManagerComponent_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	return IsValid(SelfActor) ? SelfActor->FindComponentByInterface(UCombatMovementManagerInterface::StaticClass()) : nullptr;	
}

UActorComponent* ICombatSystemInterface::GetPhysicalAnimationComponent_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	return IsValid(SelfActor) ? SelfActor->FindComponentByInterface(UCombatPhysicalAnimationInterface::StaticClass()) : nullptr;	
}

UActorComponent* ICombatSystemInterface::GetTargetManagerComponent_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	return IsValid(SelfActor) ? SelfActor->FindComponentByInterface(UCombatTargetManagerInterface::StaticClass()) : nullptr;	
}

UActorComponent* ICombatSystemInterface::GetWeaponManagerComponent_Implementation() const
{
	const AActor* SelfActor = Cast<AActor>(this);
	return IsValid(SelfActor) ? SelfActor->FindComponentByInterface(UCombatWeaponManagerInterface::StaticClass()) : nullptr;	
}
