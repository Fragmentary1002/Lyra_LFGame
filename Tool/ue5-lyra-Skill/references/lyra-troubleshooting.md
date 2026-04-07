# Lyra 故障排除指南

## 目录
1. [编译问题](#编译问题)
2. [运行时问题](#运行时问题)
3. [网络同步问题](#网络同步问题)
4. [GAS 相关问题](#gas-相关问题)
5. [Game Feature 问题](#game-feature-问题)
6. [性能优化](#性能优化)

---

## 编译问题

### 找不到 Lyra 类型

**错误**: `'LyraPawn' is not a class or namespace name`

**解决方案**:
```cpp
// 确保在 Build.cs 中添加依赖
PublicDependencyModuleNames.AddRange(new string[] {
    "LyraGame",
    "LyraInput",
    "LyraUI",
    "ModularGameplay",
    "GameFeatures"
});
```

### 模块加载失败

**错误**: `Module 'MyModule' could not be loaded`

**检查清单**:
1. `.uplugin` 文件格式是否正确
2. `Source` 目录结构是否符合 UE 规范
3. `.Build.cs` 文件是否存在且配置正确
4. 依赖模块是否已启用

### 生成项目文件失败

**解决方案**:
```bash
# 右键点击 .uproject 文件 -> Generate Visual Studio project files
# 或命令行
cd /path/to/project
"C:\Program Files\Epic Games\UE_5.x\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%CD%\Lyra.uproject" -game -engine -progress
```

---

## 运行时问题

### Pawn 未生成

**症状**: 玩家进入游戏但没有角色

**检查清单**:
```cpp
// 1. 检查 GameMode 是否配置了 DefaultPawnClass
void ALyraGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
    Super::InitGame(MapName, Options, ErrorMessage);
    
    // 确保 Experience 已加载
    if (ExperienceManager)
    {
        ExperienceManager->SetCurrentExperience(DefaultExperience);
    }
}

// 2. 检查 Experience 是否配置了 DefaultPawnData
// 在蓝图中的 LyraExperienceDefinition -> DefaultPawnData

// 3. 检查 PawnData 是否配置了 CharacterClass
// 在蓝图中的 LyraPawnData -> CharacterClass
```

### 输入无响应

**症状**: 按键但角色无反应

**调试步骤**:
```cpp
// 1. 检查 InputMappingContext 是否正确应用
void ULyraHeroComponent::InitializePlayerInput(UInputComponent* PlayerInputComponent)
{
    if (const ULyraPawnData* PawnData = GetPawnData())
    {
        if (const UInputMappingContext* IMC = PawnData->InputMappingContext.LoadSynchronous())
        {
            // 确保这里成功执行
            UE_LOG(LogLyra, Log, TEXT("Applying IMC: %s"), *IMC->GetName());
        }
    }
}

// 2. 检查 InputConfig 映射
void ULyraInputComponent::AddAbilityBindings(const ULyraInputConfig* InputConfig)
{
    for (const auto& Pair : InputConfig->InputActionToTag)
    {
        UE_LOG(LogLyra, Log, TEXT("InputAction %s -> Tag %s"), 
            *Pair.Key->GetName(),
            *Pair.Value.ToString());
    }
}

// 3. 检查 Ability 是否正确授予
void UMyComponent::DebugGrantedAbilities()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayAbilitySpecHandle* Handles = ASC->GetActivatableAbilities().GetData();
        for (int32 i = 0; i < ASC->GetActivatableAbilities().Num(); i++)
        {
            FGameplayAbilitySpec& Spec = ASC->GetActivatableAbilities()[i];
            UE_LOG(LogLyra, Log, TEXT("Granted Ability: %s"), *Spec.Ability->GetName());
        }
    }
}
```

### 相机不工作

**症状**: 视角错误或无法移动

**检查清单**:
1. Pawn 上是否有 CameraComponent
2. LyraPawnData 中是否配置了 CameraMode
3. 相机模式是否正确初始化

```cpp
void ULyraHeroComponent::SetCameraMode(TSubclassOf<ULyraCameraMode> CameraMode)
{
    if (ALyraPlayerController* PC = GetController<ALyraPlayerController>())
    {
        if (ULyraCameraComponent* CameraComponent = PC->GetCameraComponent())
        {
            CameraComponent->SetCameraMode(CameraMode);
        }
    }
}
```

---

## 网络同步问题

### 客户端看不到其他玩家

**症状**: 客户端只能看到自己，看不到其他玩家

**检查清单**:
1. PlayerState 是否正确复制
2. Pawn 是否正确复制
3. 网络权限检查

```cpp
void AMyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(AMyPawn, ReplicatedProperty);
}

// 调试网络
void AMyPawn::DebugReplication()
{
    UE_LOG(LogLyra, Log, TEXT("Role: %s, RemoteRole: %s, HasAuthority: %s"),
        *UEnum::GetValueAsString(GetLocalRole()),
        *UEnum::GetValueAsString(GetRemoteRole()),
        HasAuthority() ? TEXT("Yes") : TEXT("No"));
}
```

### ASC 复制问题

**症状**: 客户端看不到技能效果

**解决方案**:
```cpp
// 确保 ASC 在正确的位置
// PlayerState 上的 ASC 适用于持久化状态
// Pawn 上的 ASC 适用于实例特定能力

void ALyraPlayerState::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    
    // 初始化 ASC
    AbilitySystemComponent->InitAbilityActorInfo(this, GetPawn());
}

void ALyraPawn::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    
    // 更新 ASC 的 ActorInfo
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->InitAbilityActorInfo(GetPlayerState(), this);
    }
}
```

### 属性不同步

**症状**: 服务器和客户端属性值不一致

```cpp
UCLASS()
class AMyActor : public AActor
{
    GENERATED_BODY()
    
public:
    UPROPERTY(ReplicatedUsing = OnRep_MyProperty)
    float MyProperty;
    
    UFUNCTION()
    void OnRep_MyProperty()
    {
        // 处理复制后的逻辑
        UpdateUI();
    }
};

void AMyActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION(AMyActor, MyProperty, COND_OwnerOnly);
}
```

---

## GAS 相关问题

### Ability 无法激活

**症状**: 调用 TryActivateAbility 返回 false

**调试步骤**:
```cpp
void UMyComponent::DebugAbilityActivation(TSubclassOf<UGameplayAbility> AbilityClass)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        // 检查是否已授予
        FGameplayAbilitySpec* Spec = ASC->FindAbilitySpecFromClass(AbilityClass);
        if (!Spec)
        {
            UE_LOG(LogLyra, Warning, TEXT("Ability not granted!"));
            return;
        }
        
        // 检查标签阻塞
        FGameplayTagContainer BlockedTags;
        ASC->GetBlockedAbilityTags(BlockedTags);
        UE_LOG(LogLyra, Log, TEXT("Blocked tags: %s"), *BlockedTags.ToString());
        
        // 尝试激活并获取失败原因
        FGameplayAbilitySpecHandle Handle = Spec->Handle;
        bool bSuccess = ASC->TryActivateAbility(Handle);
        
        if (!bSuccess)
        {
            UE_LOG(LogLyra, Warning, TEXT("Ability activation failed!"));
            
            // 检查冷却
            if (ASC->IsCooldown(AbilityClass) > 0)
            {
                UE_LOG(LogLyra, Warning, TEXT("Ability is on cooldown"));
            }
            
            // 检查消耗
            // ...
        }
    }
}
```

### GameplayEffect 未应用

**症状**: 效果未生效，属性未改变

```cpp
void UMyComponent::DebugGameplayEffect(TSubclassOf<UGameplayEffect> EffectClass)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
        Context.AddSourceObject(this);
        
        FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EffectClass, 1.0f, Context);
        
        if (Spec.IsValid())
        {
            // 检查 Spec 属性
            UE_LOG(LogLyra, Log, TEXT("Effect Duration: %f"), Spec.Data->Duration);
            UE_LOG(LogLyra, Log, TEXT("Effect Period: %f"), Spec.Data->Period);
            
            // 应用效果
            FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
            
            if (ActiveHandle.WasSuccessfullyApplied())
            {
                UE_LOG(LogLyra, Log, TEXT("Effect applied successfully"));
            }
            else
            {
                UE_LOG(LogLyra, Warning, TEXT("Effect application failed"));
            }
        }
    }
}
```

### 预测失效

**症状**: 客户端预测失败，出现卡顿

**解决方案**:
```cpp
UCLASS()
class UMyPredictedAbility : public UGameplayAbility
{
    // 确保 Ability 支持预测
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags,
        const FGameplayTagContainer* TargetTags,
        FGameplayTagContainer* OptionalRelevantTags) const override
    {
        // 检查预测权限
        if (ActorInfo->IsNetAuthority())
        {
            return true;
        }
        
        // 客户端预测检查
        return HasAuthorityOrPredictionKey(ActorInfo, &Handle);
    }
    
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override
    {
        if (HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
        {
            // 执行预测性操作
            CommitAbility(Handle, ActorInfo, ActivationInfo);
            
            // 发送 RPC 到服务器
            if (!HasAuthority(&ActivationInfo))
            {
                Server_DoAction();
            }
        }
    }
    
    UFUNCTION(Server, Reliable)
    void Server_DoAction();
};
```

---

## Game Feature 问题

### Game Feature 未加载

**症状**: Game Feature 中的功能未生效

**调试步骤**:
```cpp
void UMyComponent::DebugGameFeatures()
{
    UGameFeaturesSubsystem& GFSubsystem = UGameFeaturesSubsystem::Get();
    
    // 列出所有已加载的插件
    TArray<FString> PluginURLs;
    GFSubsystem.GetLoadedGameFeaturePluginURLs(PluginURLs);
    
    for (const FString& URL : PluginURLs)
    {
        EGameFeaturePluginState State;
        GFSubsystem.GetPluginState(URL, State);
        
        UE_LOG(LogLyra, Log, TEXT("Plugin: %s, State: %s"),
            *URL,
            *UEnum::GetValueAsString(State));
    }
}
```

### Actions 未执行

**症状**: Game Feature 加载但 Actions 未生效

**检查清单**:
```cpp
// 1. 检查 Action 配置
void UGameFeatureAction_AddComponents::OnGameFeatureActivating(FGameFeatureActivatingContext& Context)
{
    // 确保 ActorClass 配置正确
    if (!ActorClass.IsValid())
    {
        UE_LOG(LogLyra, Warning, TEXT("ActorClass is not valid!"));
        return;
    }
    
    // 2. 检查组件类
    for (TSubclassOf<UActorComponent> CompClass : ComponentClasses)
    {
        if (!CompClass)
        {
            UE_LOG(LogLyra, Warning, TEXT("Component class is null!"));
        }
    }
}

// 3. 手动触发 Action 调试
void UMyComponent::ForceExecuteActions()
{
    if (UGameFeatureData* Data = LoadObject<UGameFeatureData>(nullptr, TEXT("/Path/To/Your/GameFeatureData.GameFeatureData")))
    {
        FGameFeatureActivatingContext Context;
        for (UGameFeatureAction* Action : Data->Actions)
        {
            if (Action)
            {
                UE_LOG(LogLyra, Log, TEXT("Executing action: %s"), *Action->GetName());
                Action->OnGameFeatureActivating(Context);
            }
        }
    }
}
```

### Experience 加载失败

**症状**: Experience 未正确加载，Game Features 未激活

```cpp
void ULyraExperienceManagerComponent::DebugExperienceLoading()
{
    UE_LOG(LogLyra, Log, TEXT("Current Experience: %s"),
        CurrentExperience ? *CurrentExperience->GetName() : TEXT("None"));
    
    UE_LOG(LogLyra, Log, TEXT("Is Experience Loaded: %s"),
        IsExperienceLoaded() ? TEXT("Yes") : TEXT("No"));
    
    if (CurrentExperience)
    {
        UE_LOG(LogLyra, Log, TEXT("Game Features to enable:"));
        for (const FString& Feature : CurrentExperience->GameFeaturesToEnable)
        {
            UE_LOG(LogLyra, Log, TEXT("  - %s"), *Feature);
            
            // 检查每个 Feature 的状态
            EGameFeaturePluginState State;
            UGameFeaturesSubsystem::Get().GetPluginState(Feature, State);
            UE_LOG(LogLyra, Log, TEXT("    State: %s"), *UEnum::GetValueAsString(State));
        }
    }
}
```

---

## 性能优化

### ASC 查询优化

```cpp
// 缓存 ASC 引用
class UMyComponent : public UActorComponent
{
    UAbilitySystemComponent* GetAbilitySystemComponent() const
    {
        if (!CachedASC)
        {
            CachedASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
        }
        return CachedASC;
    }
    
    mutable TWeakObjectPtr<UAbilitySystemComponent> CachedASC;
};
```

### GameplayTag 优化

```cpp
// 预定义常用标签
struct FLyraGameplayTags
{
    static const FGameplayTag& GetInputTag_Move()
    {
        static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("InputTag.Move"));
        return Tag;
    }
    
    static const FGameplayTag& GetStatus_Death()
    {
        static FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("Status.Death"));
        return Tag;
    }
};

// 使用
void UMyComponent::CheckDeath()
{
    if (ASC->HasMatchingGameplayTag(FLyraGameplayTags::GetStatus_Death()))
    {
        // ...
    }
}
```

### 批处理操作

```cpp
// 批量授予能力
void UMyComponent::GrantAbilitiesBatch(const TArray<TSubclassOf<UGameplayAbility>>& AbilityClasses)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        // 开始批量操作
        ASC->BatchRPCTryActivateAbility();
        
        for (const TSubclassOf<UGameplayAbility>& AbilityClass : AbilityClasses)
        {
            if (AbilityClass)
            {
                ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
            }
        }
        
        // 结束批量操作
        ASC->BatchRPCTryActivateAbilityEnd();
    }
}
```

### 内存优化

```cpp
// 使用软引用延迟加载
UPROPERTY(EditDefaultsOnly, meta = (SoftClassPtr))
TSoftClassPtr<UGameplayAbility> LazyLoadedAbility;

void UMyComponent::UseLazyLoadedAbility()
{
    if (LazyLoadedAbility.IsValid())
    {
        TSubclassOf<UGameplayAbility> LoadedClass = LazyLoadedAbility.LoadSynchronous();
        // 使用 LoadedClass
    }
}
```
