// Copyright Epic Games, Inc. All Rights Reserved.

#include "PrimaryGameLayout.h"
#include "CommonLocalPlayer.h"          // 本地玩家扩展功能
#include "Engine/GameInstance.h"        // 游戏实例基类
#include "GameUIManagerSubsystem.h"     // UI管理子系统
#include "GameUIPolicy.h"               // UI策略接口
#include "Kismet/GameplayStatics.h"     // 游戏工具函数
#include "LogCommonGame.h"              // 日志模块
#include "Widgets/CommonActivatableWidgetContainer.h" // 可激活控件容器

#include UE_INLINE_GENERATED_CPP_BY_NAME(PrimaryGameLayout)  // 自动生成的反射代码

class UObject;

/**
 * 获取主玩家的根布局实例
 * 
 * 实现路径：
 * 1. 通过游戏实例获取主玩家控制器
 * 2. 调用重载方法获取关联的根布局
 * 
 * @param WorldContextObject 世界上下文对象
 * @return 主玩家的根布局实例（不存在时返回nullptr）
 */
UPrimaryGameLayout* UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
	return GetPrimaryGameLayout(PlayerController);
}

/**
 * 通过玩家控制器获取关联的根布局
 * 
 * 核心逻辑：
 * 1. 从玩家控制器获取本地玩家实例
 * 2. 调用本地玩家版获取方法
 * 
 * @param PlayerController 玩家控制器实例
 * @return 关联的根布局（不存在时返回nullptr）
 */
UPrimaryGameLayout* UPrimaryGameLayout::GetPrimaryGameLayout(APlayerController* PlayerController)
{
	return PlayerController ? GetPrimaryGameLayout(Cast<UCommonLocalPlayer>(PlayerController->Player)) : nullptr;
}

/**
 * 通过本地玩家获取关联的根布局
 * 
 * 实现路径：
 * 1. 验证本地玩家有效性
 * 2. 通过游戏实例获取UI管理子系统
 * 3. 获取当前UI策略
 * 4. 从策略中获取关联此玩家的根布局
 * 
 * @param LocalPlayer 本地玩家实例
 * @return 关联的根布局（不存在时返回nullptr）
 */
UPrimaryGameLayout* UPrimaryGameLayout::GetPrimaryGameLayout(ULocalPlayer* LocalPlayer)
{
	if (LocalPlayer)
	{
		const UCommonLocalPlayer* CommonLocalPlayer = CastChecked<UCommonLocalPlayer>(LocalPlayer);
		if (const UGameInstance* GameInstance = CommonLocalPlayer->GetGameInstance())
		{
			// 获取UI管理子系统（管理UI策略）
			if (UGameUIManagerSubsystem* UIManager = GameInstance->GetSubsystem<UGameUIManagerSubsystem>())
			{
				// 获取当前生效的UI策略
				if (const UGameUIPolicy* Policy = UIManager->GetCurrentUIPolicy())
				{
					// 获取此玩家的根布局实例
					if (UPrimaryGameLayout* RootLayout = Policy->GetRootLayout(CommonLocalPlayer))
					{
						return RootLayout;
					}
				}
			}
		}
	}
	return nullptr;
}

/** 构造函数 */
UPrimaryGameLayout::UPrimaryGameLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

/**
 * 设置布局休眠状态
 * 
 * 功能说明：
 * - 休眠状态用于暂停非活动玩家的UI渲染（如分屏场景）
 * - 状态变更时触发日志记录和回调事件
 * 
 * @param InDormant 是否进入休眠
 */
void UPrimaryGameLayout::SetIsDormant(bool InDormant)
{
	if (bIsDormant != InDormant)  // 状态变化检测
	{
		// 获取玩家信息用于日志
		const ULocalPlayer* LP = GetOwningLocalPlayer();
		const int32 PlayerId = LP ? LP->GetControllerId() : -1;
		const TCHAR* OldDormancyStr = bIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* NewDormancyStr = InDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* PrimaryPlayerStr = LP && LP->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
		
		// 记录状态变更日志
		UE_LOG(LogCommonGame, Display, TEXT("%s PrimaryGameLayout Dormancy changed for [%d] from [%s] to [%s]"), 
			PrimaryPlayerStr, PlayerId, OldDormancyStr, NewDormancyStr);

		bIsDormant = InDormant;
		OnIsDormantChanged();  // 触发状态变更回调
	}
}

/**
 * 休眠状态变更回调
 * 
 * 设计说明：
 * - 当前实现主要为占位接口（@TODO标记）
 * - 设计意图：休眠时完全停用UI渲染和输入
 * - 典型应用场景：分屏模式中非活动玩家的UI节省资源
 */
void UPrimaryGameLayout::OnIsDormantChanged()
{
	//@TODO NDarnell 确定休眠具体行为（历史方案）：
	// - 停用玩家视图渲染
	// - 折叠UI布局节省资源
	/*
	if (UCommonLocalPlayer* LocalPlayer = GetOwningLocalPlayer<UCommonLocalPlayer>())
	{
		// 休眠时停用玩家视图渲染
		LocalPlayer->SetIsPlayerViewEnabled(!bIsDormant);
	}
	SetVisibility(bIsDormant ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	OnLayoutDormancyChanged().Broadcast(bIsDormant);
	*/
}

/**
 * 注册UI层级
 * 
 * Lyra四层UI结构管理：
 * 1. Game层：实时游戏UI（如血条、弹药）[2](@ref)
 * 2. GameMenu层：局内菜单（如暂停菜单）
 * 3. Menu层：主菜单系统
 * 4. Modal层：模态弹窗（如确认对话框）[3](@ref)
 * 
 * @param LayerTag 层级标识（需使用Lyra预定义GameplayTag）
 * @param LayerWidget 层级容器控件
 */
void UPrimaryGameLayout::RegisterLayer(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerWidget)
{
	if (!IsDesignTime())  // 跳过设计时逻辑
	{
		// 绑定层级过渡事件（用于输入管理）
		LayerWidget->OnTransitioningChanged.AddUObject(this, &UPrimaryGameLayout::OnWidgetStackTransitioning);
		
		// 禁用过渡动画（确保游戏手柄焦点正确转移）
		LayerWidget->SetTransitionDuration(0.0);

		// 注册到层级字典
		Layers.Add(LayerTag, LayerWidget);
	}
}

/**
 * 控件栈过渡事件处理
 * 
 * 输入管理机制：
 * 1. 过渡开始时暂停输入（防止操作干扰）
 * 2. 过渡结束时恢复输入
 * 3. 使用令牌系统管理多个并发过渡
 * 
 * @param Widget 发生过渡的控件栈
 * @param bIsTransitioning 是否正在过渡
 */
void UPrimaryGameLayout::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	if (bIsTransitioning)
	{
		// 暂停玩家输入（返回暂停令牌）
		const FName SuspendToken = UCommonUIExtensions::SuspendInputForPlayer(
			GetOwningLocalPlayer(), 
			TEXT("GlobalStackTransion")
		);
		SuspendInputTokens.Add(SuspendToken);
	}
	else if (ensure(SuspendInputTokens.Num() > 0))
	{
		// 恢复玩家输入（使用最后记录的令牌）
		const FName SuspendToken = SuspendInputTokens.Pop();
		UCommonUIExtensions::ResumeInputForPlayer(GetOwningLocalPlayer(), SuspendToken);
	}
}

/**
 * 从任意层级移除控件
 * 
 * 典型场景：
 * - 动态关闭未知层级的弹窗
 * - 清理残留控件（如场景切换时）
 * 
 * @param ActivatableWidget 需移除的可激活控件
 */
void UPrimaryGameLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	// 遍历所有注册层级
	for (const auto& LayerKVP : Layers)
	{
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
	}
}

/**
 * 获取指定层级的容器控件
 * 
 * @param LayerName 层级标识（GameplayTag）
 * @return 层级容器控件（不存在时返回nullptr）
 */
UCommonActivatableWidgetContainerBase* UPrimaryGameLayout::GetLayerWidget(FGameplayTag LayerName)
{
	return Layers.FindRef(LayerName);
}