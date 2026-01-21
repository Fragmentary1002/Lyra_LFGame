// Ninja Bear Studio Inc., all rights reserved.
#include "Animation/States/AnimNotifyState_MeleeScan.h"

#include "AbilitySystemComponent.h"
#include "NinjaCombatSettings.h"
#include "NinjaCombatTags.h"
#include "Animation/NinjaCombatAnimNotifyHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/NinjaCombatMeleeScan.h"

UAnimNotifyState_MeleeScan::UAnimNotifyState_MeleeScan()
{
	Source = ECombatAbilitySource::Weapon;
	WeaponQuery = FGameplayTagQuery::EmptyQuery;
	ScanChannel = GetDefault<UNinjaCombatSettings>()->MeleeScanChannel;
	ScanMode = EMeleeScanMode::LineTrace;
	SphereRadius = 20.f;
	BoxHalfExtent = FVector::OneVector;
	CapsuleExtent = FVector::OneVector;
	MeleeScanClass = GetDefault<UNinjaCombatSettings>()->MeleeScanClass;
}

void UAnimNotifyState_MeleeScan::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	const AActor* Owner = MeshComp->GetOwner();

	AActor* Causer = MeshComp->GetOwner();
	UMeshComponent* SourceMesh = nullptr;

	if (Source == ECombatAbilitySource::Weapon)
	{
		const TScriptInterface<ICombatWeaponInterface> WeaponScript = FNinjaCombatAnimNotifyHelpers::GetWeapon(Owner, WeaponQuery);
		if (WeaponScript != nullptr)
		{
			Causer = Cast<AActor>(WeaponScript.GetObject());
			SourceMesh = GetMeleeScanMesh(Causer);
		}
	}
	else
	{
		SourceMesh = GetMeleeScanMesh(Owner);
	}

	if (IsValid(SourceMesh))
	{
		const UNinjaCombatMeleeScan* MeleeScan = CreateMeleeScanInstance(MeshComp->GetOwner(), Causer, SourceMesh);
		FNinjaCombatAnimNotifyHelpers::BroadcastEvent(MeshComp->GetOwner(), Tag_Combat_Event_MeleeScan_Start, MeleeScan);
	}
}

UMeshComponent* UAnimNotifyState_MeleeScan::GetMeleeScanMesh(const AActor* Owner)
{
	if (IsValid(Owner))
	{
		// First use the interface as may provide fast access to the component or custom logic for the mesh.
		if (Owner->Implements<UCombatMeleeInterface>())
		{
			return ICombatMeleeInterface::Execute_GetMeleeMesh(Owner);	
		}

		// As a fallback, try to find the component tagged with the melee scan source tag. Not ideal.
		const AActor* OwnerActor = Cast<AActor>(Owner);
		if (IsValid(OwnerActor))
		{
			const FName Tag = Tag_Combat_Component_MeleeScanSource.GetTag().GetTagName();
			return OwnerActor->FindComponentByTag<UMeshComponent>(Tag);
		}
	}
	
	return nullptr;
}

UNinjaCombatMeleeScan* UAnimNotifyState_MeleeScan::CreateMeleeScanInstance_Implementation(AActor* Owner, AActor* Causer, UMeshComponent* SourceMesh) const
{
	TSubclassOf<UNinjaCombatMeleeScan> ScanClass = MeleeScanClass; 
	if (!IsValid(ScanClass))
	{
		// Use the default class since an actual scan class was not provided.
		ScanClass = UNinjaCombatMeleeScan::StaticClass();
	}

	FVector ShapeDimensions = FVector::ZeroVector;
	switch (ScanMode)
	{
		case EMeleeScanMode::BoxSweep:
			ShapeDimensions = BoxHalfExtent;
			break;
		case EMeleeScanMode::CapsuleSweep:
			ShapeDimensions = CapsuleExtent;
			break;
		case EMeleeScanMode::SphereSweep:
			ShapeDimensions.X = SphereRadius;
			break;
		default: ;
	}
	
	return UNinjaCombatMeleeScan::NewInstance(MeleeScanClass, Owner, Owner, Causer, SourceMesh,
		ScanSocketNames, ScanChannel, ScanMode, ShapeDimensions);
}

void UAnimNotifyState_MeleeScan::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	FNinjaCombatAnimNotifyHelpers::BroadcastEvent(MeshComp->GetOwner(), Tag_Combat_Event_MeleeScan_Stop);
}

FString UAnimNotifyState_MeleeScan::GetNotifyName_Implementation() const
{
	return "Melee Scan";
}
