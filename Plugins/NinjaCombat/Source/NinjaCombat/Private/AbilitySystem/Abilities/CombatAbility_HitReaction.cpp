// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_HitReaction.h"

#include "NinjaCombatTags.h"
#include "AbilitySystem/Providers/Animation/AnimationProvider_HitReaction.h"
#include "Kismet/KismetMathLibrary.h"

UCombatAbility_HitReaction::UCombatAbility_HitReaction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer
		.SetDefaultSubobjectClass<UAnimationProvider_HitReaction>(AnimationProviderName)
		.DoNotCreateDefaultSubobject(WarpTargetProviderName))
{
	bRotateToSource = true;
	bMatchAnimationEnd = true;
	bEnableMotionWarping = false;
	WarpTargetProvider = nullptr;
	
	AbilityTags.AddTag(Tag_Combat_Ability_HitReaction);

	// Configuration used for the ICombatAnimationContextProviderInterface.
	//
	// The Hit Reaction Animation Provider will use this to determine the type of damage
	// that was received and potentially react accordingly. For example, a melee damage
	// can trigger a certain animation but fire damage may trigger another.
	//
	TargetTagsFilter = FGameplayTagContainer::EmptyContainer;
	AnimationContext = FGameplayTagContainer::EmptyContainer;
	InstigatorTagsFilter = FGameplayTagContainer::EmptyContainer;
	InstigatorTagsFilter.AddTag(Tag_Combat_Effect_Damage);
	
	// Hit Reactions won't be activated if the character is attacking or evading. 
	// The Avatar will only be interrupted if staggered (poise is depleted).
	//
	// Obviously, you can change that behavior my modifying these tags to properly
	// suit your game design. For example, maybe you want hit reactions to always trigger.
	//
	ActivationBlockedTags.AddTag(Tag_Combat_Ability_Attack);
	ActivationBlockedTags.AddTag(Tag_Combat_Ability_Evade);

	// This ability activates via a trigger (Gameplay Event). This means it must be
	// assigned to the avatar as default, and it will just sit there waiting for the
	// appropriate event.
	//
	FAbilityTriggerData HitTrigger;
	HitTrigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	HitTrigger.TriggerTag = Tag_Combat_Event_Damage_Received;
	AbilityTriggers.Add(HitTrigger);
}

void UCombatAbility_HitReaction::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// The ability can only be activated by a Gameplay Event.
	//
	if (TriggerEventData == nullptr)
	{
		K2_CancelAbility();
	}

	// We can still have cost to this ability. Particularly useful if you want to
	// configure a cooldown to Hit Reactions, so they won't be triggered constantly.
	//
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		K2_CancelAbility();
	}

	// Watch the appropriate event (Damage.Received).
	//
	// In this structure, subclasses can simply watch for their own events.
	// At this point "TriggerEventData" is guaranteed to be valid.
	//
	if (TriggerEventData->EventTag == Tag_Combat_Event_Damage_Received)
	{
		ExecuteHitReaction(*TriggerEventData);
	}
}

void UCombatAbility_HitReaction::ExecuteHitReaction_Implementation(const FGameplayEventData& TriggerEventData)
{
	AnimationContext.Reset();
	CollectContext(TriggerEventData, AnimationContext);
		
	RotateToInstigator(TriggerEventData);
	PlayAnimationMontage();	
}

void UCombatAbility_HitReaction::RotateToInstigator_Implementation(const FGameplayEventData& TriggerEventData)
{
	if (bRotateToSource)
	{
		const AActor* DamageInstigator = TriggerEventData.ContextHandle.GetInstigator();
		if (IsValid(DamageInstigator))
		{
			AActor* MyAvatar = GetAvatarActorFromActorInfo();
			const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(MyAvatar->GetActorLocation(), DamageInstigator->GetActorLocation());
			MyAvatar->SetActorRotation(LookAtRotation);
		}
	}
}

void UCombatAbility_HitReaction::CollectContext_Implementation(const FGameplayEventData& TriggerEventData, FGameplayTagContainer& Context) const
{
	if (!InstigatorTagsFilter.IsEmpty())
	{
		const FGameplayTagContainer FilteredTags = TriggerEventData.InstigatorTags.Filter(InstigatorTagsFilter);
		Context.AppendTags(FilteredTags);
	}

	if (!TargetTagsFilter.IsEmpty())
	{
		const FGameplayTagContainer FilteredTags = TriggerEventData.InstigatorTags.Filter(TargetTagsFilter);
		Context.AppendTags(FilteredTags);
	}
}

void UCombatAbility_HitReaction::GetAnimationContext_Implementation(FGameplayTagContainer& OutContextTags) const
{
	OutContextTags = AnimationContext;
}
