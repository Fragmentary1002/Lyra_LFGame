# Lyra 代码模式与示例

## 目录
1. [创建自定义 Game Feature](#创建自定义-game-feature)
2. [创建自定义 Ability](#创建自定义-ability)
3. [创建自定义 PawnData](#创建自定义-pawndata)
4. [创建自定义 Experience](#创建自定义-experience)
5. [组件间通信模式](#组件间通信模式)
6. [数据驱动配置示例](#数据驱动配置示例)

---

## 创建自定义 Game Feature

### 1. 创建插件

```bash
# 在 Lyra 项目的 Plugins 目录下创建新插件
# 或使用 UE 编辑器：编辑 -> 插件 -> 新建插件
```

### 2. 创建 GameFeatureData

```cpp
// MyShooterFeatureData.h
#pragma once

#include "CoreMinimal.h"
#include "GameFeatures/GameFeatureData.h"
#include "MyShooterFeatureData.generated.h"

UCLASS()
class MYSHOOTERFEATURE_API UMyShooterFeatureData : public UGameFeatureData
{
    GENERATED_BODY()
};
```

### 3. 配置 Actions（蓝图方式）

在编辑器中：
1. 创建 `UGameFeatureData` 派生类的蓝图
2. 添加 `Add Abilities` Action：
   - Actor Class: `LyraPawn`
   - Ability Sets: 添加你的 AbilitySet
3. 添加 `Add Components` Action：
   - Actor Class: `LyraPawn`
   - Component Classes: 添加你的组件

### 4. 配置插件元数据

```json
// MyShooterFeature.uplugin
{
    "FileVersion": 3,
    "Version": 1,
    "VersionName": "1.0",
    "FriendlyName": "My Shooter Feature",
    "Description": "Custom shooter mechanics",
    "Category": "Game Features",
    "CreatedBy": "Your Name",
    "Plugins": [
        {
            "Name": "GameFeatures",
            "Enabled": true
        },
        {
            "Name": "ModularGameplay",
            "Enabled": true
        }
    ]
}
```

---

## 创建自定义 Ability

### 基础射击 Ability

```cpp
// MyShootAbility.h
#pragma once

#include "CoreMinimal.h"
#include "LyraGameplayAbility.h"
#include "MyShootAbility.generated.h"

UCLASS()
class UMyShootAbility : public ULyraGameplayAbility
{
    GENERATED_BODY()
    
public:
    UMyShootAbility();
    
protected:
    UPROPERTY(EditDefaultsOnly, Category = "Shooting")
    float Damage = 25.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Shooting")
    float MaxRange = 10000.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Shooting")
    TSubclassOf<UGameplayEffect> DamageEffectClass;
    
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;
    
    UFUNCTION()
    void OnFirePressed();
    
    void PerformLineTrace();
    void ApplyDamageToTarget(AActor* Target);
};
```

```cpp
// MyShootAbility.cpp
#include "MyShootAbility.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"

UMyShootAbility::UMyShootAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
    
    // 定义触发标签
    FGameplayTag InputTag = FGameplayTag::RequestGameplayTag(FName("InputTag.Weapon.Fire"));
    ActivationOwnedTags.AddTag(InputTag);
}

void UMyShootAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // 执行射击
    PerformLineTrace();
    
    // 结束 Ability
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UMyShootAbility::PerformLineTrace()
{
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Character) return;
    
    UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>();
    if (!Camera) return;
    
    FVector Start = Camera->GetComponentLocation();
    FVector End = Start + Camera->GetForwardVector() * MaxRange;
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Character);
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
    {
        if (AActor* HitActor = HitResult.GetActor())
        {
            ApplyDamageToTarget(HitActor);
        }
    }
}

void UMyShootAbility::ApplyDamageToTarget(AActor* Target)
{
    if (!DamageEffectClass) return;
    
    UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
    if (!TargetASC) return;
    
    FGameplayEffectContextHandle Context = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
    Context.AddSourceObject(this);
    
    FGameplayEffectSpecHandle Spec = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
        DamageEffectClass, 1.0f, Context);
    
    if (Spec.IsValid())
    {
        Spec.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);
        TargetASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
    }
}
```

---

## 创建自定义 PawnData

### 定义 PawnData

```cpp
// MyHeroPawnData.h
#pragma once

#include "CoreMinimal.h"
#include "Character/LyraPawnData.h"
#include "MyHeroPawnData.generated.h"

UCLASS()
class UMyHeroPawnData : public ULyraPawnData
{
    GENERATED_BODY()
    
public:
    UMyHeroPawnData();
    
    // 英雄特定属性
    UPROPERTY(EditDefaultsOnly, Category = "Hero")
    FText HeroName;
    
    UPROPERTY(EditDefaultsOnly, Category = "Hero")
    FText HeroDescription;
    
    UPROPERTY(EditDefaultsOnly, Category = "Hero")
    TSoftObjectPtr<UTexture2D> HeroIcon;
    
    // 移动属性
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float BaseSpeed = 600.0f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Movement")
    float SprintSpeed = 900.0f;
    
    // 技能配置
    UPROPERTY(EditDefaultsOnly, Category = "Abilities")
    TArray<TSubclassOf<UGameplayAbility>> HeroAbilities;
};
```

### 在 HeroComponent 中应用

```cpp
// MyHeroComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Character/LyraHeroComponent.h"
#include "MyHeroComponent.generated.h"

UCLASS()
class UMyHeroComponent : public ULyraHeroComponent
{
    GENERATED_BODY()
    
protected:
    virtual void OnPawnDataApplied() override;
    
    void SetupHeroMovement();
    void SetupHeroAbilities();
};
```

```cpp
// MyHeroComponent.cpp
#include "MyHeroComponent.h"
#include "MyHeroPawnData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

void UMyHeroComponent::OnPawnDataApplied()
{
    Super::OnPawnDataApplied();
    
    if (const UMyHeroPawnData* MyPawnData = Cast<UMyHeroPawnData>(PawnData))
    {
        SetupHeroMovement();
        SetupHeroAbilities();
    }
}

void UMyHeroComponent::SetupHeroMovement()
{
    if (const UMyHeroPawnData* MyPawnData = Cast<UMyHeroPawnData>(PawnData))
    {
        if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
        {
            if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
            {
                MoveComp->MaxWalkSpeed = MyPawnData->BaseSpeed;
            }
        }
    }
}

void UMyHeroComponent::SetupHeroAbilities()
{
    if (const UMyHeroPawnData* MyPawnData = Cast<UMyHeroPawnData>(PawnData))
    {
        if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
        {
            for (TSubclassOf<UGameplayAbility> AbilityClass : MyPawnData->HeroAbilities)
            {
                if (AbilityClass)
                {
                    ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
                }
            }
        }
    }
}
```

---

## 创建自定义 Experience

### 定义 ExperienceDefinition

```cpp
// MyBattleRoyaleExperience.h
#pragma once

#include "CoreMinimal.h"
#include "Experiences/LyraExperienceDefinition.h"
#include "MyBattleRoyaleExperience.generated.h"

UCLASS()
class UMyBattleRoyaleExperience : public ULyraExperienceDefinition
{
    GENERATED_BODY()
    
public:
    UMyBattleRoyaleExperience();
    
    // 大逃杀特定配置
    UPROPERTY(EditDefaultsOnly, Category = "BattleRoyale")
    int32 MaxPlayers = 100;
    
    UPROPERTY(EditDefaultsOnly, Category = "BattleRoyale")
    float MatchDuration = 1800.0f; // 30分钟
    
    UPROPERTY(EditDefaultsOnly, Category = "BattleRoyale")
    TArray<TSoftObjectPtr<UWorld>> PossibleMaps;
    
    UPROPERTY(EditDefaultsOnly, Category = "BattleRoyale")
    FScalableFloat SafeZoneShrinkTime;
};
```

### 配置 Game Features

在蓝图中配置 `GameFeaturesToEnable`：
```
- ShooterCore
- BattleRoyaleMode
- SafeZoneSystem
- LootSystem
- SpectatorSystem
```

### 自定义 Experience Action

```cpp
// MyBattleRoyaleInitAction.h
#pragma once

#include "CoreMinimal.h"
#include "GameFeatures/GameFeatureAction.h"
#include "MyBattleRoyaleInitAction.generated.h"

UCLASS()
class UMyBattleRoyaleInitAction : public UGameFeatureAction
{
    GENERATED_BODY()
    
public:
    virtual void OnGameFeatureActivating(FGameFeatureActivatingContext& Context) override;
    virtual void OnGameFeatureDeactivating(FGameFeatureDeactivatingContext& Context) override;
    
private:
    void InitializeSafeZoneSystem();
    void InitializeLootSystem();
    void RegisterGameModeCallbacks();
};
```

---

## 组件间通信模式

### 使用 GameplayMessageSubsystem

```cpp
// 定义消息
USTRUCT()
struct FLyraWeaponFireMessage
{
    GENERATED_BODY()
    
    UPROPERTY()
    TObjectPtr<APlayerState> Shooter;
    
    UPROPERTY()
    FVector FireLocation;
    
    UPROPERTY()
    FVector FireDirection;
    
    UPROPERTY()
    int32 AmmoRemaining;
};

// 发送消息
void UMyWeaponComponent::Fire()
{
    // ... 执行射击逻辑
    
    FLyraWeaponFireMessage Message;
    Message.Shooter = GetPlayerState();
    Message.FireLocation = GetOwner()->GetActorLocation();
    Message.FireDirection = GetOwner()->GetActorForwardVector();
    Message.AmmoRemaining = CurrentAmmo;
    
    UGameplayMessageSubsystem::Get(this).BroadcastMessage(
        FGameplayTag::RequestGameplayTag(FName("Lyra.Weapon.Fire")),
        Message);
}

// 接收消息
void UMyUIComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UGameplayMessageSubsystem::Get(this).RegisterListener(
        FGameplayTag::RequestGameplayTag(FName("Lyra.Weapon.Fire")),
        this,
        &UMyUIComponent::OnWeaponFired);
}

void UMyUIComponent::OnWeaponFired(FGameplayTag Channel, const FLyraWeaponFireMessage& Message)
{
    UpdateAmmoDisplay(Message.AmmoRemaining);
}
```

### 使用 GameplayTags

```cpp
// 定义标签
UENUM(BlueprintType)
enum class ELyraGameplayTag : uint8
{
    Status_Invulnerable,
    Status_Stunned,
    Ability_Type_Offensive,
    Ability_Type_Defensive,
    Event_Match_Start,
    Event_Match_End,
    // ...
};

// 检查标签
bool UMyComponent::CanTakeDamage() const
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
        return !ASC->HasMatchingGameplayTag(InvulnerableTag);
    }
    return true;
}

// 添加/移除标签
void UMyComponent::SetInvulnerable(bool bInvulnerable)
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayTag InvulnerableTag = FGameplayTag::RequestGameplayTag(FName("Status.Invulnerable"));
        
        if (bInvulnerable)
        {
            ASC->AddLooseGameplayTag(InvulnerableTag);
        }
        else
        {
            ASC->RemoveLooseGameplayTag(InvulnerableTag);
        }
    }
}
```

### 使用委托

```cpp
// 定义委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDied, APlayerState*, Victim);

UCLASS()
class UMyHealthComponent : public UActorComponent
{
    GENERATED_BODY()
    
public:
    UPROPERTY(BlueprintAssignable)
    FOnHealthChanged OnHealthChanged;
    
    UPROPERTY(BlueprintAssignable)
    FOnPlayerDied OnPlayerDied;
    
    void TakeDamage(float Damage);
    
private:
    float Health = 100.0f;
    float MaxHealth = 100.0f;
};

// 绑定委托
void UMyHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    if (UMyHealthComponent* HealthComp = GetHealthComponent())
    {
        HealthComp->OnHealthChanged.AddDynamic(this, &UMyHUDWidget::UpdateHealthBar);
        HealthComp->OnPlayerDied.AddDynamic(this, &UMyHUDWidget::ShowDeathScreen);
    }
}
```

---

## 数据驱动配置示例

### AbilitySet 配置

```cpp
UCLASS()
class UMyWeaponAbilitySet : public ULyraAbilitySet
{
    GENERATED_BODY()
    
public:
    UMyWeaponAbilitySet()
    {
        // 在蓝图中配置这些
        // GrantedAbilities.Add(...)
        // GrantedEffects.Add(...)
    }
};
```

### 曲线表配置

```cpp
// 使用曲线表配置伤害衰减
UCLASS()
class UMyDamageFalloffConfig : public UDataAsset
{
    GENERATED_BODY()
    
public:
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    UCurveFloat* DamageFalloffCurve;
    
    float GetDamageAtDistance(float Distance) const
    {
        if (DamageFalloffCurve)
        {
            return DamageFalloffCurve->GetFloatValue(Distance);
        }
        return 1.0f;
    }
};
```

### 数据表配置

```cpp
// 武器数据表
USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName WeaponName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float BaseDamage;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MagazineSize;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FireRate;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSoftObjectPtr<UStaticMesh> WeaponMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UGameplayAbility> PrimaryFireAbility;
};

// 使用数据表
void UMyWeaponManager::InitializeWeapons()
{
    if (UDataTable* WeaponTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/Weapons.Weapons")))
    {
        TArray<FWeaponData*> AllWeapons;
        WeaponTable->GetAllRows<FWeaponData>(TEXT("WeaponInit"), AllWeapons);
        
        for (FWeaponData* WeaponData : AllWeapons)
        {
            // 初始化武器
            CreateWeaponFromData(*WeaponData);
        }
    }
}
```
