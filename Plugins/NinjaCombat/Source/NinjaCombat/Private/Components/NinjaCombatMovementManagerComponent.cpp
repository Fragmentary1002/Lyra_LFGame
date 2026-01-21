// Ninja Bear Studio Inc., all rights reserved.
#include "Components/NinjaCombatMovementManagerComponent.h"

#include "NinjaCombatDelegates.h"
#include "NinjaCombatFunctionLibrary.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/Components/CombatDefenseManagerInterface.h"
#include "Interfaces/Components/CombatTargetManagerInterface.h"

UNinjaCombatMovementManagerComponent::UNinjaCombatMovementManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bIsBlocking = false;
	bIsStrafing = false;
	
	bStrafeWhileLockedOnTarget = true;
	bWalkWhileBlocking = true;
	BlockingWalkSpeed = 180.f;
	BlockingAcceleration = 1024.f;
	BlockingDeceleration = 1024.f;
	
	DefaultWalkSpeed = 0.f;
	DefaultAcceleration = 0.f;
	DefaultDeceleration = 0.f;
}

void UNinjaCombatMovementManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	const ACharacter* CharacterOwner = Cast<ACharacter>(GetOwner());
	UCharacterMovementComponent* CharacterMovement = IsValid(CharacterOwner) ? CharacterOwner->GetCharacterMovement() : nullptr;

	if (IsValid(CharacterMovement))
	{
		BackupMovementValues(CharacterMovement);
		OwnerCharacterMovementPtr = CharacterMovement;
	}

	BindToCombatSystem();
}

void UNinjaCombatMovementManagerComponent::HandleCombatTargetChanged_Implementation(const AActor* CombatTarget, const AActor* OldCombatTarget)
{
	const bool bShouldStrafe = IsValid(CombatTarget);
	SetStrafing(bShouldStrafe);
}

void UNinjaCombatMovementManagerComponent::HandleBlockingStateChanged_Implementation(const bool bNewIsBlocking)
{
	SetBlocking(bNewIsBlocking);
}

void UNinjaCombatMovementManagerComponent::SetBlocking(bool const bNewIsBlocking)
{
	bIsBlocking = bNewIsBlocking;
	
	UCharacterMovementComponent* CharacterMovement = GetCharacterMovementComponent();
	if (IsValid(CharacterMovement))
	{
		CharacterMovement->MaxWalkSpeed = bIsBlocking ? BlockingWalkSpeed : DefaultWalkSpeed;
		CharacterMovement->MaxAcceleration = bIsBlocking ? BlockingAcceleration : DefaultAcceleration;
		CharacterMovement->BrakingDecelerationWalking = bIsBlocking ? BlockingDeceleration : DefaultDeceleration;
	}
}

void UNinjaCombatMovementManagerComponent::SetStrafing(const bool bNewStrafing)
{
	bIsStrafing = bNewStrafing;

	UCharacterMovementComponent* CharacterMovement = GetCharacterMovementComponent();
	if (IsValid(CharacterMovement))
	{
		if (bIsStrafing)
		{
			CharacterMovement->bOrientRotationToMovement = false;
			CharacterMovement->bUseControllerDesiredRotation = true;
		}
		else
		{
			CharacterMovement->bOrientRotationToMovement = true;
			CharacterMovement->bUseControllerDesiredRotation = false;
		}
	}
}

void UNinjaCombatMovementManagerComponent::BindToCombatSystem()
{
	UActorComponent* DefenseComponent = UNinjaCombatFunctionLibrary::GetDefenseManagerComponent(GetOwner());
	if (IsValid(DefenseComponent))
	{
		FBlockingStateChangedDelegate BlockingStateChangedDelegate;
		BlockingStateChangedDelegate.BindDynamic(this, &ThisClass::HandleBlockingStateChanged);
		ICombatDefenseManagerInterface::Execute_BindToBlockingStateChangedDelegate(DefenseComponent, BlockingStateChangedDelegate);

		const bool bIsAlreadyBlocking = ICombatDefenseManagerInterface::Execute_IsBlocking(DefenseComponent);
		SetBlocking(bIsAlreadyBlocking);
	}
	
	UActorComponent* TargetComponent = UNinjaCombatFunctionLibrary::GetTargetManagerComponent(GetOwner());
	if (IsValid(TargetComponent))
	{
		FCombatTargetChangedDelegate TargetChangedDelegate;
		TargetChangedDelegate.BindDynamic(this, &ThisClass::HandleCombatTargetChanged);
		ICombatTargetManagerInterface::Execute_BindToCombatTargetChangedDelegate(TargetComponent, TargetChangedDelegate);
		
		const bool bIsAlreadyStrafing = ICombatTargetManagerInterface::Execute_HasCombatTarget(TargetComponent);
		SetStrafing(bIsAlreadyStrafing);
	}
}

void UNinjaCombatMovementManagerComponent::BackupMovementValues(const UCharacterMovementComponent* OwnerCharacterMovement)
{
	check(IsValid(OwnerCharacterMovement));

	DefaultWalkSpeed = OwnerCharacterMovement->MaxWalkSpeed;
	DefaultAcceleration = OwnerCharacterMovement->MaxAcceleration;
	DefaultDeceleration = OwnerCharacterMovement->BrakingDecelerationWalking;
}

UCharacterMovementComponent* UNinjaCombatMovementManagerComponent::GetCharacterMovementComponent() const
{
	if (OwnerCharacterMovementPtr.IsValid() && OwnerCharacterMovementPtr->IsValidLowLevelFast())
	{
		return OwnerCharacterMovementPtr.Get();
	}

	return nullptr;
}

