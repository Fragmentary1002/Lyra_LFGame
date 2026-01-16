// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameUIPolicy.h"
#include "Engine/GameInstance.h"          // 游戏实例基类
#include "Framework/Application/SlateApplication.h" // Slate应用框架
#include "GameUIManagerSubsystem.h"      // UI管理子系统
#include "CommonLocalPlayer.h"            // 本地玩家扩展功能
#include "PrimaryGameLayout.h"            // UI根布局管理器
#include "Engine/Engine.h"                // 引擎全局对象
#include "LogCommonGame.h"                // 日志模块

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameUIPolicy)  // 自动生成的反射代码

/**
 * 获取当前游戏场景的UI策略实例
 * 
 * 实现路径：
 * 1. 通过世界上下文获取游戏实例
 * 2. 通过游戏实例获取UI管理子系统
 * 3. 返回子系统当前激活的UI策略
 * 
 * @param WorldContextObject 世界上下文对象
 * @return 当前生效的UI策略实例（不存在时返回nullptr）
 */
UGameUIPolicy* UGameUIPolicy::GetGameUIPolicy(const UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			// 获取UI管理子系统（UGameUIManagerSubsystem）
			if (UGameUIManagerSubsystem* UIManager = UGameInstance::GetSubsystem<UGameUIManagerSubsystem>(GameInstance))
			{
				return UIManager->GetCurrentUIPolicy();  // 返回当前策略
			}
		}
	}
	return nullptr;
}

/** 获取所属的UI管理子系统（通过Outer链追溯） */
UGameUIManagerSubsystem* UGameUIPolicy::GetOwningUIManager() const
{
	return CastChecked<UGameUIManagerSubsystem>(GetOuter());
}

/** 获取关联的游戏世界 */
UWorld* UGameUIPolicy::GetWorld() const
{
	return GetOwningUIManager()->GetGameInstance()->GetWorld();
}

/**
 * 获取指定本地玩家的根布局
 * 
 * @param LocalPlayer 目标本地玩家
 * @return 关联的PrimaryGameLayout实例（不存在时返回nullptr）
 */
UPrimaryGameLayout* UGameUIPolicy::GetRootLayout(const UCommonLocalPlayer* LocalPlayer) const
{
	const FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->RootLayout : nullptr;
}

//----------------------------------------------------------------//
// 玩家生命周期管理
//----------------------------------------------------------------//

/**
 * 处理玩家加入事件
 * 
 * 核心逻辑：
 * 1. 监听玩家控制器设置完成事件（OnPlayerControllerSet）
 * 2. 若玩家已有布局：直接添加到视口
 * 3. 若玩家无布局：创建新布局并添加
 * 
 * @param LocalPlayer 加入的本地玩家
 */
void UGameUIPolicy::NotifyPlayerAdded(UCommonLocalPlayer* LocalPlayer)
{
	// 注册控制器设置完成回调（弱引用避免内存泄漏）
	LocalPlayer->OnPlayerControllerSet.AddWeakLambda(this, [this](UCommonLocalPlayer* LocalPlayer, APlayerController* PlayerController)
	{
		NotifyPlayerRemoved(LocalPlayer);  // 先清理可能存在的旧布局

		// 获取或创建布局
		if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
		{
			AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
			LayoutInfo->bAddedToViewport = true;
		}
		else
		{
			CreateLayoutWidget(LocalPlayer);  // 创建新布局
		}
	});

	// 立即处理当前状态
	if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		AddLayoutToViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = true;
	}
	else
	{
		CreateLayoutWidget(LocalPlayer);
	}
}

/**
 * 处理玩家移除事件
 * 
 * 关键机制：
 * 1. 从视口移除关联布局
 * 2. 分屏模式下处理布局控制权转移
 *    - SingleToggle模式：次玩家移除时激活主玩家布局
 * 
 * @param LocalPlayer 被移除的本地玩家
 */
void UGameUIPolicy::NotifyPlayerRemoved(UCommonLocalPlayer* LocalPlayer)
{
	if (FRootViewportLayoutInfo* LayoutInfo = RootViewportLayouts.FindByKey(LocalPlayer))
	{
		// 从视口移除布局
		RemoveLayoutFromViewport(LocalPlayer, LayoutInfo->RootLayout);
		LayoutInfo->bAddedToViewport = false;

		// 分屏控制权转移逻辑（仅限SingleToggle模式）
		if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && !LocalPlayer->IsPrimaryPlayer())
		{
			UPrimaryGameLayout* RootLayout = LayoutInfo->RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				// 次玩家布局控制时：停用当前布局，激活主玩家布局
				RootLayout->SetIsDormant(true);
				for (const FRootViewportLayoutInfo& RootLayoutInfo : RootViewportLayouts)
				{
					if (RootLayoutInfo.LocalPlayer->IsPrimaryPlayer())
					{
						if (UPrimaryGameLayout* PrimaryRootLayout = RootLayoutInfo.RootLayout)
						{
							PrimaryRootLayout->SetIsDormant(false);  // 激活主玩家布局
						}
					}
				}
			}
		}
	}
}

/**
 * 处理玩家销毁事件
 * 
 * 清理流程：
 * 1. 移除玩家关联的布局
 * 2. 解绑事件委托
 * 3. 释放布局资源
 * 
 * @param LocalPlayer 销毁中的本地玩家
 */
void UGameUIPolicy::NotifyPlayerDestroyed(UCommonLocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
	LocalPlayer->OnPlayerControllerSet.RemoveAll(this);  // 解绑所有委托
	
	// 从布局列表移除
	const int32 LayoutInfoIdx = RootViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		UPrimaryGameLayout* Layout = RootViewportLayouts[LayoutInfoIdx].RootLayout;
		RootViewportLayouts.RemoveAt(LayoutInfoIdx);

		RemoveLayoutFromViewport(LocalPlayer, Layout);
		OnRootLayoutReleased(LocalPlayer, Layout);  // 触发释放回调
	}
}

//----------------------------------------------------------------//
// 布局视口管理
//----------------------------------------------------------------//

/**
 * 添加布局到视口
 * 
 * 操作流程：
 * 1. 设置布局的玩家上下文（FLocalPlayerContext）
 * 2. 添加到玩家屏幕（ZOrder=1000确保顶层显示）
 * 3. 触发添加完成回调
 * 
 * @param LocalPlayer 目标玩家
 * @param Layout 待添加的布局实例
 */
void UGameUIPolicy::AddLayoutToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
	UE_LOG(LogCommonGame, Log, TEXT("[%s]添加玩家[%s]的根布局[%s]到视口"), 
		*GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));  // 绑定玩家上下文
	Layout->AddToPlayerScreen(1000);  // ZOrder=1000（顶层UI）
	OnRootLayoutAddedToViewport(LocalPlayer, Layout);  // 回调扩展点
}

/**
 * 从视口移除布局
 * 
 * 安全机制：
 * 1. 检查Slate控件有效性
 * 2. 记录移除日志（含泄漏警告）
 * 3. 触发移除完成回调
 * 
 * @param LocalPlayer 目标玩家
 * @param Layout 待移除的布局实例
 */
void UGameUIPolicy::RemoveLayoutFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
	TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogCommonGame, Log, TEXT("[%s]移除玩家[%s]的根布局[%s]"), 
			*GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();  // 从父级移除
		
		// 泄漏检测（Slate控件仍被引用）
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogCommonGame, Log, TEXT("警告：玩家[%s]的布局[%s]已移除，但Slate控件仍被引用！"), 
				*GetNameSafe(LocalPlayer), *GetNameSafe(Layout));
		}

		OnRootLayoutRemovedFromViewport(LocalPlayer, Layout);  // 回调扩展点
	}
}

/** 布局添加到视口后的回调（编辑器模式下自动聚焦游戏视口） */
void UGameUIPolicy::OnRootLayoutAddedToViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
#if WITH_EDITOR
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
	{
		// PIE模式下自动聚焦游戏视口（避免手动点击）
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
#endif
}

/** 布局从视口移除后的回调（默认空实现） */
void UGameUIPolicy::OnRootLayoutRemovedFromViewport(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
}

/** 布局资源释放回调（默认空实现） */
void UGameUIPolicy::OnRootLayoutReleased(UCommonLocalPlayer* LocalPlayer, UPrimaryGameLayout* Layout)
{
}

//----------------------------------------------------------------//
// 布局控制权管理
//----------------------------------------------------------------//

/**
 * 请求布局控制权（分屏模式专用）
 * 
 * 适用场景：
 * - 玩家切换分屏焦点时（如主玩家切换到次玩家窗口）
 * 
 * @param Layout 请求控制权的布局实例
 */
void UGameUIPolicy::RequestPrimaryControl(UPrimaryGameLayout* Layout)
{
	// 仅SingleToggle模式生效（单焦点切换）
	if (LocalMultiplayerInteractionMode == ELocalMultiplayerInteractionMode::SingleToggle && Layout->IsDormant())
	{
		// 停用当前活跃布局
		for (const FRootViewportLayoutInfo& LayoutInfo : RootViewportLayouts)
		{
			UPrimaryGameLayout* RootLayout = LayoutInfo.RootLayout;
			if (RootLayout && !RootLayout->IsDormant())
			{
				RootLayout->SetIsDormant(true);  // 休眠当前布局
				break;
			}
		}
		Layout->SetIsDormant(false);  // 激活目标布局
	}
}

//----------------------------------------------------------------//
// 布局创建逻辑
//----------------------------------------------------------------//

/**
 * 创建玩家的根布局控件
 * 
 * 核心流程：
 * 1. 获取布局控件类（通过GetLayoutWidgetClass）
 * 2. 创建控件实例（绑定到玩家控制器）
 * 3. 注册到布局管理器并添加到视口
 * 
 * @param LocalPlayer 目标本地玩家
 */
void UGameUIPolicy::CreateLayoutWidget(UCommonLocalPlayer* LocalPlayer)
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		// 获取布局控件类（通常从配置加载）
		TSubclassOf<UPrimaryGameLayout> LayoutWidgetClass = GetLayoutWidgetClass(LocalPlayer);
		if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			// 创建布局实例（由玩家控制器Owner管理）
			UPrimaryGameLayout* NewLayoutObject = CreateWidget<UPrimaryGameLayout>(PlayerController, LayoutWidgetClass);
			
			// 注册到布局列表（记录本地玩家关联）
			RootViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);
			
			// 添加到视口
			AddLayoutToViewport(LocalPlayer, NewLayoutObject);
		}
	}
}

/**
 * 获取布局控件类（可被子类覆盖）
 * 
 * 默认实现：
 * - 返回LayoutClass配置的软引用（同步加载）
 * 
 * @param LocalPlayer 目标玩家（用于差异化布局）
 * @return 布局控件类
 */
TSubclassOf<UPrimaryGameLayout> UGameUIPolicy::GetLayoutWidgetClass(UCommonLocalPlayer* LocalPlayer)
{
	return LayoutClass.LoadSynchronous();  // 同步加载配置的布局类
}