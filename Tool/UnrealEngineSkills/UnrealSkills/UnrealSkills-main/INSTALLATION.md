# UE Skills - 用户配置安装成功 ✅

Skills已成功安装到用户配置目录！

## 安装位置

```
C:\Users\Mellos\.claude\skills\
├── README.md           # 完整使用文档
├── SUMMARY.md          # 开发总结
├── INSTALLATION.md     # 本文件
├── ue-build/          # 编译skill
│   ├── SKILL.md
│   └── scripts/
│       └── detect_ue.py
└── ue-test/           # 测试skill
    └── SKILL.md
```

## 现在可以做什么

### 在任何UE项目中使用

你现在可以在**任何UE项目**目录中启动Claude Code并使用这些skills：

```bash
# 进入任何UE项目
cd /path/to/any/UEProject

# 启动Claude
claude

# 然后对话：
> 请编译项目
> 运行测试
> 列出可用的测试
```

### 从任意位置指定项目

也可以从任何地方指定项目路径：

```bash
# 在任意目录启动Claude
cd ~/Desktop
claude

# 对话时指定项目：
> 请编译 C:/Projects/MyGame
> 运行 D:/Work/UEProject 的测试
```

## 可用的Skills

### `/ue-build` - 编译UE项目

**自动功能**:
- 检测.uproject文件
- 从注册表查找GUID引擎（自定义引擎）
- 查找Launcher安装的引擎
- 识别构建目标和配置
- 执行编译并报告结果

**示例**:
```
> 请编译当前项目
> Build the BoidsFormation plugin in Development
> 用DebugGame配置编译
```

### `/ue-test` - 运行自动化测试

**自动功能**:
- 复用引擎检测
- 列出可用测试
- 运行测试并解析结果
- 生成清晰的测试报告

**示例**:
```
> 运行所有测试
> 列出可用的测试
> Run BoidsFormation tests
> 执行 MyPlugin.SpecificTest
```

## 验证安装

测试检测脚本是否正常工作：

```bash
# 在UE项目目录中
cd "C:/Users/Mellos/Documents/Unreal Projects/BoidsDemo"
python ~/.claude/skills/ue-build/scripts/detect_ue.py .

# 或从任意位置指定路径
python ~/.claude/skills/ue-build/scripts/detect_ue.py "C:/path/to/project"
```

应该输出包含项目和引擎信息的JSON。

## 支持的引擎类型

✅ **GUID引擎** (自定义/源码构建)
- 格式: `{F09BC4BB-4A2D-0412-C987-C184B1DDEFEE}`
- 从Windows注册表读取路径
- 示例：你当前的BoidsDemo项目使用此类型

✅ **路径引擎** (直接指定)
- 格式: `C:/UnrealEngine` 或 `/home/user/UE5`
- 直接使用指定路径

✅ **版本号引擎** (Launcher安装)
- 格式: `5.3`, `5.4`
- 在Epic Games目录中查找

## 下一步

1. **阅读完整文档**: `~/.claude/skills/README.md`
2. **查看开发总结**: `~/.claude/skills/SUMMARY.md`
3. **开始使用**: 在任何UE项目中启动Claude Code

## 需要帮助？

查看 `README.md` 中的"常见问题"部分，或者直接问Claude！

---

**安装时间**: ${new Date().toISOString()}
**安装位置**: C:\Users\Mellos\.claude\skills\
**测试状态**: ✅ 已验证 - detect_ue.py正常工作
