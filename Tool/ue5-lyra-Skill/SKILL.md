---
name: ue5-lyra
description: This skill provides expert knowledge and workflows for UE5 Lyra Starter Game architecture. It should be used when working with Lyra's modular Game Feature system, GAS integration, Experience system, PawnExtensionComponent architecture, or any Lyra-specific gameplay framework code.
---

# UE5 Lyra Starter Game 架构专家

This skill provides comprehensive guidance for understanding and working with the UE5 Lyra Starter Game architecture.

## 核心设计理念

- **模块化与插件化**: 功能拆分成独立的插件 (Game Features, Modular Gameplay)，便于复用、组合、热更新和团队并行开发
- **Gameplay Ability System (GAS) 为核心**: 战斗、技能、状态、效果、属性等核心游戏逻辑高度依赖 GAS 实现
- **数据驱动**: 大量使用数据资产定义游戏行为、角色能力、UI 等，减少硬编码
- **前端与后端分离**: 清晰的 UI/表现层与游戏逻辑/数据层的分离
- **支持多种游戏模式**: 架构设计易于扩展，支持 PvP、PvE、大逃杀等多种模式

## 核心架构层次

### 1. 游戏基础框架

#### LyraGameMode / LyraGameState
- **LyraGameMode**: 游戏规则和全局状态的核心管理者，负责游戏流程（匹配、回合、胜利条件等）
- **LyraGameState**: 存储所有玩家共享的、与游戏规则相关的状态（当前游戏阶段、剩余时间、团队分数、全局事件等）

#### LyraPlayerController / LyraPlayerState
- **LyraPlayerController**: 代表玩家在服务器和客户端上的存在，处理输入、生成 Pawn、管理 UI、处理相机
- **LyraPlayerState**: 存储单个玩家的持久化状态数据（名称、分数、K/D/A、等级、选择的英雄、装备等），通常持有 AbilitySystemComponent

#### LyraPawn / LyraPawnExtensionComponent / LyraHeroComponent
- **LyraPawn**: 玩家或 AI 在游戏世界中的物理表现
- **LyraPawnExtensionComponent** (关键创新): Pawn 的核心协调器
  - 管理 Pawn 生命周期状态（初始化、重生、死亡）
  - 动态加载/卸载 Game Feature 插件提供的组件
  - 持有并管理 Pawn 的 AbilitySystemComponent
  - 协调其他关键组件的初始化和交互
- **LyraHeroComponent**: 附加到英雄角色的 Pawn 上
  - 加载和应用玩家选择的 LyraPawnData
  - 设置输入映射上下文
  - 初始化相机模式
  - 管理装备外观
  - 连接 PlayerState 和 Pawn 的桥梁

#### LyraPawnData
关键数据资产，定义特定英雄或角色类型的核心配置：
- 使用的 Character Class
- 默认的 Input Mapping Context
- 初始的 Ability Sets
- 初始的 Camera Mode
- 初始的 Equipment / Cosmetics
- 关联的 UI

### 2. Game Feature 插件系统 (核心架构支柱)

#### 概念
将离散的游戏功能（武器、技能、游戏模式、UI、交互系统）封装成独立插件。

#### 关键节点
- **GameFeaturePlugin**: UE 插件基础
- **UGameFeatureData**: 定义插件内容的核心数据资产，指定插件激活时需要加载和注册的内容

#### Actions
插件激活/停用时执行的操作：
- **UGameFeatureAction_AddComponents**: 动态将特定组件添加到符合条件的 Actor（Pawn, PlayerState, GameMode）
- **UGameFeatureAction_AddAbilities**: 动态将 GameplayAbility 和 GameplayEffect 授予符合条件的 Actor

#### 工作流程
1. Game Feature 插件被加载和激活
2. 读取其 UGameFeatureData
3. 执行其中定义的 Actions
4. AddComponents Action 检测符合条件的 Pawn，动态添加组件
5. AddAbilities Action 授予技能和效果

#### 优势
- 功能解耦：新功能作为独立插件开发
- 动态组合：运行时根据需要加载/卸载功能
- 热更新：可单独更新功能插件
- 团队协作：不同团队负责不同插件
- DLC/Mod 支持：天然支持扩展内容

### 3. 体验系统 (Experience System)

#### 概念
定义在特定游戏会话中应用哪些 Game Feature 插件和配置。

#### 关键节点
- **ULyraExperienceDefinition**: 核心数据资产，代表一种"游戏体验"
  - `GameFeaturesToEnable`: 指定此体验需要激活的 Game Feature 插件
  - `DefaultPawnData`: 指定此体验中玩家默认使用的 LyraPawnData
  - `Actions`: 体验特定的初始化操作

- **ULyraExperienceManagerComponent**: 通常存在于 GameState 上，负责加载、应用和管理当前游戏会话的 LyraExperienceDefinition

#### 工作流程
1. 游戏开始时确定要加载哪个 LyraExperienceDefinition
2. LyraExperienceManagerComponent 加载该 Experience 资产
3. Experience 加载其 GameFeaturesToEnable 列表中的所有 GameFeaturePlugin
4. 激活的 GameFeaturePlugin 执行它们自己的 Actions
5. Experience 自身也可能执行其定义的 Actions

### 4. Gameplay Ability System (GAS) 集成

#### 关键节点
- **AbilitySystemComponent**: 存在于 LyraPlayerState 和 LyraPawnExtensionComponent 上
- **LyraGameplayAbility / LyraGameplayEffect**: Lyra 自定义的基础类
- **LyraAbilitySet**: 关键数据资产，定义一组 GameplayAbility 和 GameplayEffect
- **GameFeatureAction_AddAbilities**: 动态授予 AbilitySet 中定义的能力和效果

#### 流程示例 (玩家获得武器)
1. 武器功能封装在 GameFeaturePlugin 中
2. GameFeatureData 包含 AddComponents Action，将武器组件添加到 Pawn
3. GameFeatureData 包含 AddAbilities Action，授予包含开火、换弹等技能的 AbilitySet
4. 玩家选择武器时，组件和能力被动态添加

### 5. 输入系统

#### 关键节点
- **LyraInputConfig**: 关键数据资产，将 InputAction 映射到 GameplayTag
- **LyraInputComponent**: 自定义的 UEnhancedInputComponent，将输入事件转化为触发关联 GameplayTag 的 Ability
- **InputMappingContext**: 定义具体的键位/设备绑定到 InputAction

#### 流程
1. LyraHeroComponent 应用 LyraPawnData 中指定的 InputMappingContext
2. LyraInputComponent 使用 LyraInputConfig 监听绑定的 InputAction
3. 输入触发时，找到对应的 InputTag
4. InputTag 被广播，匹配的 GameplayAbility 被尝试激活

### 6. UI 系统

- **Common UI 框架**: 深度集成用于构建健壮、可扩展、支持多平台输入和本地化的 UI
- **LyraHUD**: 游戏内 HUD 的主要管理者
- **LyraUICameraManager**: 管理与 UI 渲染相关的相机设置
- **LyraWidgetFactory / Layout Widgets**: 根据数据动态创建和布局 UI 元素
- **GameplayMessage 子系统**: UI 监听 GameplayMessage 进行更新（松耦合通信）

### 7. 资产与加载

- **Primary Data Assets**: 大量使用主数据资产定义核心游戏对象
- **Asset Manager**: 负责加载、卸载和管理主数据资产

## 关键数据资产类型

| 数据资产 | 用途 |
|---------|------|
| LyraPawnData | 定义角色模板（输入、能力、相机、外观） |
| LyraExperienceDefinition | 定义游戏体验（激活的 Game Features、默认 PawnData） |
| LyraInputConfig | 输入动作到 GameplayTag 的映射 |
| LyraAbilitySet | 技能和效果的分组 |
| GameFeatureData | 定义插件激活时的 Actions |

## 调试技巧

- 使用 GameplayDebugger（按 ` 键）查看 GAS 状态
- 使用 `ShowDebug AbilitySystem` 控制台命令
- 查看官方文档和源码注释

## 学习路径建议

1. 从 ShooterCore 开始：包含最基本的角色、武器、输入、UI
2. 追踪 Pawn 的创建流程
3. 分析简单 Ability（如跳跃或开火）
4. 理解 LyraExperienceDefinition 的加载流程

## 参考文档

详细架构说明请参考 `references/lyra-architecture.md`