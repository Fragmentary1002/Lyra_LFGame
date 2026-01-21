// Ninja Bear Studio Inc., all rights reserved.
#include "Data/NinjaCombatDamageHandlerData.h"

FPrimaryAssetId UNinjaCombatDamageHandlerData::GetPrimaryAssetId() const
{
	static const FPrimaryAssetType BaseAssetType = TEXT("CombatDamageHandler");
	return FPrimaryAssetId(BaseAssetType, GetFName());
}
