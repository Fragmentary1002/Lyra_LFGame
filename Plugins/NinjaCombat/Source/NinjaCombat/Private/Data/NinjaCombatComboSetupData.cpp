// Ninja Bear Studio Inc., all rights reserved.
#include "Data/NinjaCombatComboSetupData.h"

FPrimaryAssetId UNinjaCombatComboSetupData::GetPrimaryAssetId() const
{
	static const FPrimaryAssetType BaseAssetType = TEXT("CombatComboSetup");
	return FPrimaryAssetId(BaseAssetType, GetFName());
}

FGameplayTag UNinjaCombatComboSetupData::GetComboEventTag(const UInputAction* Action) const
{
	if (ComboEvents.Contains(Action))
	{
		return *ComboEvents.Find(Action);
	}

	return FGameplayTag::EmptyTag;
}
