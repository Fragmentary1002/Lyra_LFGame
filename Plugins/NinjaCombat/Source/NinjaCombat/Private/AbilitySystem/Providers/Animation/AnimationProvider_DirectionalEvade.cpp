// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Providers/Animation/AnimationProvider_DirectionalEvade.h"

#include "Components/ArrowComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Interfaces/CombatSystemInterface.h"

UAnimationProvider_DirectionalEvade::UAnimationProvider_DirectionalEvade()
{
	DefaultSectionName = "Backward";
	InitializeDirections(EvadeDirections);
}

FName UAnimationProvider_DirectionalEvade::GetSectionName_Implementation(UNinjaCombatGameplayAbility* CombatAbility) const
{
	float Angle;
	if (CalculateEvadeAngle(CombatAbility, Angle))
	{
		const int32 Index = EvadeDirections.IndexOfByPredicate([Angle](const FEvadeDirection& D) { return D.InRange(Angle); });
		if (Index != INDEX_NONE)
		{
			return EvadeDirections[Index].SectionName;
		}
	}

	return DefaultSectionName;
}

bool UAnimationProvider_DirectionalEvade::CalculateEvadeAngle_Implementation(UNinjaCombatGameplayAbility* CombatAbility, float& OutAngle) const
{
	const FGameplayAbilityActorInfo* MyActorInfo = CombatAbility->GetCurrentActorInfo();
	if (MyActorInfo == nullptr) return false;

	const UCharacterMovementComponent* Movement = Cast<UCharacterMovementComponent>(MyActorInfo->MovementComponent);
	if (Movement == nullptr) return false;

	FVector Acceleration = Movement->GetCurrentAcceleration();
	Acceleration.Normalize();

	if (Acceleration.IsZero())
	{
		OutAngle = 0.f;
		return false;
	}
	
	// First, determine the angle between the forward reference and the current actor rotation.
	//
	const UArrowComponent* ForwardReference = ICombatSystemInterface::Execute_GetCombatForwardReference(CombatAbility->GetAvatarActorFromActorInfo());
	const float Angle = ForwardReference->GetComponentRotation().Yaw - Movement->GetOwner()->GetActorRotation().Yaw;

	// Now Rotate the Acceleration Vector to the angle, on the Yaw Axis. This gives us an ABSOLUTE
	// angle for the CURRENT character rotation. That's what we need for our Direction Mapping.
	//
	const FVector YawAxis = FVector(0, 0, 1);
	const FVector RotatedInput = Acceleration.RotateAngleAxis(Angle, YawAxis);
	const FRotator InputOrientation = RotatedInput.ToOrientationRotator();

	// Since the evade happens horizontally, we only care about the yaw. This is our Evade Direction.
	//
	const float EvadeDirection = InputOrientation.Yaw;

	OutAngle = EvadeDirection;
	return true;	
}

void UAnimationProvider_DirectionalEvade::InitializeDirections(TArray<FEvadeDirection>& DirectionsArray)
{
	static const FName Forward = "Forward", Backward = "Backward", L045 = "Left_045", L090 = "Left_090",
		L135 = "Left_135", R045 = "Right_045", R090 = "Right_090", R135 = "Right_135";
			
	DirectionsArray.Reset(10);
	DirectionsArray.Add(FEvadeDirection(0.f, 35.f, Forward));
	DirectionsArray.Add(FEvadeDirection(-35.f, 0.f, Forward));
	DirectionsArray.Add(FEvadeDirection(-74.f, -36.f, L045));
	DirectionsArray.Add(FEvadeDirection(-125.f, -75.f, L090));
	DirectionsArray.Add(FEvadeDirection(-164.f, -126.f, L135));
	DirectionsArray.Add(FEvadeDirection(36.f, 74.f, R045));
	DirectionsArray.Add(FEvadeDirection(75.f, 125.f, R090));
	DirectionsArray.Add(FEvadeDirection(126.f, 164.f, R135));
	DirectionsArray.Add(FEvadeDirection(165.f, 180.f, Backward));
	DirectionsArray.Add(FEvadeDirection(-180.f, -165.f, Backward));	
}
