// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaCombatCastActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"

ANinjaCombatCastActor::ANinjaCombatCastActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	bReplicates = true;
	bHasStarted = false;

	CastSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CastSphere"));
	CastSphere->SetSphereRadius(60.f);
	CastSphere->SetAutoActivate(false);
	SetRootComponent(CastSphere);
}

void ANinjaCombatCastActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ThisClass, bHasStarted);
	DOREPLIFETIME(ThisClass, CastSourcePtr);
}

void ANinjaCombatCastActor::SetCastSource_Implementation(AActor* SourceActor)
{
	if (IsValid(SourceActor))
	{
		CastSourcePtr = SourceActor;	
	}
}

void ANinjaCombatCastActor::SetOwnerEffectHandle_Implementation(FGameplayEffectSpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		OwnerHandle = Handle;	
	}
}

void ANinjaCombatCastActor::SetTargetEffectHandle_Implementation(FGameplayEffectSpecHandle& Handle)
{
	if (Handle.IsValid())
	{
		TargetHandle = Handle;	
	}
}

void ANinjaCombatCastActor::StartCast_Implementation()
{
	if (!bHasStarted && HasAuthority())
	{
		bHasStarted = true;
		OnRep_HasStarted();
	}
}

AActor* ANinjaCombatCastActor::GetCastSource() const
{
	if (CastSourcePtr.IsValid() && CastSourcePtr->IsValidLowLevelFast())
	{
		return CastSourcePtr.Get();
	}

	return nullptr;
}

void ANinjaCombatCastActor::ApplyEffectToSelf()
{
	if (HasAuthority() && OwnerHandle.IsValid())
	{
		const AActor* CastSource = GetCastSource();
		if (IsValid(CastSource))
		{
			UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(CastSource);
			if (IsValid(AbilityComponent))
			{
				AbilityComponent->BP_ApplyGameplayEffectSpecToSelf(OwnerHandle);
			}
		}
	}
}

void ANinjaCombatCastActor::ApplyEffectToTarget(AActor* Target)
{
	if (HasAuthority() && TargetHandle.IsValid() && IsValid(Target) && Target != GetCastSource())
	{
		UAbilitySystemComponent* AbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Target);
		if (IsValid(AbilityComponent))
		{
			AbilityComponent->BP_ApplyGameplayEffectSpecToSelf(TargetHandle);
		}
	}
}

void ANinjaCombatCastActor::OnRep_HasStarted()
{
	if (IsValid(CastSphere))
	{
		if (bHasStarted)
		{
			CastSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleOverlap);
			CastSphere->Activate();

			TArray<AActor*> CurrentOverlaps;
			CastSphere->GetOverlappingActors(CurrentOverlaps);
			for (AActor* Overlap : CurrentOverlaps)
			{
				ApplyEffectToTarget(Overlap);
			}
		}
		else
		{
			CastSphere->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::HandleOverlap);
			CastSphere->Deactivate();
		}
	}
}

bool ANinjaCombatCastActor::ShouldApplyToTarget_Implementation(AActor* Target)
{
	return true;
}

void ANinjaCombatCastActor::HandleOverlap_Implementation(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == GetCastSource())
	{
		ApplyEffectToSelf();
	}
	else if (ShouldApplyToTarget(OtherActor))
	{
		ApplyEffectToTarget(OtherActor);
	}
}
