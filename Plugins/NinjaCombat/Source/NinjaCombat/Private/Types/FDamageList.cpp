// Ninja Bear Studio Inc., all rights reserved.
#include "Types/FDamageList.h"

#include "NinjaCombatSettings.h"
#include "Engine/World.h"
#include "Components/NinjaCombatDamageManagerComponent.h"

FDamageList::FDamageList(): DamageManager(nullptr)
{
	DefaultArraySize = GetDefault<UNinjaCombatSettings>()->DamageRegistrySize;
	Entries.Reserve(DefaultArraySize);
}

FDamageList::FDamageList(UActorComponent* DamageManager): DamageManager(DamageManager)
{
	check(DamageManager && DamageManager->Implements<UCombatDamageManagerInterface>());
	DefaultArraySize = GetDefault<UNinjaCombatSettings>()->DamageRegistrySize;
	Entries.Reserve(DefaultArraySize);
}

void FDamageList::RegisterDamageTaken(const FGameplayEffectSpec& EffectSpec)
{
	CheckArrayLimits();

	const float Timestamp = DamageManager->GetWorld()->GetTimeSeconds();
	FDamageEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Initialize(Timestamp, EffectSpec);

	ICombatDamageManagerInterface::Execute_HandleDamageReceived(DamageManager, NewEntry);
	NewEntry.bHandled = true;
    
	MarkItemDirty(NewEntry);	
}

TArray<FDamageEntry> FDamageList::GetEntries() const
{
	return Entries;
}

void FDamageList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (const int32 Idx : AddedIndices)
	{
		BroadcastDamageTaken(Entries[Idx]);
	}	
}

void FDamageList::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	for (const int32 Idx : ChangedIndices)
	{
		BroadcastDamageTaken(Entries[Idx]);
	}
}

void FDamageList::CheckArrayLimits()
{
	if (Entries.Num() == DefaultArraySize)
	{
		Entries.RemoveAtSwap(0);
		MarkArrayDirty();
	}
}

void FDamageList::BroadcastDamageTaken(FDamageEntry& DamageInfo) const
{
	if (!DamageInfo.bHandled)
	{
		ICombatDamageManagerInterface::Execute_HandleDamageReceived(DamageManager, DamageInfo);
		DamageInfo.bHandled = true;
	}	
}

bool FDamageList::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
{
	return FastArrayDeltaSerialize<FDamageEntry, FDamageList>
		(Entries, DeltaParams, *this);	
}