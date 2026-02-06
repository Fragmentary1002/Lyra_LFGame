// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "GameplayTagContainer.h"
#include "LFGameAbilityComboDispatcher.generated.h"

/**
 * 连击调度器能力类
 */
UCLASS()
class LFGAME_API ULFGameAbilityComboDispatcher : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:
	// 构造函数
	ULFGameAbilityComboDispatcher(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	// 检查是否在攻击状态
	bool IsAttacking(const UAbilitySystemComponent* ASC) const;

	// 检查是否可以连击
	bool CanCombo(const UAbilitySystemComponent* ASC) const;

	// 获取当前连招段数
	int32 GetCurrentComboPhase(const UAbilitySystemComponent* ASC) const;

	// 更新当前连招标签
	void UpdateCurrentComboTag(UAbilitySystemComponent* ASC, int32 NewPhase) const;

	// 激活对应连招能力
	void ActivateComboAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, int32 ComboPhase) const;

protected:
	// 攻击标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FGameplayTag Tag_Attacking;

	// 可以连击标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FGameplayTag Tag_CanCombo;

	// 当前连招标签前缀
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	FString CurrentComboTagPrefix;

	// 连招能力类列表
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<TSubclassOf<ULyraGameplayAbility>> ComboAbilities;

	// 连招事件标签列表
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
	TArray<FGameplayTag> ComboEventTags;

	// 连招重置时间（秒）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo", meta = (ClampMin = "0.1"))
	float ComboResetTime = 2.0f;

	// 清除Combo标签的定时器句柄
	FTimerHandle ComboResetTimerHandle;


};