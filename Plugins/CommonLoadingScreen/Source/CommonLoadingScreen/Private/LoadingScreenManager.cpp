// Copyright Epic Games, Inc. All Rights Reserved.

#include "LoadingScreenManager.h"

// 引入多线程心跳检测机制，用于监控加载卡顿
#include "HAL/ThreadHeartBeat.h"

// 引擎核心模块
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/WorldSettings.h"
#include "Misc/CommandLine.h"
#include "Misc/ConfigCacheIni.h"

// 加载流程接口
#include "LoadingProcessInterface.h"

// Slate应用框架
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"

// 预加载屏幕模块
#include "PreLoadScreen.h"
#include "PreLoadScreenManager.h"

// 着色器缓存支持
#include "ShaderPipelineCache.h"
#include "CommonLoadingScreenSettings.h"

// 临时占位控件（应替换为自定义控件）
#include "Widgets/Images/SThrobber.h"
#include "Blueprint/UserWidget.h"

// 自动生成的C++代码
#include UE_INLINE_GENERATED_CPP_BY_NAME(LoadingScreenManager)

// 定义日志分类
DECLARE_LOG_CATEGORY_EXTERN(LogLoadingScreen, Log, All);
DEFINE_LOG_CATEGORY(LogLoadingScreen);


//@TODO: Why can GetLocalPlayers() have nullptr entries?  Can it really?
//@TODO: Test with PIE mode set to simulate and decide how much (if any) loading screen action should occur
//@TODO: Allow other things implementing ILoadingProcessInterface besides GameState/PlayerController (and owned components) to register as interested parties
//@TODO: ChangeMusicSettings (either here or using the LoadingScreenVisibilityChanged delegate)
//@TODO: Studio analytics (FireEvent_PIEFinishedLoading / tracking PIE startup time for regressions, either here or using the LoadingScreenVisibilityChanged delegate)

// Profiling category for loading screens

CSV_DEFINE_CATEGORY(LoadingScreen, true);
//////////////////////////////////////////////////////////////////////
// ILoadingProcessInterface 实现

bool ILoadingProcessInterface::ShouldShowLoadingScreen(UObject* TestObject, FString& OutReason)
{
    // 检查对象是否实现了加载接口
    if (TestObject != nullptr)
    {
        // 尝试转换为接口指针
        if (ILoadingProcessInterface* LoadObserver = Cast<ILoadingProcessInterface>(TestObject))
        {
            FString ObserverReason;
            // 调用接口方法判断是否需要显示加载屏幕
            if (LoadObserver->ShouldShowLoadingScreen(/*out*/ ObserverReason))
            {
                // 确保提供了有效原因
                if (ensureMsgf(!ObserverReason.IsEmpty(), TEXT("%s failed to set a reason why it wants to show the loading screen"), *GetPathNameSafe(TestObject)))
                {
                    OutReason = ObserverReason;
                }
                return true;
            }
        }
    }
    return false;
}

//////////////////////////////////////////////////////////////////////
// 控制台变量声明

namespace LoadingScreenCVars
{
    // 控制台变量：延长加载屏幕显示时间（秒）
    static float HoldLoadingScreenAdditionalSecs = 2.0f;
    static FAutoConsoleVariableRef CVarHoldLoadingScreenUpAtLeastThisLongInSecs(
        TEXT("CommonLoadingScreen.HoldLoadingScreenAdditionalSecs"),
        HoldLoadingScreenAdditionalSecs,
        TEXT("How long to hold the loading screen up after other loading finishes (in seconds) to try to give texture streaming a chance to avoid blurriness"),
        ECVF_Default | ECVF_Preview);

    // 控制台变量：每帧打印加载原因（调试用）
    static bool LogLoadingScreenReasonEveryFrame = false;
    static FAutoConsoleVariableRef CVarLogLoadingScreenReasonEveryFrame(
        TEXT("CommonLoadingScreen.LogLoadingScreenReasonEveryFrame"),
        LogLoadingScreenReasonEveryFrame,
        TEXT("When true, the reason the loading screen is shown or hidden will be printed to the log every frame."),
        ECVF_Default);

    // 控制台变量：强制显示加载屏幕
    static bool ForceLoadingScreenVisible = false;
    static FAutoConsoleVariableRef CVarForceLoadingScreenVisible(
        TEXT("CommonLoadingScreen.AlwaysShow"),
        ForceLoadingScreenVisible,
        TEXT("Force the loading screen to show."),
        ECVF_Default);
}

//////////////////////////////////////////////////////////////////////
// 输入预处理系统（屏蔽加载期间的输入）

class FLoadingScreenInputPreProcessor : public IInputProcessor
{
public:
    FLoadingScreenInputPreProcessor() { }
    virtual ~FLoadingScreenInputPreProcessor() { }

    // 判断是否需要屏蔽输入（编辑器模式下不屏蔽）
    bool CanEatInput() const
    {
        return !GIsEditor;
    }

    // 重写输入处理函数
    virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override { }
    virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanEatInput(); }
    virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override { return CanEatInput(); }
    virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent) override { return CanEatInput(); }
    virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
    virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
    virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
    virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override { return CanEatInput(); }
    virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override { return CanEatInput(); }
    virtual bool HandleMotionDetectedEvent(FSlateApplication& SlateApp, const FMotionEvent& MotionEvent) override { return CanEatInput(); }
};

//////////////////////////////////////////////////////////////////////
// ULoadingScreenManager 核心实现

void ULoadingScreenManager::Initialize(FSubsystemCollectionBase& Collection)
{
    // 注册地图加载/卸载的委托
    FCoreUObjectDelegates::PreLoadMapWithContext.AddUObject(this, &ThisClass::HandlePreLoadMap);
    FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &ThisClass::HandlePostLoadMap);

    // 获取游戏实例并验证
    const UGameInstance* LocalGameInstance = GetGameInstance();
    check(LocalGameInstance);
}

void ULoadingScreenManager::Deinitialize()
{
    // 清理资源
    StopBlockingInput();       // 停止输入屏蔽
    RemoveWidgetFromViewport(); // 移除加载控件

    // 注销委托
    FCoreUObjectDelegates::PreLoadMap.RemoveAll(this);
    FCoreUObjectDelegates::PostLoadMapWithWorld.RemoveAll(this);

    // 禁用后续Tick
    SetTickableTickType(ETickableTickType::Never);
}

bool ULoadingScreenManager::ShouldCreateSubsystem(UObject* Outer) const
{
    // 仅客户端需要加载屏幕（服务器不需要）
    const UGameInstance* GameInstance = CastChecked<UGameInstance>(Outer);
    const bool bIsServerWorld = GameInstance->IsDedicatedServerInstance();    
    return !bIsServerWorld;
}

void ULoadingScreenManager::Tick(float DeltaTime)
{
    // 每帧更新加载屏幕状态
    UpdateLoadingScreen();

    // 更新心跳日志计时器
    TimeUntilNextLogHeartbeatSeconds = FMath::Max(TimeUntilNextLogHeartbeatSeconds - DeltaTime, 0.0);
}

ETickableTickType ULoadingScreenManager::GetTickableTickType() const
{
    // 模板对象不需要Tick
    if (IsTemplate())
    {
        return ETickableTickType::Never;
    }
    return ETickableTickType::Conditional;
}

bool ULoadingScreenManager::IsTickable() const
{
    // 需要有效的游戏实例和视口客户端
    UGameInstance* GameInstance = GetGameInstance();
    return (GameInstance && GameInstance->GetGameViewportClient());
}

TStatId ULoadingScreenManager::GetStatId() const
{
    // 声明性能统计ID
    RETURN_QUICK_DECLARE_CYCLE_STAT(ULoadingScreenManager, STATGROUP_Tickables);
}

UWorld* ULoadingScreenManager::GetTickableGameObjectWorld() const
{
    // 获取关联的游戏世界
    return GetGameInstance()->GetWorld();
}

void ULoadingScreenManager::RegisterLoadingProcessor(TScriptInterface<ILoadingProcessInterface> Interface)
{
    // 注册外部加载处理器（扩展加载检测逻辑）
    ExternalLoadingProcessors.Add(Interface.GetObject());
}

void ULoadingScreenManager::UnregisterLoadingProcessor(TScriptInterface<ILoadingProcessInterface> Interface)
{
    // 注销加载处理器
    ExternalLoadingProcessors.Remove(Interface.GetObject());
}

void ULoadingScreenManager::HandlePreLoadMap(const FWorldContext& WorldContext, const FString& MapName)
{
    // 仅处理当前游戏实例的地图加载
    if (WorldContext.OwningGameInstance == GetGameInstance())
    {
        bCurrentlyInLoadMap = true; // 标记地图加载中

        // 引擎初始化后立即更新加载状态
        if (GEngine->IsInitialized())
        {
            UpdateLoadingScreen();
        }
    }
}

void ULoadingScreenManager::HandlePostLoadMap(UWorld* World)
{
    // 地图加载完成处理
    if ((World != nullptr) && (World->GetGameInstance() == GetGameInstance()))
    {
        bCurrentlyInLoadMap = false; // 清除加载标记
    }
}

void ULoadingScreenManager::UpdateLoadingScreen()
{
    // 决定是否打印加载状态日志
    bool bLogLoadingScreenStatus = LoadingScreenCVars::LogLoadingScreenReasonEveryFrame;

    if (ShouldShowLoadingScreen())
    {
        // 获取项目设置
        const UCommonLoadingScreenSettings* Settings = GetDefault<UCommonLoadingScreenSettings>();
        
        // 启动心跳检测（监控加载卡顿）
        FThreadHeartBeat::Get().MonitorCheckpointStart(GetFName(), Settings->LoadingScreenHeartbeatHangDuration);

        // 显示加载屏幕
        ShowLoadingScreen();

        // 按间隔打印心跳日志
        if ((Settings->LogLoadingScreenHeartbeatInterval > 0.0f) && (TimeUntilNextLogHeartbeatSeconds <= 0.0))
        {
            bLogLoadingScreenStatus = true;
            TimeUntilNextLogHeartbeatSeconds = Settings->LogLoadingScreenHeartbeatInterval;
        }
    }
    else
    {
        // 隐藏加载屏幕
        HideLoadingScreen();
 
        // 结束心跳检测
        FThreadHeartBeat::Get().MonitorCheckpointEnd(GetFName());
    }

    // 输出加载状态日志
    if (bLogLoadingScreenStatus)
    {
        UE_LOG(LogLoadingScreen, Log, TEXT("Loading screen showing: %d. Reason: %s"), bCurrentlyShowingLoadingScreen ? 1 : 0, *DebugReasonForShowingOrHidingLoadingScreen);
    }
}

bool ULoadingScreenManager::CheckForAnyNeedToShowLoadingScreen()
{
    // 初始化默认原因
    DebugReasonForShowingOrHidingLoadingScreen = TEXT("Reason for Showing/Hiding LoadingScreen is unknown!");

    const UGameInstance* LocalGameInstance = GetGameInstance();

    // 检查控制台强制显示
    if (LoadingScreenCVars::ForceLoadingScreenVisible)
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("CommonLoadingScreen.AlwaysShow is true"));
        return true;
    }

    // 检查世界上下文有效性
    const FWorldContext* Context = LocalGameInstance->GetWorldContext();
    if (Context == nullptr)
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("The game instance has a null WorldContext"));
        return true;
    }

    // 检查世界对象
    UWorld* World = Context->World();
    if (World == nullptr)
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have no world (FWorldContext's World() is null)"));
        return true;
    }

    // 检查游戏状态
    AGameStateBase* GameState = World->GetGameState<AGameStateBase>();
    if (GameState == nullptr)
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("GameState hasn't yet replicated (it's null)"));
        return true;
    }

    // 检查地图加载状态
    if (bCurrentlyInLoadMap)
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("bCurrentlyInLoadMap is true"));
        return true;
    }

    // 检查旅行URL
    if (!Context->TravelURL.IsEmpty())
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We have pending travel (the TravelURL is not empty)"));
        return true;
    }

    // 检查网络游戏状态
    if (Context->PendingNetGame != nullptr)
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are connecting to another server (PendingNetGame != nullptr)"));
        return true;
    }

    // 检查世界开始状态
    if (!World->HasBegunPlay())
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("World hasn't begun play"));
        return true;
    }

    // 检查无缝旅行状态
    if (World->IsInSeamlessTravel())
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("We are in seamless travel"));
        return true;
    }

    // 检查游戏状态是否需要加载屏幕
    if (ILoadingProcessInterface::ShouldShowLoadingScreen(GameState, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
    {
        return true;
    }

    // 检查游戏状态组件
    for (UActorComponent* TestComponent : GameState->GetComponents())
    {
        if (ILoadingProcessInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
        {
            return true;
        }
    }

    // 检查外部注册的加载处理器
    for (const TWeakInterfacePtr<ILoadingProcessInterface>& Processor : ExternalLoadingProcessors)
    {
        if (ILoadingProcessInterface::ShouldShowLoadingScreen(Processor.GetObject(), /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
        {
            return true;
        }
    }

    // 检查本地玩家控制器
    bool bFoundAnyLocalPC = false;
    bool bMissingAnyLocalPC = false;

    for (ULocalPlayer* LP : LocalGameInstance->GetLocalPlayers())
    {
        if (LP != nullptr)
        {
            if (APlayerController* PC = LP->PlayerController)
            {
                bFoundAnyLocalPC = true;

                // 检查玩家控制器
                if (ILoadingProcessInterface::ShouldShowLoadingScreen(PC, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
                {
                    return true;
                }

                // 检查玩家控制器组件
                for (UActorComponent* TestComponent : PC->GetComponents())
                {
                    if (ILoadingProcessInterface::ShouldShowLoadingScreen(TestComponent, /*out*/ DebugReasonForShowingOrHidingLoadingScreen))
                    {
                        return true;
                    }
                }
            }
            else
            {
                bMissingAnyLocalPC = true;
            }
        }
    }

    // 检查分屏模式下的玩家控制器
    UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();
    const bool bIsInSplitscreen = GameViewportClient->GetCurrentSplitscreenConfiguration() != ESplitScreenType::None;

    if (bIsInSplitscreen && bMissingAnyLocalPC)
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("At least one missing local player controller in splitscreen"));
        return true;
    }

    // 检查非分屏模式下的玩家控制器
    if (!bIsInSplitscreen && !bFoundAnyLocalPC)
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("Need at least one local player controller"));
        return true;
    }

    // 所有条件均不满足，无需显示加载屏幕
    DebugReasonForShowingOrHidingLoadingScreen = TEXT("(nothing wants to show it anymore)");
    return false;
}

bool ULoadingScreenManager::ShouldShowLoadingScreen()
{
    // 获取项目设置
    const UCommonLoadingScreenSettings* Settings = GetDefault<UCommonLoadingScreenSettings>();

    // 调试命令检查（非Shipping版本）
#if !UE_BUILD_SHIPPING
    static bool bCmdLineNoLoadingScreen = FParse::Param(FCommandLine::Get(), TEXT("NoLoadingScreen"));
    if (bCmdLineNoLoadingScreen)
    {
        DebugReasonForShowingOrHidingLoadingScreen = FString(TEXT("CommandLine has 'NoLoadingScreen'"));
        return false;
    }
#endif

    // 检查游戏视口有效性
    UGameInstance* LocalGameInstance = GetGameInstance();
    if (LocalGameInstance->GetGameViewportClient() == nullptr)
    {
        return false;
    }

    // 核心检测逻辑
    const bool bNeedToShowLoadingScreen = CheckForAnyNeedToShowLoadingScreen();

    // 延长显示逻辑（覆盖纹理流式加载）
    bool bWantToForceShowLoadingScreen = false;
    if (bNeedToShowLoadingScreen)
    {
        TimeLoadingScreenLastDismissed = -1.0; // 重置计时器
    }
    else
    {
        const double CurrentTime = FPlatformTime::Seconds();
        const bool bCanHoldLoadingScreen = (!GIsEditor || Settings->HoldLoadingScreenAdditionalSecsEvenInEditor);
        const double HoldLoadingScreenAdditionalSecs = bCanHoldLoadingScreen ? LoadingScreenCVars::HoldLoadingScreenAdditionalSecs : 0.0;

        if (TimeLoadingScreenLastDismissed < 0.0)
        {
            TimeLoadingScreenLastDismissed = CurrentTime; // 记录隐藏时间
        }
        const double TimeSinceScreenDismissed = CurrentTime - TimeLoadingScreenLastDismissed;

        // 延长显示以覆盖纹理流式加载
        if ((HoldLoadingScreenAdditionalSecs > 0.0) && (TimeSinceScreenDismissed < HoldLoadingScreenAdditionalSecs))
        {
            // 确保世界渲染开启（促进纹理流式加载）
            UGameViewportClient* GameViewportClient = GetGameInstance()->GetGameViewportClient();
            GameViewportClient->bDisableWorldRendering = false;

            DebugReasonForShowingOrHidingLoadingScreen = FString::Printf(TEXT("Keeping loading screen up for an additional %.2f seconds to allow texture streaming"), HoldLoadingScreenAdditionalSecs);
            bWantToForceShowLoadingScreen = true;
        }
    }

    return bNeedToShowLoadingScreen || bWantToForceShowLoadingScreen;
}

bool ULoadingScreenManager::IsShowingInitialLoadingScreen() const
{
    // 检查预加载屏幕状态
    FPreLoadScreenManager* PreLoadScreenManager = FPreLoadScreenManager::Get();
    return (PreLoadScreenManager != nullptr) && PreLoadScreenManager->HasValidActivePreLoadScreen();
}

void ULoadingScreenManager::ShowLoadingScreen()
{
    // 避免重复显示
    if (bCurrentlyShowingLoadingScreen)
    {
        return;
    }

    // 引擎初始加载屏幕运行时跳过
    if (FPreLoadScreenManager::Get() && FPreLoadScreenManager::Get()->HasActivePreLoadScreenType(EPreLoadScreenTypes::EngineLoadingScreen))
    {
        return;
    }

    // 记录显示时间
    TimeLoadingScreenShown = FPlatformTime::Seconds();
    bCurrentlyShowingLoadingScreen = true;

    // 性能分析标记
    CSV_EVENT(LoadingScreen, TEXT("Show"));

    const UCommonLoadingScreenSettings* Settings = GetDefault<UCommonLoadingScreenSettings>();

    if (IsShowingInitialLoadingScreen())
    {
        // 初始加载屏幕已存在的处理
        UE_LOG(LogLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is true."));
        UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
    }
    else
    {
        // 创建自定义加载屏幕
        UE_LOG(LogLoadingScreen, Log, TEXT("Showing loading screen when 'IsShowingInitialLoadingScreen()' is false."));
        UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

        UGameInstance* LocalGameInstance = GetGameInstance();

        // 启动输入屏蔽
        StartBlockingInput();

        // 通知观察者
        LoadingScreenVisibilityChanged.Broadcast(/*bIsVisible=*/ true);

        // 创建加载屏幕控件
        TSubclassOf<UUserWidget> LoadingScreenWidgetClass = Settings->LoadingScreenWidget.TryLoadClass<UUserWidget>();
        if (UUserWidget* UserWidget = UUserWidget::CreateWidgetInstance(*LocalGameInstance, LoadingScreenWidgetClass, NAME_None))
        {
            LoadingScreenWidget = UserWidget->TakeWidget(); // 获取Slate控件
        }
        else
        {
            // 回退到占位控件
            UE_LOG(LogLoadingScreen, Error, TEXT("Failed to load the loading screen widget %s, falling back to placeholder."), *Settings->LoadingScreenWidget.ToString());
            LoadingScreenWidget = SNew(SThrobber);
        }

        // 添加到视口
        UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();
        GameViewportClient->AddViewportWidgetContent(LoadingScreenWidget.ToSharedRef(), Settings->LoadingScreenZOrder);

        // 调整性能设置
        ChangePerformanceSettings(/*bEnableLoadingScreen=*/ true);

        // 强制刷新Slate（确保立即显示）
        if (!GIsEditor || Settings->ForceTickLoadingScreenEvenInEditor)
        {
            FSlateApplication::Get().Tick();
        }
    }
}

void ULoadingScreenManager::HideLoadingScreen()
{
    // 避免重复隐藏
    if (!bCurrentlyShowingLoadingScreen)
    {
        return;
    }

    // 停止输入屏蔽
    StopBlockingInput();

    if (IsShowingInitialLoadingScreen())
    {
        // 处理初始加载屏幕隐藏
        UE_LOG(LogLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is true."));
        UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);
    }
    else
    {
        // 处理自定义加载屏幕隐藏
        UE_LOG(LogLoadingScreen, Log, TEXT("Hiding loading screen when 'IsShowingInitialLoadingScreen()' is false."));
        UE_LOG(LogLoadingScreen, Log, TEXT("%s"), *DebugReasonForShowingOrHidingLoadingScreen);

        // 触发垃圾回收（释放加载资源）
        UE_LOG(LogLoadingScreen, Log, TEXT("Garbage Collecting before dropping load screen"));
        GEngine->ForceGarbageCollection(true);

        // 移除控件
        RemoveWidgetFromViewport();
    
        // 恢复性能设置
        ChangePerformanceSettings(/*bEnableLoadingScreen=*/ false);

        // 通知观察者
        LoadingScreenVisibilityChanged.Broadcast(/*bIsVisible=*/ false);
    }

    // 性能分析标记
    CSV_EVENT(LoadingScreen, TEXT("Hide"));

    // 计算显示时长
    const double LoadingScreenDuration = FPlatformTime::Seconds() - TimeLoadingScreenShown;
    UE_LOG(LogLoadingScreen, Log, TEXT("LoadingScreen was visible for %.2fs"), LoadingScreenDuration);

    bCurrentlyShowingLoadingScreen = false;
}

void ULoadingScreenManager::RemoveWidgetFromViewport()
{
    // 安全移除控件
    UGameInstance* LocalGameInstance = GetGameInstance();
    if (LoadingScreenWidget.IsValid())
    {
        if (UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient())
        {
            GameViewportClient->RemoveViewportWidgetContent(LoadingScreenWidget.ToSharedRef());
        }
        LoadingScreenWidget.Reset(); // 释放智能指针
    }
}

void ULoadingScreenManager::StartBlockingInput()
{
    // 注册输入预处理器
    if (!InputPreProcessor.IsValid())
    {
        InputPreProcessor = MakeShareable<FLoadingScreenInputPreProcessor>(new FLoadingScreenInputPreProcessor());
        FSlateApplication::Get().RegisterInputPreProcessor(InputPreProcessor, 0);
    }
}

void ULoadingScreenManager::StopBlockingInput()
{
    // 注销输入预处理器
    if (InputPreProcessor.IsValid())
    {
        FSlateApplication::Get().UnregisterInputPreProcessor(InputPreProcessor);
        InputPreProcessor.Reset();
    }
}

void ULoadingScreenManager::ChangePerformanceSettings(bool bEnabingLoadingScreen)
{
    // 调整加载期间的性能参数
    UGameInstance* LocalGameInstance = GetGameInstance();
    UGameViewportClient* GameViewportClient = LocalGameInstance->GetGameViewportClient();

    // 设置着色器缓存模式
    FShaderPipelineCache::SetBatchMode(bEnabingLoadingScreen ? FShaderPipelineCache::BatchMode::Fast : FShaderPipelineCache::BatchMode::Background);

    // 开关世界渲染
    GameViewportClient->bDisableWorldRendering = bEnabingLoadingScreen;

    // 设置高优先级加载
    if (UWorld* ViewportWorld = GameViewportClient->GetWorld())
    {
        if (AWorldSettings* WorldSettings = ViewportWorld->GetWorldSettings(false, false))
        {
            WorldSettings->bHighPriorityLoadingLocal = bEnabingLoadingScreen;
        }
    }

    if (bEnabingLoadingScreen)
    {
        // 延长心跳超时阈值
        double HangDurationMultiplier;
        if (!GConfig || !GConfig->GetDouble(TEXT("Core.System"), TEXT("LoadingScreenHangDurationMultiplier"), /*out*/ HangDurationMultiplier, GEngineIni))
        {
            HangDurationMultiplier = 1.0;
        }
        FThreadHeartBeat::Get().SetDurationMultiplier(HangDurationMultiplier);

        // 暂停卡顿检测
        FGameThreadHitchHeartBeat::Get().SuspendHeartBeat();
    }
    else
    {
        // 恢复默认心跳阈值
        FThreadHeartBeat::Get().SetDurationMultiplier(1.0);

        // 恢复卡顿检测
        FGameThreadHitchHeartBeat::Get().ResumeHeartBeat();
    }
}