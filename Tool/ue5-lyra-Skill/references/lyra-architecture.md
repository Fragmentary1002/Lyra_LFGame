# Lyra 架构详细参考

## 目录
1. [核心类层次结构](#核心类层次结构)
2. [Game Feature 系统详解](#game-feature-系统详解)
3. [GAS 集成详解](#gas-集成详解)
4. [Experience 系统详解](#experience-系统详解)
5. [输入系统详解](#输入系统详解)
6. [UI 系统详解](#ui-系统详解)
7. [常见问题与解决方案](#常见问题与解决方案)

---

## 核心类层次结构

### 游戏框架类

```
AGameModeBase
└── ALyraGameMode
    ├── 负责游戏流程管理
    ├── 协调各个 Game Feature
    └── 轻量逻辑，主要做协调

AGameStateBase
└── ALyraGameState
    ├── 存储共享游戏状态
    ├── 持有 LyraExperienceManagerComponent
    └── 复制状态到所有客户端

APlayerController
└── ALyraPlayerController
    ├── 处理玩家输入
    ├── 生成和管理 Pawn
    ├── 管理 UI
    └── 处理相机

APlayerState
└── ALyraPlayerState
    ├── 存储持久化玩家数据
    ├── 持有 AbilitySystemComponent
    └── 复制数据到所有相关客户端

APawn
└── ALyraPawn
    ├── 物理表现
    ├── 持有 LyraPawnExtensionComponent
    └── 逻辑较少，由组件处理
```

### 关键组件类

```
UActorComponent
├── ULyraPawnExtensionComponent (关键)
│   ├── 管理 Pawn 生命周期
│   ├── 动态加载/卸载组件
│   ├── 持有 ASC
│   └── 协调其他组件
│
├── ULyraHeroComponent
│   ├── 应用 LyraPawnData
│   ├── 设置输入映射
│   ├── 初始化相机
│   └── 管理装备外观
│
└── ULyraExperienceManagerComponent
    ├── 加载 ExperienceDefinition
    ├── 管理 GameFeatures
    └── 处理体验级 Actions
```

---

## Game Feature 系统详解

### 插件结构

```
MyGameFeature/
├── MyGameFeature.uplugin
├── Source/
│   └── MyGameFeature/
│       └── MyGameFeatureData.h/cpp
└── Content/
    └── GameFeatureData/
        └── MyGameFeatureData.uasset
```

### GameFeatureData 配置

```cpp
UCLASS()
class MYGAMEFEATURE_API UMyGameFeatureData : public UGameFeatureData
{
    GENERATED_BODY()
    
public:
    // 定义插件激活时执行的操作
    UPROPERTY(EditDefaultsOnly, Category = "Actions")
    TArray<TObjectPtr<UGameFeatureAction>> Actions;
};
```

### 常用 Actions

#### AddComponents Action

```cpp
// 动态添加组件到符合条件的 Actor
UCLASS()
class UGameFeatureAction_AddComponents : public UGameFeatureAction
{
    // 目标 Actor 类型
    UPROPERTY(EditDefaultsOnly)
    TSoftClassPtr<AActor> ActorClass;
    
    // 要添加的组件
    UPROPERTY(EditDefaultsOnly)
    TArray<TSubclassOf<UActorComponent>> ComponentClasses;
};
```

#### AddAbilities Action

```cpp
// 动态授予能力和效果
UCLASS()
class UGameFeatureAction_AddAbilities : public UGameFeatureAction
{
    // 目标 Actor 类型
    UPROPERTY(EditDefaultsOnly)
    TSoftClassPtr<AActor> ActorClass;
    
    // 要授予的 Ability Sets
    UPROPERTY(EditDefaultsOnly)
    TArray<TSoftObjectPtr<ULyraAbilitySet>> AbilitySets;
};
```

### 自定义 Action 示例

```cpp
UCLASS()
class UMyCustomGameFeatureAction : public UGameFeatureAction
{
    GENERATED_BODY()
    
public:
    virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override
    {
        // 插件激活时执行
        // 可以注册回调、初始化系统等
    }
    
    virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override
    {
        // 插件停用时执行
        // 清理资源、注销回调等
    }
};
```

---

## GAS 集成详解

### AbilitySystemComponent 位置

```cpp
// PlayerState 上的 ASC（玩家状态相关能力）
class ALyraPlayerState : public APlayerState
{
    UPROPERTY(VisibleAnywhere, Category = "GAS")
    TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;
};

// PawnExtensionComponent 上的 ASC（Pawn 实例相关能力）
class ULyraPawnExtensionComponent : public UActorComponent
{
    UPROPERTY(VisibleAnywhere, Category = "GAS")
    TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;
};
```

### LyraAbilitySet 使用

```cpp
UCLASS()
class ULyraAbilitySet : public UPrimaryDataAsset
{
    GENERATED_BODY()
    
public:
    // 授予的能力
    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
    TArray<TSubclassOf<ULyraGameplayAbility>> GrantedAbilities;
    
    // 授予的效果
    UPROPERTY(EditDefaultsOnly, Category = "Effects")
    TArray<TSubclassOf<ULyraGameplayEffect>> GrantedEffects;
    
    // 授予给目标 ASC
    void GiveToAbilitySystem(UAbilitySystemComponent* ASC);
};
```

### 自定义 GameplayAbility

```cpp
UCLASS()
class ULyraGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()
    
public:
    // 激活标签
    UPROPERTY(EditDefaultsOnly, Category = "Tags")
    FGameplayTagContainer ActivationOwnedTags;
    
    // 触发标签
    UPROPERTY(EditDefaultsOnly, Category = "Tags")
    FGameplayTagContainer ActivationRequiredTags;
    
    // 阻塞标签
    UPROPERTY(EditDefaultsOnly, Category = "Tags")
    FGameplayTagContainer ActivationBlockedTags;
    
protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
};
```

### 属性集定义

```cpp
UCLASS()
class ULyraHealthSet : public UAttributeSet
{
    GENERATED_BODY()
    
public:
    UPROPERTY(BlueprintReadOnly, Category = "Health")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(ULyraHealthSet, Health)
    
    UPROPERTY(BlueprintReadOnly, Category = "Health")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(ULyraHealthSet, MaxHealth)
    
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
```

---

## Experience 系统详解

### ExperienceDefinition 结构

```cpp
UCLASS()
class ULyraExperienceDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
    
public:
    // 要激活的 Game Features
    UPROPERTY(EditDefaultsOnly, Category = "Game Features")
    TArray<FString> GameFeaturesToEnable;
    
    // 默认 PawnData
    UPROPERTY(EditDefaultsOnly, Category = "Pawn")
    TSoftObjectPtr<ULyraPawnData> DefaultPawnData;
    
    // 体验级 Actions
    UPROPERTY(EditDefaultsOnly, Category = "Actions")
    TArray<TObjectPtr<UGameFeatureAction>> Actions;
};
```

### ExperienceManager 工作流程

```cpp
class ULyraExperienceManagerComponent : public UActorComponent
{
public:
    // 设置当前 Experience
    UFUNCTION(BlueprintCallable)
    void SetCurrentExperience(TSoftObjectPtr<ULyraExperienceDefinition> Experience);
    
    // 检查 Experience 是否已加载
    bool IsExperienceLoaded() const;
    
private:
    // 加载 Experience 及其依赖
    void LoadExperience();
    
    // 激活 Game Features
    void ActivateGameFeatures();
    
    // 执行 Experience Actions
    void ExecuteActions();
};
```

### 加载流程

```
1. GameMode 初始化
   └── 确定 Experience（通过地图设置或游戏模式配置）

2. ExperienceManager 加载 ExperienceDefinition
   └── 异步加载 Primary Data Asset

3. 加载 GameFeaturesToEnable 列表中的插件
   └── 每个插件被加载并激活

4. 执行插件的 Actions
   └── AddComponents、AddAbilities 等

5. 执行 Experience 自身的 Actions
   └── 体验特定的初始化

6. Experience 加载完成，游戏开始
```

---

## 输入系统详解

### LyraInputConfig

```cpp
UCLASS()
class ULyraInputConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()
    
public:
    // 输入映射
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TMap<TObjectPtr<UInputAction>, FGameplayTag> InputActionToTag;
    
    // 根据 InputAction 获取 Tag
    FGameplayTag FindTagForInputAction(const UInputAction* InputAction) const;
};
```

### LyraInputComponent

```cpp
class ULyraInputComponent : public UEnhancedInputComponent
{
public:
    // 添加能力绑定
    void AddAbilityBindings(const ULyraInputConfig* InputConfig);
    
protected:
    // 输入触发时广播 Tag
    void OnInputTriggered(const FGameplayTag& InputTag);
};
```

### 输入绑定流程

```
1. LyraHeroComponent::InitializePlayerInput
   └── 应用 InputMappingContext

2. LyraInputComponent::AddAbilityBindings
   └── 遍历 InputConfig 中的映射

3. 绑定输入事件到 OnInputTriggered

4. 输入触发时
   └── 查找对应的 InputTag
   └── 广播 InputTag
   └── ASC 尝试激活匹配的 Ability
```

### 配置示例

```cpp
// LyraPawnData 中配置
UCLASS()
class ULyraPawnData : public UPrimaryDataAsset
{
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TSoftObjectPtr<UInputMappingContext> InputMappingContext;
    
    UPROPERTY(EditDefaultsOnly, Category = "Input")
    TSoftObjectPtr<ULyraInputConfig> InputConfig;
};
```

---

## UI 系统详解

### Common UI 框架集成

```cpp
// Lyra HUD
UCLASS()
class ALyraHUD : public AHUD
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<ULyraHUDLayout> HUDLayoutClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TArray<TSubclassOf<ULyraWidget>> WidgetClasses;
};

// HUD 布局
UCLASS()
class ULyraHUDLayout : public UCommonUserWidget
{
    // 定义 HUD 布局结构
};
```

### GameplayMessage 子系统

```cpp
// 定义消息类型
USTRUCT()
struct FLyraPlayerDeathMessage
{
    GENERATED_BODY()
    
    UPROPERTY()
    TObjectPtr<APlayerState> Victim;
    
    UPROPERTY()
    TObjectPtr<APlayerState> Killer;
};

// 广播消息
UGameplayMessageSubsystem::Get(this).BroadcastMessage(TAG_Lyra_Elimination_Message, Message);

// 监听消息
UGameplayMessageSubsystem::Get(this).RegisterListener(TAG_Lyra_Elimination_Message, this, &UMyWidget::OnPlayerDeath);
```

### UI 数据绑定

```cpp
UCLASS()
class ULyraPlayerStatusWidget : public UCommonUserWidget
{
protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonTextBlock> HealthText;
    
    UFUNCTION()
    void OnHealthChanged(float NewHealth, float MaxHealth);
};
```

---

## 常见问题与解决方案

### 1. Pawn 初始化顺序问题

**问题**: 组件在 Pawn 完全初始化前尝试访问 ASC

**解决方案**:
```cpp
void ULyraPawnExtensionComponent::OnPawnReadyToInitialize()
{
    // 等待 Pawn 完全初始化后再执行逻辑
    if (ALyraPawn* Pawn = GetPawn<ALyraPawn>())
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
        {
            // 安全地访问 ASC
        }
    }
}
```

### 2. Game Feature 加载顺序

**问题**: 依赖的 Game Feature 尚未加载

**解决方案**:
```cpp
// 在 ExperienceDefinition 中正确排序 GameFeaturesToEnable
// 或使用依赖声明

UCLASS()
class UMyGameFeatureData : public UGameFeatureData
{
    UPROPERTY(EditDefaultsOnly)
    TArray<FString> Dependencies; // 声明依赖的其他插件
};
```

### 3. Ability 输入绑定失败

**问题**: 输入触发但 Ability 未激活

**检查清单**:
- InputConfig 是否正确配置 InputAction 到 Tag 的映射
- Ability 的 ActivationRequiredTags 是否匹配 InputTag
- ASC 是否已授予该 Ability
- InputMappingContext 是否正确应用到 PlayerController

### 4. 网络复制问题

**问题**: 客户端看不到服务器的状态变化

**检查清单**:
- 属性是否标记为 Replicated
- GetLifetimeReplicatedProps 是否正确实现
- ASC 是否在正确的位置（PlayerState 或 Pawn）
- 网络权限检查（HasAuthority）

### 5. Experience 加载失败

**问题**: ExperienceDefinition 加载后 Game Features 未激活

**调试步骤**:
```cpp
// 检查 ExperienceManager 状态
UE_LOG(LogLyraExperience, Log, TEXT("Experience loaded: %s"), 
    ExperienceManager->IsExperienceLoaded() ? TEXT("Yes") : TEXT("No"));

// 检查 Game Feature 状态
UGameFeaturesSubsystem::Get().GetPluginState(MyPluginURL, State);
```

---

## 最佳实践

1. **保持 Game Feature 独立**: 避免 Game Feature 之间的直接依赖
2. **使用数据资产**: 将可配置数据提取到 PrimaryDataAsset
3. **遵循生命周期**: 正确实现组件的初始化/清理
4. **利用 GameplayTags**: 使用标签系统进行松耦合通信
5. **测试网络**: 始终在监听服务器模式下测试多人功能
