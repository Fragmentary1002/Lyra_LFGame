// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraUIManagerSubsystem.h"
#include "CommonLocalPlayer.h"          // 本地玩家扩展功能
#include "Engine/GameInstance.h"        // 游戏实例基类
#include "GameFramework/HUD.h"          // HUD控制
#include "GameUIPolicy.h"               // UI策略接口
#include "PrimaryGameLayout.h"          // 根布局管理器

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraUIManagerSubsystem)  // 自动生成的反射代码

class FSubsystemCollectionBase;

/** 构造函数 */
ULyraUIManagerSubsystem::ULyraUIManagerSubsystem()
{
}

/**
 * 初始化UI管理子系统
 * 
 * 核心职责：
 * 1. 注册Tick委托实现每帧更新
 * 2. 确保与父类初始化流程同步
 * 
 * @param Collection 子系统依赖集合（自动注入）[7](@ref)
 */
void ULyraUIManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);  // 父类初始化

	// 注册Tick委托（CoreTicker确保帧同步）
	TickHandle = FTSTicker::GetCoreTicker().AddTicker(
		FTickerDelegate::CreateUObject(this, &ULyraUIManagerSubsystem::Tick),
		0.0f  // 立即执行
	);
}

/**
 * 反初始化子系统
 * 
 * 清理顺序：
 * 1. 移除Tick委托避免无效调用
 * 2. 执行父类资源释放
 */
void ULyraUIManagerSubsystem::Deinitialize()
{
	Super::Deinitialize();  // 父类清理
	// 移除已注册的Tick委托
	FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
	
}

/**
 * 每帧更新逻辑
 * 
 * 功能：
 * - 同步根布局与HUD的可见性状态
 * - 需保持轻量级以避免性能问题
 * 
 * @param DeltaTime 帧间隔时间
 * @return 始终返回true保持持续触发[5](@ref)
 */
bool ULyraUIManagerSubsystem::Tick(float DeltaTime)
{
	SyncRootLayoutVisibilityToShowHUD();  // 核心同步逻辑
	return true;  // 保持持续触发
}

/**
 * 同步根布局与HUD的可见性状态
 * 
 * 设计原理：
 * 1. 当HUD隐藏时（如过场动画），强制折叠UI布局
 * 2. UI策略（GameUIPolicy）管理不同玩家的根布局实例
 * 3. 可见性状态映射：
 *    - HUD显示 → UI布局可见（SelfHitTestInvisible 可交互）
 *    - HUD隐藏 → UI布局折叠（Collapsed 不可见且不占布局空间）
 * 
 * 实现流程：
 * 1. 获取当前UI策略
 * 2. 遍历所有本地玩家
 * 3. 检测各玩家控制器的HUD显示状态
 * 4. 动态调整PrimaryGameLayout的Slate可见性[2,5](@ref)
 */
void ULyraUIManagerSubsystem::SyncRootLayoutVisibilityToShowHUD()
{
	// 获取当前生效的UI策略（如分层管理策略）
	if (const UGameUIPolicy* Policy = GetCurrentUIPolicy())
	{
		// 遍历游戏实例中的所有本地玩家
		for (const ULocalPlayer* LocalPlayer : GetGameInstance()->GetLocalPlayers())
		{
			bool bShouldShowUI = true;  // 默认显示UI

			// 获取玩家控制器及其HUD状态
			if (const APlayerController* PC = LocalPlayer->GetPlayerController(GetWorld()))
			{
				if (const AHUD* HUD = PC->GetHUD())
				{
					// HUD显隐状态决定UI显隐
					bShouldShowUI = HUD->bShowHUD;
				}
			}

			// 获取当前玩家的根布局（PrimaryGameLayout）
			if (UPrimaryGameLayout* RootLayout = Policy->GetRootLayout(CastChecked<UCommonLocalPlayer>(LocalPlayer)))
			{
				// 计算目标可见性状态
				const ESlateVisibility DesiredVisibility = 
					bShouldShowUI ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed;

				// 状态变化时更新（避免冗余操作）
				if (DesiredVisibility != RootLayout->GetVisibility())
				{
					RootLayout->SetVisibility(DesiredVisibility);
				}
			}
		}
	}
}