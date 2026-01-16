// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameUIManagerSubsystem.h"

#include "Engine/GameInstance.h"
#include "GameUIPolicy.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUIManagerSubsystem)

class FSubsystemCollectionBase;
class UClass;

/**
 * 初始化UI管理子系统
 * 
 * 主要职责：
 * 1. 加载配置的默认UI策略类
 * 2. 创建初始UI策略实例
 * 3. 建立UI策略与子系统的关联
 * 
 * @param Collection 子系统依赖集合
 */
void UGameUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// 当无当前策略且配置了默认策略类时初始化默认策略
	if (!CurrentPolicy && !DefaultUIPolicyClass.IsNull())
	{
		// 同步加载配置的策略类
		TSubclassOf<UGameUIPolicy> PolicyClass = DefaultUIPolicyClass.LoadSynchronous();
		
		// 创建策略实例并切换策略
		SwitchToPolicy(NewObject<UGameUIPolicy>(this, PolicyClass));
	}
}

/**
 * 反初始化子系统
 * 
 * 清理顺序：
 * 1. 清除当前UI策略
 * 2. 断开所有玩家关联
 * 3. 释放子系统资源
 */
void UGameUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	// 清空当前策略（会触发策略清理）
	SwitchToPolicy(nullptr);

}

/**
 * 判断是否应创建该子系统
 * 
 * 创建规则：
 * 1. 非专用服务器才需要创建
 * 2. 仅当没有派生类覆盖时创建基类实例
 * 
 * @param Outer 外部对象（应指向GameInstance）
 * @return 是否应创建此子系统实例
 */
bool UGameUIManagerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// 跳过专用服务器实例
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// 仅当不存在派生实现时创建基类
		return ChildClasses.Num() == 0;
	}

	return false;
}

/**
 * 处理新玩家加入事件
 * 
 * 职责链：
 * 1. 验证玩家有效性
 * 2. 将事件转发给当前策略
 * 
 * @param LocalPlayer 加入的本地玩家实例
 */
void UGameUIManagerSubsystem::NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer)
{
	// 确保玩家有效且策略存在
	if (ensure(LocalPlayer) && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerAdded(LocalPlayer);
	}
}

/**
 * 处理玩家移除事件
 * 
 * @param LocalPlayer 被移除的本地玩家
 */
void UGameUIManagerSubsystem::NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerRemoved(LocalPlayer);
	}
}

/**
 * 处理玩家销毁事件
 * 
 * @param LocalPlayer 即将销毁的玩家实例
 */
void UGameUIManagerSubsystem::NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer)
{
	if (LocalPlayer && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerDestroyed(LocalPlayer);
	}
}

/**
 * 执行UI策略切换
 * 
 * 切换流程：
 * 1. 验证新旧策略不同
 * 2. 清除旧策略（触发其清理流程）
 * 3. 设置新策略并初始化
 * 
 * @param InPolicy 新策略实例
 */
void UGameUIManagerSubsystem::SwitchToPolicy(UGameUIPolicy* InPolicy)
{
	// 避免重复设置相同策略
	if (CurrentPolicy != InPolicy)
	{
		// 释放现有策略（策略对象会收到Deinitialize事件）
		CurrentPolicy = InPolicy;
	}
}