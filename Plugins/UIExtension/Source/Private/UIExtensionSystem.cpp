// Copyright Epic Games, Inc. All Rights Reserved.

#include "UIExtensionSystem.h"  // UI扩展系统主头文件

#include "Blueprint/UserWidget.h"  // 引入用户控件类
#include "LogUIExtension.h"  // UI扩展系统日志
#include "UObject/Stack.h"  // 对象栈支持

#include UE_INLINE_GENERATED_CPP_BY_NAME(UIExtensionSystem)  // 自动生成的C++代码

class FSubsystemCollectionBase;  // 前置声明子系统集合基类

//=========================================================
// FUIExtensionPointHandle 扩展点句柄实现
//=========================================================

void FUIExtensionPointHandle::Unregister()
{
    // 检查扩展源有效性并执行注销
    if (UUIExtensionSubsystem* ExtensionSourcePtr = ExtensionSource.Get())
    {
        ExtensionSourcePtr->UnregisterExtensionPoint(*this);
    }
}

//=========================================================
// FUIExtensionHandle 扩展句柄实现
//=========================================================

void FUIExtensionHandle::Unregister()
{
    // 检查扩展源有效性并执行注销
    if (UUIExtensionSubsystem* ExtensionSourcePtr = ExtensionSource.Get())
    {
        ExtensionSourcePtr->UnregisterExtension(*this);
    }
}

//=========================================================
// FUIExtensionPoint 扩展点实现
//=========================================================

bool FUIExtensionPoint::DoesExtensionPassContract(const FUIExtension* Extension) const
{
    // 验证扩展数据有效性
    if (UObject* DataPtr = Extension->Data)
    {
        // 检查上下文对象匹配
        const bool bMatchesContext = 
            (ContextObject.IsExplicitlyNull() && Extension->ContextObject.IsExplicitlyNull()) ||
            ContextObject == Extension->ContextObject;

        // 上下文匹配时的类型检查
        if (bMatchesContext)
        {
            // 获取数据类（支持直接类或类实例）
            const UClass* DataClass = DataPtr->IsA(UClass::StaticClass()) ? Cast<UClass>(DataPtr) : DataPtr->GetClass();
            
            // 检查是否属于允许的数据类
            for (const UClass* AllowedDataClass : AllowedDataClasses)
            {
                if (DataClass->IsChildOf(AllowedDataClass) || DataClass->ImplementsInterface(AllowedDataClass))
                {
                    return true;  // 扩展符合契约
                }
            }
        }
    }
    return false;  // 扩展不符合契约
}

//=========================================================
// UUIExtensionSubsystem UI扩展子系统实现
//=========================================================

void UUIExtensionSubsystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
    Super::AddReferencedObjects(InThis, Collector);  // 调用父类引用收集

    // 收集扩展点和扩展数据的引用
    if (UUIExtensionSubsystem* ExtensionSubsystem = Cast<UUIExtensionSubsystem>(InThis))
    {
        // 收集扩展点允许的数据类引用
        for (auto MapIt = ExtensionSubsystem->ExtensionPointMap.CreateIterator(); MapIt; ++MapIt)
        {
            for (const TSharedPtr<FUIExtensionPoint>& ValueElement : MapIt.Value())
            {
                Collector.AddReferencedObjects(ValueElement->AllowedDataClasses);
            }
        }

        // 收集扩展数据对象引用
        for (auto MapIt = ExtensionSubsystem->ExtensionMap.CreateIterator(); MapIt; ++MapIt)
        {
            for (const TSharedPtr<FUIExtension>& ValueElement : MapIt.Value())
            {
                Collector.AddReferencedObject(ValueElement->Data);
            }
        }
    }
}

void UUIExtensionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);  // 调用父类初始化
}

void UUIExtensionSubsystem::Deinitialize()
{
    Super::Deinitialize();  // 调用父类反初始化
}

FUIExtensionPointHandle UUIExtensionSubsystem::RegisterExtensionPoint(
    const FGameplayTag& ExtensionPointTag,
    EUIExtensionPointMatch ExtensionPointTagMatchType,
    const TArray<UClass*>& AllowedDataClasses,
    FExtendExtensionPointDelegate ExtensionCallback)
{
    // 委托无上下文版本注册
    return RegisterExtensionPointForContext(ExtensionPointTag, nullptr, ExtensionPointTagMatchType, AllowedDataClasses, ExtensionCallback);
}

FUIExtensionPointHandle UUIExtensionSubsystem::RegisterExtensionPointForContext(
    const FGameplayTag& ExtensionPointTag,
    UObject* ContextObject,
    EUIExtensionPointMatch ExtensionPointTagMatchType,
    const TArray<UClass*>& AllowedDataClasses,
    FExtendExtensionPointDelegate ExtensionCallback)
{
    // 验证扩展点有效性
    if (!ExtensionPointTag.IsValid())
    {
        UE_LOG(LogUIExtension, Warning, TEXT("Trying to register an invalid extension point."));
        return FUIExtensionPointHandle();  // 返回空句柄
    }

    // 验证回调有效性
    if (!ExtensionCallback.IsBound())
    {
        UE_LOG(LogUIExtension, Warning, TEXT("Trying to register an invalid extension point."));
        return FUIExtensionPointHandle();
    }

    // 验证允许的数据类
    if (AllowedDataClasses.Num() == 0)
    {
        UE_LOG(LogUIExtension, Warning, TEXT("Trying to register an invalid extension point."));
        return FUIExtensionPointHandle();
    }

    // 创建或获取扩展点列表
    FExtensionPointList& List = ExtensionPointMap.FindOrAdd(ExtensionPointTag);

    // 创建新扩展点条目
    TSharedPtr<FUIExtensionPoint>& Entry = List.Add_GetRef(MakeShared<FUIExtensionPoint>());
    Entry->ExtensionPointTag = ExtensionPointTag;
    Entry->ContextObject = ContextObject;
    Entry->ExtensionPointTagMatchType = ExtensionPointTagMatchType;
    Entry->AllowedDataClasses = AllowedDataClasses;
    Entry->Callback = MoveTemp(ExtensionCallback);  // 转移回调所有权

    UE_LOG(LogUIExtension, Verbose, TEXT("Extension Point [%s] Registered"), *ExtensionPointTag.ToString());

    // 通知新扩展点现有扩展
    NotifyExtensionPointOfExtensions(Entry);

    // 返回有效句柄
    return FUIExtensionPointHandle(this, Entry);
}

FUIExtensionHandle UUIExtensionSubsystem::RegisterExtensionAsWidget(
    const FGameplayTag& ExtensionPointTag,
    TSubclassOf<UUserWidget> WidgetClass,
    int32 Priority)
{
    // 无上下文控件扩展注册
    return RegisterExtensionAsData(ExtensionPointTag, nullptr, WidgetClass, Priority);
}

FUIExtensionHandle UUIExtensionSubsystem::RegisterExtensionAsWidgetForContext(
    const FGameplayTag& ExtensionPointTag,
    UObject* ContextObject,
    TSubclassOf<UUserWidget> WidgetClass,
    int32 Priority)
{
    // 带上下文控件扩展注册
    return RegisterExtensionAsData(ExtensionPointTag, ContextObject, WidgetClass, Priority);
}

FUIExtensionHandle UUIExtensionSubsystem::RegisterExtensionAsData(
    const FGameplayTag& ExtensionPointTag,
    UObject* ContextObject,
    UObject* Data,
    int32 Priority)
{
    // 验证扩展点有效性
    if (!ExtensionPointTag.IsValid())
    {
        UE_LOG(LogUIExtension, Warning, TEXT("Trying to register an invalid extension."));
        return FUIExtensionHandle();  // 返回空句柄
    }

    // 验证数据有效性
    if (!Data)
    {
        UE_LOG(LogUIExtension, Warning, TEXT("Trying to register an invalid extension."));
        return FUIExtensionHandle();
    }

    // 创建或获取扩展列表
    FExtensionList& List = ExtensionMap.FindOrAdd(ExtensionPointTag);

    // 创建新扩展条目
    TSharedPtr<FUIExtension>& Entry = List.Add_GetRef(MakeShared<FUIExtension>());
    Entry->ExtensionPointTag = ExtensionPointTag;
    Entry->ContextObject = ContextObject;
    Entry->Data = Data;
    Entry->Priority = Priority;

    // 记录注册日志
    if (ContextObject)
    {
        UE_LOG(LogUIExtension, Verbose, TEXT("Extension [%s] @ [%s] Registered"), *GetNameSafe(Data), *ExtensionPointTag.ToString());
    }
    else
    {
        UE_LOG(LogUIExtension, Verbose, TEXT("Extension [%s] for [%s] @ [%s] Registered"), *GetNameSafe(Data), *GetNameSafe(ContextObject), *ExtensionPointTag.ToString());
    }

    // 通知所有相关扩展点
    NotifyExtensionPointsOfExtension(EUIExtensionAction::Added, Entry);

    // 返回有效句柄
    return FUIExtensionHandle(this, Entry);
}

void UUIExtensionSubsystem::NotifyExtensionPointOfExtensions(TSharedPtr<FUIExtensionPoint>& ExtensionPoint)
{
    // 遍历扩展点标签层级
    for (FGameplayTag Tag = ExtensionPoint->ExtensionPointTag; Tag.IsValid(); Tag = Tag.RequestDirectParent())
    {
        // 查找匹配的扩展列表
        if (const FExtensionList* ListPtr = ExtensionMap.Find(Tag))
        {
            // 复制扩展列表防止回调期间修改
            FExtensionList ExtensionArray(*ListPtr);

            // 检查每个扩展是否符合契约
            for (const TSharedPtr<FUIExtension>& Extension : ExtensionArray)
            {
                if (ExtensionPoint->DoesExtensionPassContract(Extension.Get()))
                {
                    // 创建扩展请求并触发回调
                    FUIExtensionRequest Request = CreateExtensionRequest(Extension);
                    ExtensionPoint->Callback.ExecuteIfBound(EUIExtensionAction::Added, Request);
                }
            }
        }

        // 精确匹配时提前退出
        if (ExtensionPoint->ExtensionPointTagMatchType == EUIExtensionPointMatch::ExactMatch)
        {
            break;
        }
    }
}

void UUIExtensionSubsystem::NotifyExtensionPointsOfExtension(EUIExtensionAction Action,TSharedPtr<FUIExtension>& Extension)
{
    bool bOnInitialTag = true;  // 标记初始标签层级
    
    // 遍历扩展标签层级
    for (FGameplayTag Tag = Extension->ExtensionPointTag; Tag.IsValid(); Tag = Tag.RequestDirectParent())
    {
        // 查找匹配的扩展点列表
        if (const FExtensionPointList* ListPtr = ExtensionPointMap.Find(Tag))
        {
            // 复制扩展点列表防止回调期间修改
            FExtensionPointList ExtensionPointArray(*ListPtr);

            // 检查每个扩展点
            for (const TSharedPtr<FUIExtensionPoint>& ExtensionPoint : ExtensionPointArray)
            {
                // 检查匹配条件
                const bool bShouldNotify = 
                    bOnInitialTag || 
                    (ExtensionPoint->ExtensionPointTagMatchType == EUIExtensionPointMatch::PartialMatch);
                
                if (bShouldNotify && ExtensionPoint->DoesExtensionPassContract(Extension.Get()))
                {
                    // 创建扩展请求并触发回调
                    FUIExtensionRequest Request = CreateExtensionRequest(Extension);
                    ExtensionPoint->Callback.ExecuteIfBound(Action, Request);
                }
            }
        }
        
        bOnInitialTag = false;  // 离开初始标签层级
    }
}

void UUIExtensionSubsystem::UnregisterExtension(const FUIExtensionHandle& ExtensionHandle)
{
    // 验证句柄有效性
    if (ExtensionHandle.IsValid())
    {
        // 安全检查扩展源一致性
        checkf(ExtensionHandle.ExtensionSource == this, TEXT("Trying to unregister an extension that's not from this extension subsystem."));

        TSharedPtr<FUIExtension> Extension = ExtensionHandle.DataPtr;
        
        // 查找扩展列表
        if (FExtensionList* ListPtr = ExtensionMap.Find(Extension->ExtensionPointTag))
        {
            // 记录注销日志
            if (Extension->ContextObject.IsExplicitlyNull())
            {
                UE_LOG(LogUIExtension, Verbose, TEXT("Extension [%s] @ [%s] Unregistered"), *GetNameSafe(Extension->Data), *Extension->ExtensionPointTag.ToString());
            }
            else
            {
                UE_LOG(LogUIExtension, Verbose, TEXT("Extension [%s] for [%s] @ [%s] Unregistered"), *GetNameSafe(Extension->Data), *GetNameSafe(Extension->ContextObject.Get()), *Extension->ExtensionPointTag.ToString());
            }

            // 通知扩展点移除操作
            NotifyExtensionPointsOfExtension(EUIExtensionAction::Removed, Extension);

            // 从列表中移除扩展
            ListPtr->RemoveSwap(Extension);
            
            // 清理空列表
            if (ListPtr->Num() == 0)
            {
                ExtensionMap.Remove(Extension->ExtensionPointTag);
            }
        }
    }
    else
    {
        UE_LOG(LogUIExtension, Warning, TEXT("Trying to unregister an invalid Handle."));
    }
}

void UUIExtensionSubsystem::UnregisterExtensionPoint(const FUIExtensionPointHandle& ExtensionPointHandle)
{
    // 验证句柄有效性
    if (ExtensionPointHandle.IsValid())
    {
        // 安全检查扩展源一致性
        check(ExtensionPointHandle.ExtensionSource == this);

        const TSharedPtr<FUIExtensionPoint> ExtensionPoint = ExtensionPointHandle.DataPtr;
        
        // 查找扩展点列表
        if (FExtensionPointList* ListPtr = ExtensionPointMap.Find(ExtensionPoint->ExtensionPointTag))
        {
            UE_LOG(LogUIExtension, Verbose, TEXT("Extension Point [%s] Unregistered"), *ExtensionPoint->ExtensionPointTag.ToString());

            // 从列表中移除扩展点
            ListPtr->RemoveSwap(ExtensionPoint);
            
            // 清理空列表
            if (ListPtr->Num() == 0)
            {
                ExtensionPointMap.Remove(ExtensionPoint->ExtensionPointTag);
            }
        }
    }
    else
    {
        UE_LOG(LogUIExtension, Warning, TEXT("Trying to unregister an invalid Handle."));
    }
}

FUIExtensionRequest UUIExtensionSubsystem::CreateExtensionRequest(const TSharedPtr<FUIExtension>& Extension)
{
    // 构建扩展请求结构
    FUIExtensionRequest Request;
    Request.ExtensionHandle = FUIExtensionHandle(this, Extension);
    Request.ExtensionPointTag = Extension->ExtensionPointTag;
    Request.Priority = Extension->Priority;
    Request.Data = Extension->Data;
    Request.ContextObject = Extension->ContextObject.Get();

    return Request;
}

//=========================================================
// 蓝图兼容函数
//=========================================================

FUIExtensionPointHandle UUIExtensionSubsystem::K2_RegisterExtensionPoint(
    FGameplayTag ExtensionPointTag,
    EUIExtensionPointMatch ExtensionPointTagMatchType,
    const TArray<UClass*>& AllowedDataClasses,
    FExtendExtensionPointDynamicDelegate ExtensionCallback)
{
    // 创建弱引用回调适配器
    return RegisterExtensionPoint(ExtensionPointTag, ExtensionPointTagMatchType, AllowedDataClasses, 
        FExtendExtensionPointDelegate::CreateWeakLambda(ExtensionCallback.GetUObject(), 
            [this, ExtensionCallback](EUIExtensionAction Action, const FUIExtensionRequest& Request) {
                ExtensionCallback.ExecuteIfBound(Action, Request);
            }));
}

FUIExtensionHandle UUIExtensionSubsystem::K2_RegisterExtensionAsWidget(
    FGameplayTag ExtensionPointTag,
    TSubclassOf<UUserWidget> WidgetClass,
    int32 Priority)
{
    // 无上下文控件扩展注册
    return RegisterExtensionAsWidget(ExtensionPointTag, WidgetClass, Priority);
}

FUIExtensionHandle UUIExtensionSubsystem::K2_RegisterExtensionAsWidgetForContext(
    FGameplayTag ExtensionPointTag,
    TSubclassOf<UUserWidget> WidgetClass,
    UObject* ContextObject,
    int32 Priority)
{
    // 带上下文控件扩展注册
    if (ContextObject)
    {
        return RegisterExtensionAsWidgetForContext(ExtensionPointTag, ContextObject, WidgetClass, Priority);
    }
    else
    {
        // 报告无效上下文对象
        FFrame::KismetExecutionMessage(TEXT("A null ContextObject was passed to Register Extension (Widget For Context)"), ELogVerbosity::Error);
        return FUIExtensionHandle();
    }
}

FUIExtensionHandle UUIExtensionSubsystem::K2_RegisterExtensionAsData(
    FGameplayTag ExtensionPointTag,
    UObject* Data,
    int32 Priority)
{
    // 无上下文数据扩展注册
    return RegisterExtensionAsData(ExtensionPointTag, nullptr, Data, Priority);
}

FUIExtensionHandle UUIExtensionSubsystem::K2_RegisterExtensionAsDataForContext(
    FGameplayTag ExtensionPointTag,
    UObject* ContextObject,
    UObject* Data,
    int32 Priority)
{
    // 带上下文数据扩展注册
    if (ContextObject)
    {
        return RegisterExtensionAsData(ExtensionPointTag, ContextObject, Data, Priority);
    }
    else
    {
        // 报告无效上下文对象
        FFrame::KismetExecutionMessage(TEXT("A null ContextObject was passed to Register Extension (Data For Context)"), ELogVerbosity::Error);
        return FUIExtensionHandle();
    }
}

//=========================================================
// UUIExtensionHandleFunctions 扩展句柄函数库
//=========================================================

void UUIExtensionHandleFunctions::Unregister(FUIExtensionHandle& Handle)
{
    Handle.Unregister();  // 注销扩展句柄
}

bool UUIExtensionHandleFunctions::IsValid(FUIExtensionHandle& Handle)
{
    return Handle.IsValid();  // 检查句柄有效性
}

//=========================================================
// UUIExtensionPointHandleFunctions 扩展点句柄函数库
//=========================================================

void UUIExtensionPointHandleFunctions::Unregister(FUIExtensionPointHandle& Handle)
{
    Handle.Unregister();  // 注销扩展点句柄
}

bool UUIExtensionPointHandleFunctions::IsValid(FUIExtensionPointHandle& Handle)
{
    return Handle.IsValid();  // 检查句柄有效性
}