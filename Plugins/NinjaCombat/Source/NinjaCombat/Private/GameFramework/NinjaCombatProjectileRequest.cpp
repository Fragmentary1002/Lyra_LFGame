// Ninja Bear Studio Inc., all rights reserved.
#include "GameFramework/NinjaCombatProjectileRequest.h"

#include "Interfaces/CombatProjectileInterface.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogNinjaCombatProjectileRequest);

UNinjaCombatProjectileRequest::UNinjaCombatProjectileRequest()
{
	bHasTargetLocation = false;
	TargetLocation = FVector::ZeroVector;
	SocketName = TEXT("sProjectile");
}

UNinjaCombatProjectileRequest* UNinjaCombatProjectileRequest::NewInstance(const TSubclassOf<UNinjaCombatProjectileRequest> RequestClass,
	AActor* Owner, AActor* Instigator, UMeshComponent* SourceMesh, const FName SocketName, const TSubclassOf<AActor> ProjectileClass)
{
	UNinjaCombatProjectileRequest* Request = NewObject<UNinjaCombatProjectileRequest>(Owner, RequestClass);
	Request->RequestOwner = Owner;
	Request->Instigator = Cast<APawn>(Instigator);
	Request->SourceMesh = SourceMesh;
	Request->SocketName = SocketName;
	Request->ProjectileClass = ProjectileClass;
	return Request;
}

FVector UNinjaCombatProjectileRequest::GetSourceLocation() const
{
	if (IsValid(SourceMesh) && SourceMesh->DoesSocketExist(SocketName))
	{
		return SourceMesh->GetSocketLocation(SocketName);
	}
	
	return RequestOwner->GetActorLocation();
}

FRotator UNinjaCombatProjectileRequest::GetSourceRotation() const
{
	if (bHasTargetLocation)
	{
		const FVector SourceLocation = GetSourceLocation();
		return UKismetMathLibrary::FindLookAtRotation(SourceLocation, TargetLocation);	
	}
	
	return RequestOwner->GetActorRotation();
}

void UNinjaCombatProjectileRequest::SetInstigator(APawn* NewInstigator)
{
	Instigator = NewInstigator;
}

void UNinjaCombatProjectileRequest::SetTargetLocation(const FVector& NewTargetLocation)
{
	bHasTargetLocation = true;
	TargetLocation = NewTargetLocation;
}

void UNinjaCombatProjectileRequest::SetProjectileClassFallback(const TSubclassOf<AActor> ProjectileClassFallback)
{
	if (ProjectileClass == nullptr)
	{
		ProjectileClass = ProjectileClassFallback;
	}
}

AActor* UNinjaCombatProjectileRequest::SpawnProjectile() const
{
	checkf(IsValid(ProjectileClass), TEXT("Invalid projectile class provided to the request."));
	checkf(ProjectileClass->ImplementsInterface(UCombatProjectileInterface::StaticClass()), TEXT("Projectile class must implement the projectile interface."));
	
	UWorld* World = RequestOwner->GetWorld();

	FTransform SpawnTransform = FTransform::Identity;
	SpawnTransform.SetLocation(GetSourceLocation());
	SpawnTransform.SetRotation(FQuat::MakeFromRotator(GetSourceRotation()));
	
	AActor* Projectile = World->SpawnActorDeferred<AActor>(ProjectileClass, SpawnTransform, RequestOwner, Instigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (IsValid(Projectile))
	{
		ModifyProjectile(Projectile);
		Projectile->FinishSpawning(SpawnTransform);
	}

	return Projectile;
}

void UNinjaCombatProjectileRequest::ModifyProjectile_Implementation(AActor* Projectile) const
{
}

bool UNinjaCombatProjectileRequest::HasValidRequestData() const
{
	if (RequestOwner == nullptr)
	{
		UE_LOG(LogNinjaCombatProjectileRequest, Warning, TEXT("Projectile Request requires a valid Owner."));
		return false;
	}

	if (Instigator == nullptr)
	{
		UE_LOG(LogNinjaCombatProjectileRequest, Warning, TEXT("Projectile Request requires a valid Instigator."));
		return false;
	}
	
	if (SourceMesh == nullptr || !SourceMesh->DoesSocketExist(SocketName))
	{
		UE_LOG(LogNinjaCombatProjectileRequest, Warning, TEXT("Projectile Request requires a valid Mesh Component with the provided socket."));
		return false;
	}

	if (ProjectileClass == nullptr)
	{
		UE_LOG(LogNinjaCombatProjectileRequest, Warning, TEXT("Projectile Request requires a valid Projectile Class."));
		return false;
	}
	
	return true;
}
