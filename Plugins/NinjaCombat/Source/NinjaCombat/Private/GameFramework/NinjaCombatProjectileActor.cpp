// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaCombatProjectileActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NinjaCombatSettings.h"
#include "Components/SphereComponent.h"
#include "Engine/AssetManager.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundBase.h"
#include "TargetingSystem/TargetingPreset.h"
#include "TargetingSystem/TargetingSubsystem.h"

ANinjaCombatProjectileActor::ANinjaCombatProjectileActor(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	bReplicates = true;
	bEnableDebug = false;
	bUsesTargetingSystem = false;
	bExecuteAsync = true;
	TargetIndex = 0;
	BounceCount = 0;
	MaxBounces = 5;
	ImpactStrength = 1000000.f;
	TraceChannel = GetDefault<UNinjaCombatSettings>()->ProjectileChannel;
	TargetingPreset = nullptr;
	SetReplicatingMovement(true);
	
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetSphereRadius(25.f);
	SphereComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	SphereComponent->SetCanEverAffectNavigation(false);
	SetRootComponent(SphereComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->bShouldBounce = true;
	ProjectileMovement->bInterpMovement = true;
	ProjectileMovement->bInterpRotation = true;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->Friction = 0.f;
	ProjectileMovement->BounceVelocityStopSimulatingThreshold = 500.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->Bounciness = 1.f;
	ProjectileMovement->HomingAccelerationMagnitude = 100000.f;
	ProjectileMovement->SetIsReplicated(true);
}

void ANinjaCombatProjectileActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(SphereComponent))
	{
		SphereComponent->MoveIgnoreActors.Add(GetInstigator());
		GetInstigator()->MoveIgnoreActorAdd(this);

		TArray<AActor*> AttachedActors;
		GetInstigator()->GetAttachedActors(AttachedActors);
		SphereComponent->MoveIgnoreActors.Append(AttachedActors);
	}
	
	if (IsValid(ProjectileMovement))
	{
		// Bind the delegates to the internal Handlers so we can always ensure internal state is correct.
		ProjectileMovement->OnProjectileStop.AddUniqueDynamic(this, &ThisClass::OnProjectileStop);
		ProjectileMovement->OnProjectileBounce.AddUniqueDynamic(this, &ThisClass::OnProjectileBounce);
	}
}

void ANinjaCombatProjectileActor::Launch_Implementation()
{
	if (bUsesTargetingSystem && TargetingPreset)
	{
		ExecuteTargeting();
	}
	else
	{
		LaunchForward();
	}
}

void ANinjaCombatProjectileActor::LaunchForward_Implementation()
{
	if (BounceCount == 0)
	{
		const FRotator InitialRotation = GetInitialRotation();
		const FVector RotationVector = UKismetMathLibrary::GetForwardVector(InitialRotation);
		const float Speed = ProjectileMovement->InitialSpeed;
		const FVector Velocity = Speed * RotationVector;
		ProjectileMovement->Velocity = Velocity;
	}
}

FRotator ANinjaCombatProjectileActor::GetInitialRotation_Implementation() const
{
	return GetActorRotation();
}

void ANinjaCombatProjectileActor::ExecuteTargeting()
{
	check(IsValid(TargetingPreset));
	
	UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::Get(GetWorld());
	check(IsValid(TargetingSubsystem));
	
	FTargetingSourceContext SourceContext;
	SourceContext.SourceActor = this;
	SourceContext.SourceLocation = GetActorLocation();
	SourceContext.InstigatorActor = GetInstigator();

	const FTargetingRequestHandle TargetingHandle = UTargetingSubsystem::MakeTargetRequestHandle(TargetingPreset, SourceContext);
	
	const FTargetingRequestDelegate Delegate = FTargetingRequestDelegate::CreateWeakLambda(this, [this](const FTargetingRequestHandle& Handle)
		{ HandleTargetReceived(Handle); });

	if (bExecuteAsync)
	{
		FTargetingAsyncTaskData& AsyncTaskData = FTargetingAsyncTaskData::FindOrAdd(TargetingHandle);
		AsyncTaskData.bReleaseOnCompletion = true;
		TargetingSubsystem->StartAsyncTargetingRequestWithHandle(TargetingHandle, Delegate);
	}
	else
	{
		FTargetingImmediateTaskData& ImmediateTaskData = FTargetingImmediateTaskData::FindOrAdd(TargetingHandle);
		ImmediateTaskData.bReleaseOnCompletion = true;
		TargetingSubsystem->ExecuteTargetingRequestWithHandle(TargetingHandle, Delegate);
	}	
}

void ANinjaCombatProjectileActor::HandleTargetReceived(const FTargetingRequestHandle& TargetingHandle)
{
	TArray<AActor*> TargetedActors;
	
	const UTargetingSubsystem* TargetingSubsystem = UTargetingSubsystem::Get(GetWorld());
	check(IsValid(TargetingSubsystem));

	TargetingSubsystem->GetTargetingResultsActors(TargetingHandle, TargetedActors);
	if (TargetedActors.IsEmpty())
	{
		CurrentTarget = nullptr;
		LaunchForward();
		return;
	}

	CurrentTarget = TargetedActors[0];
	LaunchToTarget(CurrentTarget);
}

void ANinjaCombatProjectileActor::LaunchToTarget_Implementation(const AActor* Target)
{
	check(IsValid(Target));
	
	ProjectileMovement->StopMovementImmediately();
	
	const FRotator Direction = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation());
	SetActorRotation(Direction);

	const float Speed = BounceCount > 0 ? LastBounceVelocity.Size() : ProjectileMovement->InitialSpeed;
	const FVector Velocity = Speed * UKismetMathLibrary::GetForwardVector(GetActorRotation());
	ProjectileMovement->Velocity = Velocity;
}

void ANinjaCombatProjectileActor::OnProjectileBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	BounceCount++;
	LastBounceVelocity = ImpactVelocity;

	if (ImpactResult.HasValidHitObjectHandle())
	{
		HandleActorHit(ImpactResult);
	}
	
	if (BounceCount == MaxBounces)
	{
		// We can't hit anything else.
		HandleProjectileExhausted();
	}
	else if (bUsesTargetingSystem)
	{
		// Find a new target to launch towards.
		ExecuteTargeting();
	}
	else
	{
		// Keep bouncing aimlessly.
		LaunchForward();
	}
}

void ANinjaCombatProjectileActor::OnProjectileStop(const FHitResult& ImpactResult)
{
	if (ImpactResult.HasValidHitObjectHandle())
	{
		HandleActorHit(ImpactResult);
		HandleProjectileExhausted();
	}
}

void ANinjaCombatProjectileActor::HandleActorHit_Implementation(const FHitResult& ImpactResult)
{
	if (ImpactResult.HasValidHitObjectHandle() && ImpactEffectHandle.IsValid())
	{
		AActor* ActorHit = ImpactResult.GetActor();
		if (CurrentTarget == ActorHit)
		{
			ActorsHit.AddUnique(ActorHit);
			SphereComponent->MoveIgnoreActors.Add(ActorHit);

			const FString Message = FString::Printf(TEXT("Projectile %s hit %s."), *GetNameSafe(this), *GetNameSafe(ActorHit));
			PrintMessage(Message);			
		}

		if (HasAuthority())
		{
			UAbilitySystemComponent* TargetAbilityComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(ActorHit);
			if (IsValid(TargetAbilityComponent))
			{
				FHitResult AdjustedResult(ForceInit);
				AdjustHitResult(ImpactResult, AdjustedResult);
			
				if (AdjustedResult.IsValidBlockingHit())
				{
					ImpactEffectHandle.Data.Get()->GetContext().AddHitResult(AdjustedResult, true);
				}
			
				TargetAbilityComponent->BP_ApplyGameplayEffectSpecToSelf(ImpactEffectHandle);	
			}
		}		
	}

	if (!UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		Execute_HandleImpactCosmetics(this, ImpactResult);	
	}
}

void ANinjaCombatProjectileActor::AdjustHitResult_Implementation(const FHitResult& ImpactResult, FHitResult& AdjustedResult)
{
	if (ImpactResult.IsValidBlockingHit() && ImpactResult.GetComponent()->IsA(UMeshComponent::StaticClass()))
	{
		// This impact already contains a valid Mesh Component. Use it as-is.
		AdjustedResult = ImpactResult;
		DrawNewTarget(ImpactResult, FColor::Cyan);
		return;
	}

	const AActor* Target = ImpactResult.GetActor();

	FCollisionQueryParams TraceParams = FCollisionQueryParams(SCENE_QUERY_STAT(AdjustProjectileHit));
	TraceParams.TraceTag = TEXT("ProjectileHitTrace");
	TraceParams.bReturnPhysicalMaterial = true;
	TraceParams.bTraceComplex = true;
	TraceParams.AddIgnoredActor(this);
	TraceParams.AddIgnoredActor(GetInstigator());
	TraceParams.AddIgnoredComponent(ImpactResult.GetComponent());
	
	const FVector Start = ImpactResult.TraceStart;
	const FRotator Direction = UKismetMathLibrary::FindLookAtRotation(Start, Target->GetActorLocation());
	const FVector End = Start + UKismetMathLibrary::GetForwardVector(Direction) * 500.f;

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel, TraceParams))
	{
		AdjustedResult = Hit;
		DrawNewTarget(Hit, FColor::Green);
	}
	else
	{
		AdjustedResult = ImpactResult;
		DrawNewTarget(Hit, FColor::Yellow);
	}

	DrawScanLine(Hit);
}

void ANinjaCombatProjectileActor::HandleProjectileExhausted_Implementation()
{
	ProjectileMovement->StopMovementImmediately();
	Destroy();
}

void ANinjaCombatProjectileActor::HandleImpactCosmetics_Implementation(const FHitResult& HitResult) const
{
	if (HitResult.HasValidHitObjectHandle())
	{
		const FVector ImpactLocation = HitResult.ImpactPoint;
		const FRotator ImpactRotation = FRotationMatrix::MakeFromZ(HitResult.Normal).Rotator();

		TArray<FSoftObjectPath> EffectPaths;

		if (ImpactFX.Get() == nullptr && !ImpactFX.IsNull())
		{
			EffectPaths.Emplace(ImpactFX.ToSoftObjectPath());
		}

		if (ImpactSound.Get() == nullptr && !ImpactSound.IsNull())
		{
			EffectPaths.Emplace(ImpactSound.ToSoftObjectPath());
		}

		if (EffectPaths.IsEmpty())
		{
			PlayNiagaraSystem(GetWorld(), ImpactFX.Get(), ImpactLocation, ImpactRotation);
			PlaySound(GetWorld(), ImpactSound.Get(), ImpactLocation, ImpactRotation);
		}
		else
		{
			UAssetManager::GetStreamableManager().RequestAsyncLoad(EffectPaths, [this, ImpactLocation, ImpactRotation]()
			{
				PlayNiagaraSystem(GetWorld(), ImpactFX.Get(), ImpactLocation, ImpactRotation);
				PlaySound(GetWorld(), ImpactSound.Get(), ImpactLocation, ImpactRotation);
			});
		}
	}
}

void ANinjaCombatProjectileActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// If this projectile was destroyed in the world, then play the destruction cosmetics.
	// Destroyed in the world: Too many Bounces, lower speed than allowed.
	//
	if (!UKismetSystemLibrary::IsDedicatedServer(GetWorld()) && EndPlayReason == EEndPlayReason::Destroyed)
	{
		Execute_HandleDestructionCosmetics(this);	
	}

	GetInstigator()->MoveIgnoreActorRemove(this);
	Super::EndPlay(EndPlayReason);
}

void ANinjaCombatProjectileActor::HandleDestructionCosmetics_Implementation() const
{
	TArray<FSoftObjectPath> EffectPaths;
	
	if (DestructionFX.Get() == nullptr && !DestructionFX.IsNull())
	{
		EffectPaths.Emplace(DestructionFX.ToSoftObjectPath());
	}

	if (EffectPaths.IsEmpty())
	{
		PlayNiagaraSystem(GetWorld(), DestructionFX.Get(), GetActorLocation(), GetActorRotation());
	}
	else
	{
		UAssetManager::GetStreamableManager().RequestAsyncLoad(EffectPaths, [this]()
			{ PlayNiagaraSystem(GetWorld(), DestructionFX.Get(), GetActorLocation(), GetActorRotation()); });
	}
}

TSubclassOf<UGameplayEffect> ANinjaCombatProjectileActor::GetImpactEffectClass_Implementation() const
{
	return ImpactEffectClass;
}

void ANinjaCombatProjectileActor::SetImpactEffectHandle_Implementation(FGameplayEffectSpecHandle& Handle)
{
	ImpactEffectHandle = Handle;
}

TArray<AActor*> ANinjaCombatProjectileActor::GetActorsHit_Implementation() const
{
	TArray<AActor*> Result;
	Result.Reserve(ActorsHit.Num());
	
	for (const TWeakObjectPtr<AActor>& ActorPtr : ActorsHit)
	{
		if (ActorPtr.IsValid() && ActorPtr->IsValidLowLevelFast())
		{
			Result.Add(ActorPtr.Get());
		}
	}

	return Result;
}

float ANinjaCombatProjectileActor::GetProjectileImpactStrength_Implementation() const
{
	return ImpactStrength;
}

void ANinjaCombatProjectileActor::PlayNiagaraSystem(const UWorld* World, UNiagaraSystem* System,
	const FVector& Location, const FRotator& Rotation)
{
	if (IsValid(World) && !World->bIsTearingDown && IsValid(System))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, System, Location, Rotation, FVector::One(),
			true, true, ENCPoolMethod::AutoRelease, true);		
	}
}

void ANinjaCombatProjectileActor::PlaySound(const UWorld* World, USoundBase* Sound,
	const FVector& Location, const FRotator& Rotation)
{
	if (IsValid(World) && !World->bIsTearingDown && IsValid(Sound))
	{
		UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, Rotation);
	}
}

void ANinjaCombatProjectileActor::PrintMessage(const FString& Message) const
{
#if WITH_EDITOR
	if (bEnableDebug && IsValid(GEngine))
	{
		GEngine->AddOnScreenDebugMessage(INDEX_NONE, 3.f, FColor::Emerald, Message);
	}
#endif	
}

void ANinjaCombatProjectileActor::DrawScanLine(const FHitResult& NewHit) const
{
#if WITH_EDITOR
	if (bEnableDebug)
	{
		const FColor Color = NewHit.IsValidBlockingHit() ? FColor::Green : FColor::Red; 
		::DrawDebugLine(GetWorld(), NewHit.TraceStart, NewHit.TraceEnd, Color, false, 2.f);
	}
#endif
}

void ANinjaCombatProjectileActor::DrawNewTarget(const FHitResult& NewHit, const FColor& Color) const
{
#if WITH_EDITOR
	if (bEnableDebug)
	{
		const FVector StartPosition = NewHit.Location;
		DrawDebugSphere(GetWorld(), StartPosition, 24.f, 12.f, Color, false, 2.f);
	}
#endif	
}
