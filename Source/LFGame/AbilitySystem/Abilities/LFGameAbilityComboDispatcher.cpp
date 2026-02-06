// Fill out your copyright notice in the Description page of Project Settings.

#include "LFGameAbilityComboDispatcher.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

ULFGameAbilityComboDispatcher::ULFGameAbilityComboDispatcher(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// 设置默认值
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	// 默认标签值
	Tag_Attacking = FGameplayTag::RequestGameplayTag(FName("LF_RPG.Window.Attacking"));
	Tag_CanCombo = FGameplayTag::RequestGameplayTag(FName("LF_RPG.Window.Combo"));
	CurrentComboTagPrefix = "LF_RPG.Combo.";
}

void ULFGameAbilityComboDispatcher::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 确保能力可以激活
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 获取能力系统组件
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 检查状态并处理连招
	if (!IsAttacking(ASC) && !CanCombo(ASC))
	{
		// 激活第一段连招
		ActivateComboAbility(Handle, ActorInfo, 1);
		
		// 添加攻击标签
		ASC->AddLooseGameplayTag(Tag_Attacking);
		
		// 启动连招重置定时器
		StartComboResetTimer(ActorInfo);
	}
	else if (CanCombo(ASC))
	{
		// 获取当前连招段数
		int32 CurrentPhase = GetCurrentComboPhase(ASC);
		
		// 计算下一段连招
		int32 NextPhase = CurrentPhase + 1;
		
		// 激活下一段连招
		ActivateComboAbility(Handle, ActorInfo, NextPhase);
		
		// 更新当前连招标签
		UpdateCurrentComboTag(ASC, NextPhase);
		
		// 重启连招重置定时器
		StartComboResetTimer(ActorInfo);
	}

	// 结束调度器能力
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}

void ULFGameAbilityComboDispatcher::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	                                    const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool ULFGameAbilityComboDispatcher::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	                                    const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	                                    const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 获取能力系统组件
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (!ASC)
	{
		return false;
	}

	// 检查是否可以激活（在攻击状态或可以连击状态）
	return !IsAttacking(ASC) || CanCombo(ASC);
}

bool ULFGameAbilityComboDispatcher::IsAttacking(const UAbilitySystemComponent* ASC) const
{
	return ASC && Tag_Attacking.IsValid() && ASC->HasMatchingGameplayTag(Tag_Attacking);
}

bool ULFGameAbilityComboDispatcher::CanCombo(const UAbilitySystemComponent* ASC) const
{
	return ASC && Tag_CanCombo.IsValid() && ASC->HasMatchingGameplayTag(Tag_CanCombo);
}

int32 ULFGameAbilityComboDispatcher::GetCurrentComboPhase(const UAbilitySystemComponent* ASC) const
{
	if (!ASC || CurrentComboTagPrefix.IsEmpty())
	{
		return 0;
	}

	// 获取所有标签
	FGameplayTagContainer AllTags;
	ASC->GetOwnedGameplayTags(AllTags);

	// 查找当前连招标签
	for (const FGameplayTag& Tag : AllTags)
	{
		FString TagName = Tag.GetTagName().ToString();
		if (TagName.StartsWith(CurrentComboTagPrefix))
		{
			// 提取数字部分
			FString NumberStr = TagName.RightChop(CurrentComboTagPrefix.Len());
			int32 Phase = FCString::Atoi(*NumberStr);
			if (Phase > 0)
			{
				return Phase;
			}
		}
	}

	return 0;
}

void ULFGameAbilityComboDispatcher::UpdateCurrentComboTag(UAbilitySystemComponent* ASC, int32 NewPhase) const
{
	if (!ASC || CurrentComboTagPrefix.IsEmpty())
	{
		return;
	}

	// 移除旧的连招标签
	FGameplayTagContainer AllTags;
	ASC->GetOwnedGameplayTags(AllTags);

	for (const FGameplayTag& Tag : AllTags)
	{
		FString TagName = Tag.GetTagName().ToString();
		if (TagName.StartsWith(CurrentComboTagPrefix))
		{
			ASC->RemoveLooseGameplayTag(Tag);
		}
	}

	// 添加新的连招标签
	FString NewTagStr = CurrentComboTagPrefix;
	NewTagStr.AppendChars(TEXT("X"), NewPhase);
	FGameplayTag NewTag = FGameplayTag::RequestGameplayTag(FName(*NewTagStr));
	if (NewTag.IsValid())
	{
		ASC->AddLooseGameplayTag(NewTag);
	}
}



void ULFGameAbilityComboDispatcher::ActivateComboAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, int32 ComboPhase) const
{
	// 确保连招段数有效
	if (ComboPhase < 1)
	{
		return;
	}

	// 检查是否有对应的连招能力
	if (ComboAbilities.IsValidIndex(ComboPhase - 1))
	{
		// 通过事件触发连招能力
		if (ComboEventTags.IsValidIndex(ComboPhase - 1))
		{
			FGameplayEventData EventData;
			EventData.EventMagnitude = ComboPhase;
			EventData.OptionalObject = ActorInfo->AvatarActor.Get();
			
			UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
			if (ASC)
			{
				ASC->HandleGameplayEvent(ComboEventTags[ComboPhase - 1], &EventData);
			}
		}
	}
	else
	{
		// 如果没有配置对应能力，使用默认命名方式触发
		FString AbilityName = CurrentComboTagPrefix;
		AbilityName.AppendChars(TEXT("X"), ComboPhase);
		UE_LOG(LogTemp, Warning, TEXT("Activating Combo Ability: %s"), *AbilityName);
		FGameplayTag AbilityTag = FGameplayTag::RequestGameplayTag(FName(*AbilityName));
	
		if (AbilityTag.IsValid())
		{
			FGameplayEventData EventData;
			EventData.EventMagnitude = ComboPhase;
			EventData.OptionalObject = ActorInfo->AvatarActor.Get();
			
			UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
			if (ASC)
			{
				ASC->HandleGameplayEvent(AbilityTag, &EventData);
			}
		}
	}
}