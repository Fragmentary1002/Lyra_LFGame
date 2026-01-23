// Ninja Bear Studio Inc., all rights reserved.
#include "AbilitySystem/Abilities/CombatAbility_Combo.h"

#include "InputAction.h"
#include "NinjaCombatFunctionLibrary.h"
#include "NinjaCombatTags.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Effects/CombatEffect_ComboWindow.h"
#include "Data/NinjaCombatComboSetupData.h"
#include "Interfaces/Components/CombatComboManagerInterface.h"

// 构造函数
/**
 * 初始化连击能力
 * 设置连击窗口效果类
 */

UCombatAbility_Combo::UCombatAbility_Combo(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ComboWindowEffectClass = UCombatEffect_ComboWindow::StaticClass();
}

/**
 * 检查是否在连击窗口内
 * @return 是否在连击窗口内
 */
bool UCombatAbility_Combo::InComboWindow() const
{
	check(IsValid(ComboManager));
	return ICombatComboManagerInterface::Execute_InComboWindow(ComboManager);
}

/**
 * 获取当前连击计数
 * @return 当前连击计数
 */
int32 UCombatAbility_Combo::GetComboCounter() const
{
	check(IsValid(ComboManager));
	return ICombatComboManagerInterface::Execute_GetComboCount(ComboManager);
}

/**
 * 检查能力是否可以激活
 * @param Handle 能力规格句柄
 * @param ActorInfo 能力拥有者信息
 * @param SourceTags 源标签容器
 * @param TargetTags 目标标签容器
 * @param OptionalRelevantTags 可选相关标签容器
 * @return 是否可以激活能力
 */
bool UCombatAbility_Combo::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)
		&& ensureMsgf(IsValid(ComboData), TEXT("A Combo Data Asset is required to activate the ability."));
}

/**
 * 激活能力
 * @param Handle 能力规格句柄
 * @param ActorInfo 能力拥有者信息
 * @param ActivationInfo 能力激活信息
 * @param TriggerEventData 触发事件数据
 */
void UCombatAbility_Combo::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	ComboManager = GetComboManagerComponentFromActorInfo();
	if (IsValid(ComboManager))
	{
		static constexpr bool bExactOnly = false;
		ComboEventTask = InitializeEventTask(Tag_Combat_Event_Combo, bExactOnly);
		ComboEventTask->ReadyForActivation();

		FComboFinishedDelegate Delegate;
		Delegate.BindDynamic(this, &ThisClass::UCombatAbility_Combo::HandleComboFinished);
	
		ICombatComboManagerInterface::Execute_BindToComboFinishedDelegate(ComboManager, Delegate);
		ICombatComboManagerInterface::Execute_StartCombo(ComboManager, ComboData);
	}
	else
	{
		K2_CancelAbility();
	}
}

/**
 * 处理接收到的事件
 * @param Payload 事件数据
 */
void UCombatAbility_Combo::HandleEventReceived_Implementation(const FGameplayEventData Payload)
{
	Super::HandleEventReceived_Implementation(Payload);

	if (Payload.EventTag == Tag_Combat_Event_Combo_Attack && InComboWindow())
	{
		const UInputAction* Action = GetInputActionFromEvent(Payload);
		if (ensure(IsValid(Action)))
		{
			ICombatComboManagerInterface::Execute_AdvanceCombo(ComboManager, Action);
		}
	}
	else if (Payload.EventTag == Tag_Combat_Event_Combo_Begin)
	{
		GrantComboWindowEffect();
	}
	else if (Payload.EventTag == Tag_Combat_Event_Combo_End)
	{
		RevokeComboWindowEffect();
	}
}

/**
 * 处理连击完成事件
 * @param FinishedComboData 完成的连击数据
 * @param bSucceeded 连击是否成功
 */
void UCombatAbility_Combo::HandleComboFinished(const UNinjaCombatComboSetupData* FinishedComboData, const bool bSucceeded)
{
	if (ComboData == FinishedComboData)
	{
		constexpr bool bReplicateEndAbility = false;
		const bool bWasCancelled = !bSucceeded;
		
		EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(),
			bReplicateEndAbility, bWasCancelled);
	}
}

/**
 * 从事件数据中获取输入动作
 * @param Payload 事件数据
 * @return 输入动作对象
 */
const UInputAction* UCombatAbility_Combo::GetInputActionFromEvent_Implementation(
	const FGameplayEventData& Payload) const
{
	return Cast<UInputAction>(Payload.OptionalObject);
}

/**
 * 授予连击窗口效果
 */
void UCombatAbility_Combo::GrantComboWindowEffect()
{
	if (IsValid(ComboWindowEffectClass) && !InComboWindow())
	{
		const FGameplayEffectSpecHandle Handle = MakeOutgoingGameplayEffectSpec(ComboWindowEffectClass);
		ComboWindowEffectHandle = K2_ApplyGameplayEffectSpecToOwner(Handle);

		if (ComboWindowEffectHandle.IsValid())
		{
			if (bEnableDebug)
			{
				const FString& DebugMessage = FString::Printf(TEXT("Granted Combo Window Effect."));
				AddDebugMessage(DebugMessage);	
			}
		}
	}	
}

/**
 * 撤销连击窗口效果
 */
void UCombatAbility_Combo::RevokeComboWindowEffect()
{
	if (ComboWindowEffectHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(ComboWindowEffectHandle);
		ComboWindowEffectHandle.Invalidate();

		if (bEnableDebug)
		{
			const FString& DebugMessage = FString::Printf(TEXT("Revoked Combo Window Effect."));
			AddDebugMessage(DebugMessage);	
		}
	}	
}

/**
 * 结束能力
 * @param Handle 能力规格句柄
 * @param ActorInfo 能力拥有者信息
 * @param ActivationInfo 能力激活信息
 * @param bReplicateEndAbility 是否复制结束能力
 * @param bWasCancelled 能力是否被取消
 */
void UCombatAbility_Combo::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const bool bReplicateEndAbility, const bool bWasCancelled)
{
	ICombatComboManagerInterface::Execute_UnbindFromComboFinishedDelegate(ComboManager, this);
	
	FinishLatentTasks({ ComboEventTask });
	RevokeComboWindowEffect();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

