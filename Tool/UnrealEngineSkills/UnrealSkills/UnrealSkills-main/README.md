# Unreal Engine Skills for Claude Code

这是一套为Unreal Engine项目设计的Claude Code自定义skills，可以自动检测项目配置、编译和测试UE项目。

## 包含的Skills

### `/ue-build` - UE编译Skill
自动检测UE引擎安装（Launcher或源码版）并编译项目。

**功能**:
- 自动检测.uproject文件和引擎路径
- 支持GUID（注册表）、路径和版本号三种引擎关联方式
- 跨平台支持（Windows、Linux、macOS）
- 智能选择编译目标
- 详细的错误报告

**用法示例**:
```
请编译项目
Build the BoidsFormation plugin
Compile in DebugGame configuration
```

### `/ue-test` - UE测试Skill
运行Unreal Engine自动化测试。

**功能**:
- 自动检测引擎和项目配置
- 列出可用测试
- 运行特定测试或测试套件
- 解析并报告测试结果
- 支持无头模式（NullRHI）和窗口模式

**用法示例**:
```
请运行所有测试
List available tests
Run BoidsFormation tests
```

## 安装到用户配置

### 方式一：复制到个人配置目录（推荐）

将skills复制到个人Claude配置目录，这样可以在所有UE项目中使用：

#### Windows
```bash
# 复制整个skills目录到用户配置
xcopy /E /I .claude\skills %USERPROFILE%\.claude\skills
```

#### Linux/macOS
```bash
# 复制整个skills目录到用户配置
cp -r .claude/skills ~/.claude/skills
```

### 方式二：保留在项目中

如果只想在当前项目中使用，skills已经在 `.claude/skills/` 目录中，无需额外操作。

## 工作原理

### 引擎检测

`detect_ue.py` 脚本会自动检测：

**用法**:
```bash
python detect_ue.py [project_path]
```

**参数**:
- `project_path`（可选）：
  - 可以是 `.uproject` 文件路径
  - 可以是包含 `.uproject` 的目录路径
  - 如果不提供，从当前工作目录搜索

**检测步骤**:

1. **查找.uproject文件**：
   - 如果提供了.uproject文件路径 → 直接使用
   - 如果提供了目录路径 → 在该目录及向上5级父目录中搜索
   - 如果未提供参数 → 从当前工作目录向上搜索

2. **读取EngineAssociation**：从.uproject中获取引擎关联信息

3. **根据关联类型定位引擎**：
   - **GUID格式** (`{XXXXXXXX-...}`)：从Windows注册表查找
     - 查找路径：`HKCU\Software\Epic Games\Unreal Engine\Builds`
     - 查找路径：`HKLM\Software\Epic Games\Unreal Engine\Builds`
     - 查找路径：`HKLM\Software\Wow6432Node\Epic Games\Unreal Engine\Builds`
   - **路径格式** (`C:/UnrealEngine`)：直接使用指定路径
   - **版本号格式** (`5.4`)：查找Epic Games Launcher安装

4. **验证引擎**：确认Build.bat/Build.sh和UnrealEditor-Cmd存在

5. **提取构建目标**：从.uproject中读取模块和插件列表

6. **读取引擎版本**：从Build.version文件读取准确的版本号

**输出JSON格式**:
```json
{
  "uproject": {
    "path": "完整的.uproject路径",
    "name": "项目名称",
    "directory": "项目目录",
    "engine_association": "引擎关联值"
  },
  "engine": {
    "version": "5.4",
    "path": "引擎根目录",
    "type": "launcher|source|custom",
    "build_tool": "Build.bat或Build.sh的完整路径",
    "editor_cmd": "UnrealEditor-Cmd的完整路径"
  },
  "targets": {
    "modules": [...],
    "plugins": [...]
  },
  "available_installations": [...]
}
```

### 支持的引擎安装类型

| 类型 | EngineAssociation格式 | 检测方式 |
|------|----------------------|---------|
| Launcher安装 | `"5.4"` | 扫描`Program Files/Epic Games/UE_*` |
| 源码构建 | `"C:/UnrealEngine"` | 直接使用路径 |
| 自定义引擎 | `"{12345678-...}"` | Windows注册表查询 |

## Skill结构

每个skill遵循Claude Code的标准结构：

```
skill-name/
├── SKILL.md              # 主skill文件（包含YAML frontmatter和Markdown指令）
├── scripts/              # 辅助脚本（可选）
│   └── detect_ue.py     # UE项目检测脚本
└── reference.md          # 参考文档（可选，未使用）
```

## SKILL.md格式

```yaml
---
name: skill-name
description: 简短描述，Claude用此判断何时使用
allowed-tools: Bash, Read, Grep  # 限制可用工具
---

# Skill内容
Markdown格式的详细指令...
```

## 自定义和扩展

### 修改检测脚本

如果需要支持其他引擎位置或平台，可以修改 `scripts/detect_ue.py`：

- `find_ue_launcher_installations()` - 添加更多Launcher搜索路径
- `find_ue_from_registry()` - 修改Windows注册表查找逻辑
- `find_ue_source_build()` - 添加自定义路径处理

### 添加新的编译配置

在SKILL.md中修改 "Step 3: Choose Build Configuration" 部分，添加你需要的配置。

### 添加新的测试过滤器

在ue-test/SKILL.md中修改 "Step 2: Determine Test Scope" 部分。

## 使用示例

### 在项目中使用（项目配置）

```bash
# 进入UE项目目录
cd /path/to/MyUEProject

# 启动Claude Code
claude

# 然后直接对话：
> 请编译项目
> 运行所有测试
```

Claude会自动：
1. 运行 `python .claude/skills/ue-build/scripts/detect_ue.py`（从当前目录搜索）
2. 检测引擎和项目配置
3. 执行编译或测试

### 在任意位置使用（用户配置）

假设skills已安装到 `~/.claude/skills/`：

```bash
# 在任意位置启动Claude Code
cd ~/Desktop
claude

# 对话时指定项目：
> 请编译 /path/to/MyUEProject
> 运行 C:/Projects/GameProject 的测试
```

Claude会：
1. 运行 `python ~/.claude/skills/ue-build/scripts/detect_ue.py /path/to/MyUEProject`
2. 使用提供的路径进行检测
3. 执行编译或测试

### 手动测试检测脚本

```bash
# 测试当前目录
python ~/.claude/skills/ue-build/scripts/detect_ue.py .

# 测试指定目录
python ~/.claude/skills/ue-build/scripts/detect_ue.py /path/to/project

# 测试指定.uproject文件
python ~/.claude/skills/ue-build/scripts/detect_ue.py /path/to/MyGame.uproject

# 查看输出（格式化JSON）
python ~/.claude/skills/ue-build/scripts/detect_ue.py . | python -m json.tool
```

## 常见问题

### Q: Skills没有被Claude识别
**A**: 确保：
1. SKILL.md文件在正确位置
2. YAML frontmatter格式正确
3. `name` 字段与目录名匹配
4. 重启Claude Code

### Q: 检测脚本找不到引擎
**A**: 手动运行检测脚本查看详细输出：

```bash
# 如果是用户配置
python ~/.claude/skills/ue-build/scripts/detect_ue.py /path/to/project

# 如果是项目配置
cd /path/to/project
python .claude/skills/ue-build/scripts/detect_ue.py
```

检查：
1. .uproject文件的EngineAssociation字段
2. Windows注册表中的引擎注册信息（GUID关联）
3. Epic Games Launcher安装路径

### Q: 用户配置的skill如何知道项目路径？
**A**: Claude应该：
1. 从用户消息中提取项目路径（"编译 /path/to/project"）
2. 使用当前工作目录（如果用户在项目目录中）
3. 询问用户项目位置（如果无法确定）

关键是运行检测脚本时传递正确的路径参数

### Q: 编译失败
**A**: Skills会报告具体错误。常见原因：
- Visual Studio未安装或版本不匹配
- 源码有编译错误
- 引擎路径不正确

### Q: 测试无法运行
**A**: 确保：
1. 项目已成功编译
2. UnrealEditor-Cmd.exe存在
3. 测试名称正确（先用 `Automation List` 列出）

## 贡献

欢迎改进这些skills！建议：
- 添加对更多平台的支持
- 改进错误检测和报告
- 添加更多UE操作（打包、烘焙等）
- 优化检测性能

## 许可

这些skills作为示例提供，你可以自由修改和分发。

## 相关资源

- [Claude Code文档](https://code.claude.com/docs)
- [Claude Code Skills指南](https://code.claude.com/docs/en/skills.md)
- [Unreal Engine自动化测试](https://docs.unrealengine.com/5.4/en-US/automation-system-user-guide-in-unreal-engine/)
- [Unreal Build Tool](https://docs.unrealengine.com/5.4/en-US/unreal-build-tool-in-unreal-engine/)
