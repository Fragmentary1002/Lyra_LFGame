// Ninja Bear Studio Inc., all rights reserved.
#include "Animation/States/AnimNotifyState_WeaponTrail.h"

#include "NiagaraComponent.h"
#include "Animation/NinjaCombatAnimNotifyHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "Interfaces/CombatMeleeInterface.h"
#include "Kismet/KismetSystemLibrary.h"

UAnimNotifyState_WeaponTrail::UAnimNotifyState_WeaponTrail()
{
	WeaponQuery = FGameplayTagQuery::EmptyQuery;
	StartSocket = TEXT("sTrailStart");
	EndSocket = TEXT("sTrailEnd");
	TrailBeginParameterName = TEXT("StartTrail");
	TrailEndParameterName = TEXT("EndTrail");
}

void UAnimNotifyState_WeaponTrail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(MeshComp->GetWorld()))
	{
		const TScriptInterface<ICombatWeaponInterface> WeaponScript = FNinjaCombatAnimNotifyHelpers::GetWeapon(MeshComp->GetOwner(), WeaponQuery);
		if (WeaponScript != nullptr)
		{
			const TScriptInterface<ICombatMeleeInterface> MeleeScript = UNinjaCombatFunctionLibrary::GetMeleeScript(WeaponScript.GetObject());
			if (MeleeScript != nullptr)
			{
				WeaponMeshComponent = MeleeScript->Execute_GetMeleeMesh(WeaponScript.GetObject());
				TrailComponents = MeleeScript->Execute_GetAttackTrails(WeaponScript.GetObject());
				
				for (UNiagaraComponent* TrailComponent : TrailComponents)
				{
					TrailComponent->ActivateSystem(true);
				}				
			}
		}
	}
}

void UAnimNotifyState_WeaponTrail::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(MeshComp->GetWorld()))
	{
		if (IsValid(WeaponMeshComponent))
		{
			const FVector BeginLocation = WeaponMeshComponent->GetSocketLocation(StartSocket);
			const FVector EndLocation = WeaponMeshComponent->GetSocketLocation(EndSocket);
	
			for (UNiagaraComponent* TrailComponent : TrailComponents)
			{
				TrailComponent->SetVectorParameter(TrailBeginParameterName, BeginLocation);
				TrailComponent->SetVectorParameter(TrailEndParameterName, EndLocation);
			}
		}
	}
}

void UAnimNotifyState_WeaponTrail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!UKismetSystemLibrary::IsDedicatedServer(MeshComp->GetWorld()))
	{
		for (auto It = TrailComponents.CreateIterator(); It; ++It)
		{
			UNiagaraComponent* NiagaraComponent = *It;
			NiagaraComponent->Deactivate();
			It.RemoveCurrent();
		}
	
		WeaponMeshComponent = nullptr;
	}
}

FString UAnimNotifyState_WeaponTrail::GetNotifyName_Implementation() const
{
	return "Weapon Trail";
}
