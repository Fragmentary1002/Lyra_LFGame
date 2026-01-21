// Ninja Bear Studio Inc., all rights reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "CombatEffect_InitializeAttributes.generated.h"

/**
 * A base template to create effects used to initialize combat attributes.
 * 
 * This will initialize the foundation attributes, but due to the way the "Total" attributes are calculated,
 * current health/stamina/magic are initializes later, if this main effect succeeds.
 */
UCLASS()
class NINJACOMBAT_API UCombatEffect_InitializeAttributes : public UGameplayEffect
{
	
	GENERATED_BODY()

public:

	UCombatEffect_InitializeAttributes();

protected:

	void SetupAttributeWithFloat(const FGameplayAttribute& Attribute, float Value);

};

/**
 * Initializes the Health Attribute from the Total Max Health.
 */
UCLASS()
class NINJACOMBAT_API UCombatEffect_InitializeHealthAttribute : public UGameplayEffect
{
	
	GENERATED_BODY()

public:

	UCombatEffect_InitializeHealthAttribute();

};

/**
 * Initializes the Stamina Attribute from the Total Max Health.
 */
UCLASS()
class NINJACOMBAT_API UCombatEffect_InitializeStaminaAttribute : public UGameplayEffect
{
	
	GENERATED_BODY()

public:

	UCombatEffect_InitializeStaminaAttribute();

};

/**
 * Initializes the Magic Attribute from the Total Max Health.
 */
UCLASS()
class NINJACOMBAT_API UCombatEffect_InitializeMagicAttribute : public UGameplayEffect
{
	
	GENERATED_BODY()

public:

	UCombatEffect_InitializeMagicAttribute();

};