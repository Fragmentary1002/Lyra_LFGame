#include "LFAnimNotifyStateBase.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"

ULFAnimNotifyStateBase::ULFAnimNotifyStateBase()
{
	// 设置默认值

#if WITH_EDITORONLY_DATA
	// Ninja Bear Green! :D
	NotifyColor = FColor(211, 221, 197);
#endif
}

bool ULFAnimNotifyStateBase::AddGameplayTags(const FGameplayTag& GameplayTag, bool bIsActivate)
{
	// 检查标签是否有效
	if (!GameplayTag.IsValid())
	{
		return false;
	}

	// 尝试获取能力系统组件
	IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(GetOuter());
	if (!AbilitySystemInterface)
	{
		return false;
	}

	UAbilitySystemComponent* ASC = AbilitySystemInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return false;
	}

	// 添加或移除宽松游戏性标签
	if (bIsActivate)
	{
		ASC->AddLooseGameplayTag(GameplayTag);
	}
	else
	{
		ASC->RemoveLooseGameplayTag(GameplayTag);
	}

	return true;
}
