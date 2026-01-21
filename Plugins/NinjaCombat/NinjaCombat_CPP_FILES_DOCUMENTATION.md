# NinjaCombat 插件 C++ 文件功能说明文档

## 目录
1. [核心模块](#核心模块)
2. [UI 模块](#ui-模块)
3. [AbilitySystem 模块](#abilitysystem-模块)
4. [Animation 模块](#animation-模块)
5. [Components 模块](#components-模块)
6. [GameFramework 模块](#gameframework-模块)
7. [Damage 模块](#damage-模块)
8. [Types 模块](#types-模块)
9. [Interfaces 模块](#interfaces-模块)
10. [Data 模块](#data-模块)
11. [Targeting 模块](#targeting-模块)

---

## 核心模块

### NinjaCombat.cpp
插件主模块文件,定义了 `FNinjaCombatModule` 类,负责插件的启动和关闭。这是 Unreal Engine 插件的标准入口点。

### NinjaCombatSubsystem.cpp
战斗子系统,提供全局战斗相关的服务:
- 初始化 AbilitySystemGlobals
- 管理随机数生成器 (RandomStream)
- 提供随机数生成功能 `GetRandomFloatInRange()`

### NinjaCombatSettings.cpp
战斗系统设置类,定义全局配置:
- AbilitySystem 代理类配置 (ActorInfoProxyClass, EffectContextProxyClass)
- Motion Warping 默认启用设置
- 近战扫描配置 (扫描通道、扫描类、调试持续时间)
- 投射物配置 (Socket 名称、通道、请求类)
- 伤害注册表大小配置

### NinjaCombatFunctionLibrary.cpp
战斗系统功能库,提供静态辅助函数:
- 获取各种战斗管理器组件 (ComboManager, DamageManager, DefenseManager, MotionWarping, MovementManager, PhysicalAnimation, TargetManager, WeaponManager)
- 获取近战和远程脚本接口
- 分解伤害结构体 `BreakDamageStruct()`
- 执行 Gameplay Cue (本地执行、添加、移除)
- 从 EffectContext 创建 Gameplay Cue 参数
- 生成血迹贴花 `SpawnBloodDecal()`

### NinjaCombatTags.cpp
定义所有战斗相关的 Gameplay Tags:
- 能力标签 (Attack, Block, Death, Evade, HitReaction, Stagger, TargetLock)
- 组件标签 (ForwardReference, HealthWidget, Mesh, MeleeScanSource, ProjectileSource)
- 数据标签 (CastHits, ComboCounter, CriticalHitChance, CriticalHitMultiplier, Damage, PoiseConsumption)
- 效果标签 (Damage 类型: Blocked, Breaker, Critical, Fatal, LastStand, Mitigated, Melee, Ranged, Stagger)
- 事件标签 (Cast, Combo, Damage, Death, Invulnerability, MeleeScan, Projectile, Target)
- 状态标签 (Blocking, ComboWindow, Dead, Invulnerable, LockedOnTarget)
- Gameplay Cue 标签 (Death, Hit)

### NinjaCombatDelegates.cpp
定义战斗相关的委托,用于事件广播和回调。

---

## UI 模块

### NinjaCombatUI.cpp
UI 模块主文件,定义 `FNinjaCombatUIModule` 类,负责 UI 插件的启动和关闭。

### NinjaCombatWidgetComponent.cpp
战斗 UI 组件,管理战斗相关的用户界面:
- 初始化 Widget 并设置战斗 Actor
- 处理 Gameplay Cue 事件并传递给 Widget

### NinjaCombatBaseWidget.cpp
战斗 UI 基类 Widget:
- 设置战斗 Actor `SetCombatActor()`
- 处理 Gameplay Cue `HandleGameplayCue()`
- 获取战斗 Actor 和 AbilitySystemComponent
- 遍历子 Widget 并传递事件

### NinjaCombatBaseTransientWidget.cpp
临时显示的 Widget 基类:
- 控制显示和隐藏逻辑
- 支持自动隐藏功能 (bHideWhenUnchanged)
- 管理显示持续时间 (DisplayDuration)
- 使用定时器控制显示/隐藏

### NinjaCombatBaseOverheadBarWidget.cpp
头顶状态条 Widget 基类 (血条、体力条、魔法条):
- 绑定到 AbilitySystemComponent 的属性变化
- 监听当前值、最大值、附加值、百分比值的变化
- 更新状态条显示
- 处理 Gameplay Cue (致命伤害时隐藏,受伤时显示)
- 支持在致命伤害后自动隐藏

### NinjaCombatHealthBarWidget.cpp
血条 Widget,继承自头顶状态条:
- 绑定到 Health 属性
- 绑定到 MaxHealth, MaxHealthAdd, MaxHealthPercent, MaxHealthTotal 属性

### NinjaCombatStaminaBarWidget.cpp
体力条 Widget,继承自头顶状态条:
- 绑定到 Stamina 属性
- 绑定到 MaxStamina, MaxStaminaAdd, MaxStaminaPercent, MaxStaminaTotal 属性

### NinjaCombatMagicBarWidget.cpp
魔法条 Widget,继承自头顶状态条:
- 绑定到 Magic 属性
- 绑定到 MaxMagic, MaxMagicAdd, MaxMagicPercent, MaxMagicTotal 属性

### NinjaCombatDamageInfoWidget.cpp
伤害信息显示 Widget:
- 显示伤害数值
- 支持伤害累积 (bAccumulateDamage)
- 处理 Gameplay Cue 并更新伤害显示
- 区分暴击、破防、致命伤害
- 隐藏时重置累积伤害

---

## AbilitySystem 模块

### NinjaCombatGameplayAbility.cpp
战斗能力基类,所有战斗能力的父类:
- 提供获取各种战斗管理器组件的方法
- 初始化事件任务 `InitializeEventTask()`
- 完成延迟任务 `FinishLatentTasks()`
- 添加调试消息 `AddDebugMessage()`
- 支持调试模式 (bEnableDebug)

### NinjaCombatMontageAbility.cpp
蒙太奇能力基类,用于播放动画的能力:
- 播放动画蒙太奇
- 支持 Motion Warping (动作变形)
- 管理动画提供者 (AnimationProvider)
- 管理变形目标提供者 (WarpTargetProvider)
- 处理动画完成、取消事件
- 支持等待变形目标后再播放动画

### NinjaCombatAttributeSet.cpp
战斗属性集,定义所有战斗相关属性:
- **生命属性**: Health, MaxHealth, MaxHealthAdd, MaxHealthPercent, MaxHealthTotal, HealthRegenRate
- **体力属性**: Stamina, MaxStamina, MaxStaminaAdd, MaxStaminaPercent, MaxStaminaTotal, StaminaRegenRate
- **魔法属性**: Magic, MaxMagic, MaxMagicAdd, MaxMagicPercent, MaxMagicTotal, MagicRegenRate
- **战斗属性**: BaseDamage, CriticalHitChance, CriticalHitMultiplier
- **格挡属性**: BlockChance, BlockMitigation, BlockAngle, BlockLimit, BlockStaminaCostRate
- **防御属性**: DefenseChance, DefenseMitigation, DefenseLimit, DefenseStaminaCostRate
- **护甲属性**: ArmorReduction
- **架势属性**: Poise, MaxPoise
- **最后一口气属性**: LastStandCount, LastStandHealthPercent
- 处理属性变化 `PreAttributeChange()`
- 处理 GameplayEffect 执行 `PostGameplayEffectExecute()`
- 处理伤害计算和应用 `HandleIncomingDamage()`, `ApplyDamage()`
- 网络复制所有属性

### NinjaCombatAbilitySystemGlobals.cpp
AbilitySystem 全局配置,扩展 GAS 的全局设置。

### NinjaCombatGameplayCueManager.cpp
Gameplay Cue 管理器,处理战斗相关的 Gameplay Cue 事件。

#### Abilities 子目录

### CombatAbility_Attack.cpp
攻击能力,处理近战和远程攻击:
- 支持近战攻击 (bIsMeleeAttack) 和远程攻击 (bIsRangedAttack)
- 获取连击计数器 `GetComboCounter()`
- 初始化近战扫描任务 `InitializeMeleeScanTask()`
- 初始化投射物任务 `InitializeProjectileTask()`
- 处理近战扫描事件 (开始/停止)
- 处理投射物发射事件
- 处理近战目标命中 `HandleMeleeScanTargetsReceived()`
- 处理投射物发射 `HandleProjectileLaunched()`
- 应用伤害值到 GameplayEffectSpec
- 获取近战和投射物的 GameplayEffect 类和等级

### CombatAbility_Block.cpp
格挡能力:
- 检查是否正在格挡 `IsBlocking()`
- 激活时应用格挡效果 (BlockingEffectClass)
- 结束时移除格挡效果
- 支持移除预存在的格挡效果

### CombatAbility_Evade.cpp
闪避能力:
- 使用方向性闪避动画提供者
- 监听无敌事件 (开始/结束)
- 授予和撤销无敌效果 (InvulnerabilityEffectClass)
- 使用方向性闪避动画提供者

### CombatAbility_Death.cpp
死亡能力:
- 禁用 Motion Warping
- 由死亡事件触发 (Tag_Combat_Event_Death)
- 播放死亡动画
- 完成死亡流程 `FinishDying()`
- 取消所有能力并清理 AbilitySystem

### CombatAbility_HitReaction.cpp
受击反应能力,处理受到攻击时的反应动画。

### CombatAbility_Stagger.cpp
硬直能力,处理架势耗尽时的硬直状态。

### CombatAbility_Cast.cpp
施法能力,处理技能施放逻辑。

### CombatAbility_Combo.cpp
连击能力,处理连击系统逻辑。

### CombatAbility_TargetLock.cpp
目标锁定能力,处理目标锁定系统。

#### Effects 子目录

### CombatEffect_InitializeAttributes.cpp
初始化属性效果,初始化角色的战斗属性。

### CombatEffect_ReplenishHealth.cpp
恢复生命效果,恢复角色的生命值。

### CombatEffect_ReplenishStamina.cpp
恢复体力效果,恢复角色的体力值。

### CombatEffect_ReplenishMagic.cpp
恢复魔法效果,恢复角色的魔法值。

### CombatEffect_ReplenishPoise.cpp
恢复架势效果,恢复角色的架势值。

### CombatEffect_Damage.cpp
伤害效果,应用伤害到目标。

### CombatEffect_MeleeHit.cpp
近战命中效果,处理近战攻击的命中逻辑。

### CombatEffect_RangedHit.cpp
远程命中效果,处理远程攻击的命中逻辑。

### CombatEffect_Blocking.cpp
格挡效果,应用格挡状态和属性修改。

### CombatEffect_ComboWindow.cpp
连击窗口效果,管理连击时间窗口。

### CombatEffect_ConsumePoise.cpp
消耗架势效果,减少目标的架势值。

### CombatEffect_Invulnerability.cpp
无敌效果,使角色在指定时间内免疫伤害。

### CombatEffect_LastStand.cpp
最后一口气效果,当生命值归零时提供短暂的生存机会。

### CombatEffect_LockedOnTarget.cpp
目标锁定效果,管理目标锁定状态。

#### Executions 子目录

### CombatExecution_Damage.cpp
伤害执行计算,执行伤害数值的计算逻辑。

#### Calculations 子目录

### CombatCalculation_LastStandHealth.cpp
最后一口气生命计算,计算最后一口气状态下的生命值。

#### Tasks 子目录

### AbilityTask_ScanMeleeTarget.cpp
近战目标扫描任务:
- 持续扫描近战范围内的目标
- 支持多个扫描源
- 每帧执行扫描并广播结果
- 支持调试模式

### AbilityTask_SpawnProjectile.cpp
生成投射物任务:
- 根据投射物请求生成投射物
- 支持多个投射物请求
- 广播投射物发射事件

### AbilityTask_SpawnCast.cpp
生成施法任务,处理技能施放的投射物生成。

### AbilityTask_TrackDistance.cpp
距离追踪任务,追踪与目标的距离。

#### Providers 子目录

### NinjaCombatAnimationProvider.cpp
动画提供者基类,为能力提供动画资源。

### NinjaCombatMotionWarpingTargetProvider.cpp
Motion Warping 目标提供者,提供动作变形的目标位置。

#### Animation 子目录

### AnimationProvider_HitReaction.cpp
受击反应动画提供者,提供受击反应的动画。

### AnimationProvider_DirectionalEvade.cpp
方向性闪避动画提供者,根据输入方向提供闪避动画。

#### Target 子目录

### TargetProvider_TargetingSystem.cpp
目标系统提供者,使用目标系统提供目标。

#### Proxies 子目录

### NinjaCombatEffectContextProxy.cpp
EffectContext 代理,扩展 GameplayEffectContext 以存储战斗相关数据。

### NinjaCombatAbilityActorInfoProxy.cpp
AbilityActorInfo 代理,扩展 AbilityActorInfo 以访问战斗组件。

#### Types 子目录

### FNinjaCombatGameplayEffectContext.cpp
战斗 GameplayEffectContext,存储伤害、减免等战斗相关数据。

### FNinjaCombatGameplayAbilityActorInfo.cpp
战斗 GameplayAbilityActorInfo,扩展 ActorInfo 以支持战斗系统。

#### Combo 子目录

### STEvaluator_ComboState.cpp
连击状态评估器,评估连击状态。

### STTask_ActivateComboAbility.cpp
激活连击能力任务,激活连击能力。

---

## Animation 模块

### NinjaCombatAnimNotify.cpp
动画通知基类,所有战斗动画通知的基类,设置 Ninja Bear 绿色标识。

### NinjaCombatAnimNotifyState.cpp
动画通知状态基类,所有持续动画通知的基类,设置 Ninja Bear 绿色标识。

#### States 子目录

### AnimNotifyState_WeaponTrail.cpp
武器轨迹通知状态:
- 在武器上显示 Niagara 轨迹效果
- 使用 StartSocket 和 EndSocket 定义轨迹起点和终点
- 更新轨迹参数 (TrailBeginParameterName, TrailEndParameterName)
- 支持多个轨迹组件
- 在通知结束时停用轨迹

### AnimNotifyState_MeleeScan.cpp
近战扫描通知状态:
- 在动画播放期间执行近战扫描
- 支持从武器或角色进行扫描
- 支持多种扫描模式 (LineTrace, BoxSweep, CapsuleSweep, SphereSweep)
- 创建近战扫描实例并广播扫描事件
- 处理扫描开始和停止事件

### AnimNotifyState_ComboWindow.cpp
连击窗口通知状态,管理连击时间窗口。

### AnimNotifyState_Invulnerability.cpp
无敌通知状态,管理无敌状态的开始和结束。

#### Notifies 子目录

### AnimNotify_LaunchProjectile.cpp
发射投射物通知,触发投射物发射事件。

### AnimNotify_Cast.cpp
施法通知,触发技能施放事件。

---

## Components 模块

### NinjaCombatBaseComponent.cpp
战斗组件基类,所有战斗组件的父类:
- 管理 AbilitySystemComponent 的绑定
- 提供网络权限检查 (OwnerHasAuthority, OwnerIsLocallyControlled)
- 检查 GameplayTag
- 提供随机数生成功能 `Roll()`
- 管理定时器

### NinjaCombatDamageManagerComponent.cpp
伤害管理组件:
- 管理伤害接收和死亡状态
- 维护伤害列表 (DamageTakenList)
- 计算伤害 `CalculateDamage()`
- 注册伤害接收 `RegisterDamageReceived()`
- 处理伤害接收 `HandleDamageReceived()`
- 广播伤害 Gameplay 事件
- 播放伤害 Gameplay Cue
- 管理死亡流程 (StartDying, FinishDying)
- 获取死亡能力 `GetDeathAbility()`
- 执行伤害处理器 `ExecuteDamageHandlers()`
- 支持伤害接收和死亡委托绑定

### NinjaCombatDefenseManagerComponent.cpp
防御管理组件,管理格挡和防御逻辑。

### NinjaCombatComboManagerComponent.cpp
连击管理组件,管理连击计数和连击状态。

### NinjaCombatWeaponManagerComponent.cpp
武器管理组件,管理武器装备和切换。

### NinjaCombatTargetManagerComponent.cpp
目标管理组件,管理目标锁定和目标选择。

### NinjaCombatMovementManagerComponent.cpp
移动管理组件,管理战斗相关的移动逻辑。

### NinjaCombatMotionWarpingComponent.cpp
动作变形组件,管理动作变形目标和参数。

### NinjaCombatPhysicalAnimationComponent.cpp
物理动画组件,管理物理动画效果。

---

## GameFramework 模块

### NinjaCombatProjectileActor.cpp
投射物 Actor:
- 支持弹跳和追踪
- 集成目标系统 (TargetingSystem)
- 处理碰撞和命中
- 应用伤害 GameplayEffect
- 播放命中和销毁特效
- 支持调试可视化
- 管理投射物生命周期

### NinjaCombatWeaponActor.cpp
武器 Actor:
- 存储武器标签 (WeaponTags)
- 提供投射物类信息
- 支持网络复制

### NinjaCombatMeleeScan.cpp
近战扫描类:
- 执行近战范围扫描
- 支持多种扫描形状 (Line, Box, Capsule, Sphere)
- 管理扫描 Socket 位置
- 过滤已命中的目标
- 支持调试可视化
- 提供扫描配置和结果

### NinjaCombatProjectileRequest.cpp
投射物请求类,定义投射物生成的参数和配置。

### NinjaCombatCastActor.cpp
施法 Actor,处理技能施放的 Actor。

---

## Damage 模块

### NinjaCombatDamageHandler.cpp
伤害处理器基类:
- 定义伤害处理接口
- 提供伤害类型判断 (近战/远程)
- 提供权限判断 (权威/本地控制)
- 提供目标和来源判断

#### Handlers 子目录

### DamageHandler_Widget.cpp
Widget 伤害处理器,处理伤害相关的 UI 更新。

### DamageHandler_PhysicalAnimation.cpp
物理动画伤害处理器,处理伤害触发的物理动画效果。

### DamageHandler_DamageSense.cpp
伤害感知处理器,处理伤害感知系统的响应。

### DamageHandler_Cosmetics.cpp
特效伤害处理器,处理伤害相关的视觉和听觉特效。

---

## Types 模块

### FDamageEntry.cpp
伤害条目结构体:
- 存储单次伤害的完整信息
- 包含时间戳、EffectContext、标签
- 判断伤害类型 (暴击、格挡、破防、致命、硬直)
- 获取伤害来源和造成者
- 获取应用伤害值
- 生成 GameplayCue 参数
- 支持网络序列化

### FDamageList.cpp
伤害列表结构体:
- 管理多个伤害条目
- 注册伤害接收
- 维护固定大小的伤害历史
- 处理复制后的广播
- 支持网络增量序列化

---

## Interfaces 模块

### CombatSystemInterface.cpp
战斗系统接口,定义战斗系统的核心接口。

### CombatMeleeInterface.cpp
近战接口,定义近战攻击相关的接口。

### CombatRangedInterface.cpp
远程接口,定义远程攻击相关的接口。

### CombatProjectileInterface.cpp
投射物接口,定义投射物相关的接口。

### CombatProjectileProviderInterface.cpp
投射物提供者接口,定义投射物生成的接口。

### CombatDamageModifierInterface.cpp
伤害修改器接口,定义伤害修改的接口。

#### Components 子目录

### CombatTargetManagerInterface.cpp
目标管理器接口,定义目标管理的接口。

---

## Data 模块

### NinjaCombatDamageHandlerData.cpp
伤害处理器数据,配置伤害处理器的集合。

### NinjaCombatComboSetupData.cpp
连击设置数据,配置连击系统的参数。

---

## Targeting 模块

### TargetingFilterTask_Dead.cpp
死亡过滤任务,过滤已死亡的目标。

### TargetingFilterTask_Facing.cpp
朝向过滤任务,根据朝向过滤目标。

### TargetingFilterTask_ProjectileHit.cpp
投射物命中过滤任务,过滤投射物可命中的目标。

---

## 总结

NinjaCombat 插件是一个完整的战斗系统,包含以下核心功能:

1. **AbilitySystem 集成**: 基于 GAS 的能力系统,支持各种战斗能力
2. **伤害系统**: 完整的伤害计算、应用、反馈系统
3. **UI 系统**: 血条、体力条、魔法条、伤害信息显示
4. **动画系统**: 武器轨迹、近战扫描、受击反应等动画通知
5. **组件系统**: 模块化的战斗组件,易于扩展和组合
6. **投射物系统**: 支持弹跳、追踪、目标系统
7. **连击系统**: 支持连击计数和连击窗口
8. **格挡和防御系统**: 完整的格挡和防御机制
9. **架势系统**: 硬直和破防机制
10. **目标锁定系统**: 目标选择和锁定功能

所有模块都支持网络复制,适合多人游戏开发。
