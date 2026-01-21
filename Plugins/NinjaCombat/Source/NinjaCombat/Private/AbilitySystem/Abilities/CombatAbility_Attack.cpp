// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_Attack.h"

#include "NinjaCombatFunctionLibrary.h"
#include "NinjaCombatTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tasks/AbilityTask_ScanMeleeTarget.h"
#include "AbilitySystem/Tasks/AbilityTask_SpawnProjectile.h"
#include "Components/NinjaCombatComboManagerComponent.h"
#include "GameFramework/NinjaCombatMeleeScan.h"
#include "GameFramework/NinjaCombatProjectileRequest.h"
#include "Interfaces/CombatProjectileInterface.h"

UCombatAbility_Attack::UCombatAbility_Attack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsMeleeAttack = true;
	bIsRangedAttack = true;
	DefaultMeleeEffectLevel = 1.f;
	DefaultProjectileEffectLevel = 1.f;
	AbilityTags.AddTag(Tag_Combat_Ability_Attack);
}

int32 UCombatAbility_Attack::GetComboCounter() const
{
	const AActor* MyAvatar = GetAvatarActorFromActorInfo();
	
	const UActorComponent* ComboComponent = UNinjaCombatFunctionLibrary::GetComboManagerComponent(MyAvatar);
	if (IsValid(ComboComponent) && ensure(ComboComponent->Implements<UCombatComboManagerInterface>()))
	{
		return ICombatComboManagerInterface::Execute_GetComboCount(ComboComponent);
	}

	return 0;
}

void UCombatAbility_Attack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (bIsMeleeAttack)
	{
		static constexpr bool bExactOnly = false;
		MeleeScanEventTask = InitializeEventTask(Tag_Combat_Event_MeleeScan, bExactOnly);
		MeleeScanEventTask->ReadyForActivation();
	}

	if (bIsRangedAttack)
	{
		LaunchProjectileEventTask = InitializeEventTask(Tag_Combat_Event_Projectile_Launch);
		LaunchProjectileEventTask->ReadyForActivation();	
	}
}

UAbilityTask_ScanMeleeTarget* UCombatAbility_Attack::InitializeMeleeScanTask(const UNinjaCombatMeleeScan* MeleeScan)
{
	UAbilityTask_ScanMeleeTarget* Task = UAbilityTask_ScanMeleeTarget::CreateTask(this, bEnableDebug);
	checkf(IsValid(Task), TEXT("Unexpected invalid Melee Scan Task instance."));

	Task->Join(MeleeScan);
	Task->OnMeleeTargetsFound.AddDynamic(this, &ThisClass::HandleMeleeScanTargetsReceived);
	return Task;	
}

UAbilityTask_SpawnProjectile* UCombatAbility_Attack::InitializeProjectileTask(
	const UNinjaCombatProjectileRequest* ProjectileRequest)
{
	UAbilityTask_SpawnProjectile* Task = UAbilityTask_SpawnProjectile::CreateTask(this, bEnableDebug);
	checkf(IsValid(Task), TEXT("Unexpected invalid Spawn Projectile Task instance."));

	Task->Join(ProjectileRequest);
	Task->OnProjectileLaunched.AddDynamic(this, &ThisClass::HandleProjectileLaunched);
	return Task;
}

void UCombatAbility_Attack::HandleEventReceived_Implementation(const FGameplayEventData Payload)
{
	Super::HandleEventReceived_Implementation(Payload);
	
	if (bIsMeleeAttack)
	{
		if (Payload.EventTag == Tag_Combat_Event_MeleeScan_Start)
		{
			const UNinjaCombatMeleeScan* Scan = Cast<UNinjaCombatMeleeScan>(Payload.OptionalObject);
			if (ensureMsgf(IsValid(Scan) && Scan->HasValidScanData(), TEXT("Missing or invalid Melee Scan.")))
			{
				if (IsValid(MeleeScanTask) && MeleeScanTask->IsActive())
				{
					MeleeScanTask->Join(Scan);	
				}
				else
				{
					FinishLatentTask(MeleeScanTask);
					MeleeScanTask = InitializeMeleeScanTask(Scan);
					MeleeScanTask->ReadyForActivation();
				}
			}
		}
		else if (Payload.EventTag == Tag_Combat_Event_MeleeScan_Stop)
		{
			FinishLatentTask(MeleeScanTask);
		}	
	}
	
	if (bIsRangedAttack)
	{
		const FGameplayAbilityActivationInfo Info = GetCurrentActivationInfo();
		if (Payload.EventTag == Tag_Combat_Event_Projectile_Launch && HasAuthority(&Info))
		{
			const UNinjaCombatProjectileRequest* Request = GetProjectileRequest(Payload);
			if (ensureMsgf(IsValid(Request) && Request->HasValidRequestData(), TEXT("Missing or invalid Projectile Request.")))
			{
				if (IsValid(SpawnProjectileTask) && SpawnProjectileTask->IsActive())
				{
					SpawnProjectileTask->Join(Request);
				}
				else
				{
					FinishLatentTask(SpawnProjectileTask);
					SpawnProjectileTask = InitializeProjectileTask(Request);
					SpawnProjectileTask->ReadyForActivation();
				}
			}
		}
	}
}

UNinjaCombatProjectileRequest* UCombatAbility_Attack::GetProjectileRequest(const FGameplayEventData& Payload) const
{
	if (IsValid(Payload.OptionalObject))
	{
		UObject* OptionalPayload = const_cast<UObject*>(Payload.OptionalObject.Get());
		UNinjaCombatProjectileRequest* Request = Cast<UNinjaCombatProjectileRequest>(OptionalPayload);
		
		if (IsValid(Request))
		{
			Request->SetProjectileClassFallback(ProjectileClass);
		}

		return Request;
	}

	return nullptr;
}

void UCombatAbility_Attack::HandleMeleeScanTargetsReceived_Implementation(const UNinjaCombatMeleeScan* MeleeScan, 
	const FGameplayAbilityTargetDataHandle& Data)
{
	for (int32 Idx = 0; Idx < Data.Num(); ++Idx)
	{
		const FHitResult* HitResult = Data.Get(Idx)->GetHitResult();
		if (HitResult != nullptr)
		{
			TObjectPtr<AActor> Target = HitResult->GetActor();
			if (IsValid(Target))
			{
				FGameplayEffectSpecHandle SpecHandle;
				if (CreateSpecHandleForMeleeHit(MeleeScan, SpecHandle) && SpecHandle.IsValid())
				{
					ApplyGameplayEffectSpecToHitResult(SpecHandle, *HitResult);
				}
			}
		}
	}	
}

void UCombatAbility_Attack::HandleProjectileLaunched_Implementation(const UNinjaCombatProjectileRequest* Request, AActor* Projectile)
{
	if (ensure(IsValid(Projectile)))
	{
		const TSubclassOf<UGameplayEffect> RangedDamageEffect = GetProjectileGameplayEffectClass(Projectile);
		const float EffectLevel = GetProjectileGameplayEffectLevel(Projectile);
		
		if (IsValid(RangedDamageEffect))
		{
			FGameplayEffectSpecHandle Handle = MakeOutgoingGameplayEffectSpec(RangedDamageEffect, EffectLevel);
			if (Handle.IsValid())
			{
				Handle.Data.Get()->GetContext().AddInstigator(GetAvatarActorFromActorInfo(), Projectile);
				ApplyDamageValues(Projectile, Handle);
				ICombatProjectileInterface::Execute_SetImpactEffectHandle(Projectile, Handle);
			}
		}

		ICombatProjectileInterface::Execute_Launch(Projectile);
	}
}

bool UCombatAbility_Attack::CreateSpecHandleForMeleeHit(const UNinjaCombatMeleeScan* MeleeScan, FGameplayEffectSpecHandle& OutHandle) const
{
	if (ensure(IsValid(MeleeScan)))
	{
		AActor* EffectCauser = MeleeScan->GetCauser();
		if (IsValid(EffectCauser))
		{
			const TSubclassOf<UGameplayEffect> HitEffect = GetMeleeGameplayEffectClass(EffectCauser);
			const float EffectLevel = GetMeleeGameplayEffectLevel(EffectCauser);

			if (IsValid(HitEffect))
			{
				OutHandle = MakeOutgoingGameplayEffectSpec(HitEffect, EffectLevel);
				if (OutHandle.IsValid())
				{
					OutHandle.Data.Get()->GetContext().AddInstigator(GetAvatarActorFromActorInfo(), EffectCauser);
					ApplyDamageValues(EffectCauser, OutHandle);
					return true;
				}
			}
		}
	}

	return false;	
}

TArray<FActiveGameplayEffectHandle> UCombatAbility_Attack::ApplyGameplayEffectSpecToHitResult(
	const FGameplayEffectSpecHandle& SpecHandle, const FHitResult& HitResult)
{
	FGameplayAbilityTargetingLocationInfo TargetInfo;
	TargetInfo.LocationType = EGameplayAbilityTargetingLocationType::ActorTransform;
	TargetInfo.SourceActor = GetAvatarActorFromActorInfo();
	TargetInfo.SourceAbility = this;

	const FGameplayAbilityTargetDataHandle TargetData = TargetInfo.MakeTargetDataHandleFromHitResult(this, HitResult);
	return ApplyGameplayEffectSpecToTarget(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), SpecHandle, TargetData);	
}

void UCombatAbility_Attack::ApplyDamageValues(const AActor* DamageSource, FGameplayEffectSpecHandle& SpecHandle) const
{
	check(IsValid(DamageSource));

	static const FGameplayTag ComboCounterDataTag = Tag_Combat_Data_ComboCounter.GetTag(); 
	SpecHandle.Data.Get()->SetSetByCallerMagnitude(ComboCounterDataTag, GetComboCounter());
	
	if (DamageSource->Implements<UCombatMeleeInterface>())
	{
		float Damage = 0.f;
		if (ICombatMeleeInterface::Execute_GetDamage(DamageSource, Damage))
		{
			static const FGameplayTag DamageDataTag = Tag_Combat_Data_Damage.GetTag(); 
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageDataTag, Damage);
		}

		float PoiseConsumption = 0.f;
		if (ICombatMeleeInterface::Execute_GetPoiseConsumption(DamageSource, PoiseConsumption))
		{
			static const FGameplayTag PoiseConsumptionDataTag = Tag_Combat_Data_PoiseConsumption.GetTag(); 
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(PoiseConsumptionDataTag, PoiseConsumption);
		}		
	}
	else if (DamageSource->Implements<UCombatProjectileInterface>())
	{
		float Damage = 0.f;
		if (ICombatProjectileInterface::Execute_GetDamage(DamageSource, Damage))
		{
			static const FGameplayTag DamageDataTag = Tag_Combat_Data_Damage.GetTag(); 
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(DamageDataTag, Damage);
		}

		float PoiseConsumption = 0.f;
		if (ICombatProjectileInterface::Execute_GetPoiseConsumption(DamageSource, PoiseConsumption))
		{
			static const FGameplayTag PoiseConsumptionDataTag = Tag_Combat_Data_PoiseConsumption.GetTag(); 
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(PoiseConsumptionDataTag, PoiseConsumption);
		}
	}
}

void UCombatAbility_Attack::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	FinishLatentTasks({ MeleeScanEventTask, LaunchProjectileEventTask, MeleeScanTask, SpawnProjectileTask });	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

TSubclassOf<UGameplayEffect> UCombatAbility_Attack::GetMeleeGameplayEffectClass_Implementation(const AActor* EffectCauser) const
{
	if (IsValid(EffectCauser) && EffectCauser->Implements<UCombatMeleeInterface>())
	{
		const TSubclassOf<UGameplayEffect> EffectClassOverride = ICombatMeleeInterface::Execute_GetHitEffectClass(EffectCauser);
		if (IsValid(EffectClassOverride))
		{
			return EffectClassOverride;
		}
	}
	
	return MeleeDamageEffectClass;
}

float UCombatAbility_Attack::GetMeleeGameplayEffectLevel_Implementation(const AActor* EffectCauser) const
{
	if (IsValid(EffectCauser) && EffectCauser->Implements<UCombatMeleeInterface>())
	{
		const float EffectLevel = ICombatMeleeInterface::Execute_GetHitEffectLevel(EffectCauser);
		if (EffectLevel > 0.f)
		{
			return EffectLevel;
		}
	}
	
	return DefaultMeleeEffectLevel;
}

TSubclassOf<UGameplayEffect> UCombatAbility_Attack::GetProjectileGameplayEffectClass_Implementation(const AActor* Projectile) const
{
	if (IsValid(Projectile) && Projectile->Implements<UCombatProjectileInterface>())
	{
		const TSubclassOf<UGameplayEffect> EffectClassOverride = ICombatProjectileInterface::Execute_GetImpactEffectClass(Projectile);
		if (IsValid(EffectClassOverride))
		{
			return EffectClassOverride;
		}
	}
	
	return ProjectileDamageEffectClass;
}

float UCombatAbility_Attack::GetProjectileGameplayEffectLevel_Implementation(const AActor* Projectile) const
{
	if (IsValid(Projectile) && Projectile->Implements<UCombatProjectileInterface>())
	{
		const float EffectLevel = ICombatProjectileInterface::Execute_GetImpactEffectLevel(Projectile);
		if (EffectLevel > 0.f)
		{
			return EffectLevel;
		}
	}
	
	return DefaultProjectileEffectLevel;
}