// Ninja Bear Studio Inc., all rights reserved.
#include "Animation/Notifies/AnimNotify_Cast.h"

#include "NinjaCombatTags.h"
#include "Animation/NinjaCombatAnimNotifyHelpers.h"
#include "Components/SkeletalMeshComponent.h"

UAnimNotify_Cast::UAnimNotify_Cast()
{
}

void UAnimNotify_Cast::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	FNinjaCombatAnimNotifyHelpers::BroadcastEvent(MeshComp->GetOwner(), Tag_Combat_Event_Cast);
}

FString UAnimNotify_Cast::GetNotifyName_Implementation() const
{
	return "Cast";
}
