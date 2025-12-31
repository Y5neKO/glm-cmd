<p align="center">
  <a href="README_en.md">English</a> | 简体中文
</p>

# GLM-CMD

自然语言转命令工具 - 用自然语言描述需求，自动生成 shell 命令

## 主要功能

- **自然语言输入** - 用中文、英文等自然语言描述需求，自动生成对应命令
- **思考过程可视化** - 实时显示 AI 推理过程，了解命令生成逻辑
- **智能命令生成** - 将自然语言转换为可执行的 shell 命令
- **流式输出** - 实时显示 AI 思考过程，响应更快，体验更流畅
- **长时间推理支持** - 支持 60 秒以上的深度推理，不受超时限制
- **对话记忆** - 记住最近的对话历史，支持连续提问和上下文理解
- **跨平台支持** - 支持 Linux、macOS、Windows (WSL)
- **配置文件** - 支持 `~/.glm-cmd/config.ini` 配置
- **一键执行** - 生成后确认即可执行

## 快速开始

### 1. 安装依赖

> **提示**: 详细的依赖安装指南请参阅 [DEPENDENCIES.md](DEPENDENCIES.md)

**macOS:**
```bash
brew install curl cjson
```

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential libcurl4-openssl-dev libcjson-dev
```

**CentOS/RHEL:**
```bash
sudo yum groupinstall "Development Tools"
sudo yum install libcurl-devel cjson-devel
```

**Arch Linux:**
```bash
sudo pacman -S base-devel curl cjson
```

### 2. 编译安装

```bash
# 克隆仓库
git clone https://github.com/Y5neKO/glm-cmd.git
cd glm-cmd

# 编译
make

# 安装（可选）
sudo make install
```

**Windows 用户**：请使用 WSL（Windows Subsystem for Linux）环境，按照 Linux 方式编译。

### 3. 初始化配置

```bash
# 运行初始化向导
./glm-cmd --init
```

按提示输入：
1. **API Key** - 从 [智谱AI开放平台](https://bigmodel.cn) 获取
2. **端点选择** - 推荐 Coding 端点（命令生成专用）
3. **模型名称** - 如 `glm-4.7`、`glm-4-plus` 等
4. **温度参数** - 控制随机性，默认 0.7
5. **最大 tokens** - 响应长度，默认 2048
6. **超时时间** - 请求超时，默认 30秒

### 4. 使用示例

```bash
# 中文输入
glm-cmd "查找当前目录下所有大于100MB的文件并按大小排序"
glm-cmd "查看占用8080端口的进程"
glm-cmd "撤销最后一次提交但保留修改"

# 英文输入
glm-cmd "find all files larger than 100MB in current directory"
glm-cmd "check which process is using port 8080"
glm-cmd "undo last commit but keep changes"
```

## 工作原理

1. 输入自然语言需求描述（中文、英文等）
2. 检测系统信息（操作系统、Shell、架构等）
3. 调用智谱AI API生成命令
4. 展示生成逻辑（可选）
5. 确认后执行命令

## 配置说明

### 配置文件位置

默认位置：`~/.glm-cmd/config.ini`

自定义位置：
```bash
export GLM_CMD_CONFIG="/path/to/config.ini"
```

### 配置项说明

**必需配置：**

```ini
# API密钥（从 https://bigmodel.cn 获取）
api_key="your_api_key_here"

# API端点（必须配置）
# 推荐使用Coding端点（专门用于命令生成）
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
```

**可选配置：**

```ini
# 模型名称（自由输入）
model="glm-4.7"           # 标准模型
# model="glm-4-plus"      # 增强模型
# model="glm-4-flash"     # 快速模型
# model="glm-4-air"       # 轻量级模型

# 用户自定义提示词（前置到每次输入）
# user_prompt="请用最简洁的命令"

# 对话记忆功能
memory_enabled=true       # 启用对话记忆（默认false）
memory_rounds=5           # 记住最近5轮对话（默认5）

# 流式输出功能
stream_enabled=true       # 启用流式输出（默认true）

# 温度参数（0.0-2.0，默认0.7）
temperature=0.7

# 最大token数（默认8192，支持详细推理过程）
max_tokens=8192

# 超时时间（秒，默认30）
timeout=30
```

**提示**：使用 `--verbose` 或 `-V` 参数启用详细输出。

### 端点选择说明

**1. Coding端点（推荐）：**
```
https://open.bigmodel.cn/api/coding/paas/v4
```
- 专用于命令生成和代码推荐
- 适合开发、运维场景
- 命令准确率更高

**2. 标准端点：**
```
https://open.bigmodel.cn/api/paas/v4
```
- 通用自然语言任务
- 文本处理和分析

## 使用示例

### 文件操作

```bash
# 查找大文件
glm-cmd "查找当前目录下所有大于100MB的文件"

# 批量重命名
glm-cmd "将所有JPG文件重命名为小写"

# 合并文件
glm-cmd "合并所有txt文件到一个文件"
```

### 进程管理

```bash
# 查看端口占用
glm-cmd "查看占用8080端口的进程"

# 杀死进程
glm-cmd "杀死所有名为python的进程"

# 查看进程资源占用
glm-cmd "按内存使用率排序显示所有进程"
```

### 系统维护

```bash
# 清理临时文件
glm-cmd "清理7天前的临时文件"

# 磁盘空间检查
glm-cmd "显示各个目录的磁盘占用情况"

# 网络测试
glm-cmd "测试到baidu.com的连通性"
```

### Git操作

```bash
# 撤销提交
glm-cmd "撤销最后一次提交但保留修改"

# 查看历史
glm-cmd "查看最近3次提交的详细信息"

# 分支管理
glm-cmd "删除所有已合并的本地分支"
```

## 高级功能

### 思考过程可视化

GLM-CMD 现在支持实时显示 AI 的思考过程 (reasoning_content)，让您了解命令是如何生成的。

**工作原理：**

GLM-CMD 使用智谱 AI Coding 端点时，API 会返回两种内容：
- **思考过程** (reasoning_content): AI 分析需求、构建命令的逻辑推理
- **最终回答** (content): 经过思考后生成的最终命令和说明

**视觉效果：**

```
[* Thinking Process]
用户想要查找当前目录下所有大于100MB的文件...
需要使用 find 命令，参数 -type f 表示查找文件...
-size +100M 表示大于100MB...
-exec ls -lh {} \; 用于显示详细文件信息...

[+] Generated Answer
要查找当前目录下所有大于100MB的文件并显示详细信息，可以使用以下命令：

```bash
find . -type f -size +100M -exec ls -lh {} \;
```

[Command] find . -type f -size +100M -exec ls -lh {} \;
```

- **青色标题** `[* Thinking Process]`: 思考过程开始
- **灰色文本**: 实时显示的推理内容
- **绿色标题** `[+] Generated Answer`: 最终回答开始
- **黄色文本**: 最终回答内容（包含命令说明）
- **绿色命令**: 最终提取的可执行命令

**配置：**

思考过程可视化功能默认启用，需要使用智谱 AI Coding 端点：

```ini
# 使用 Coding 端点（支持思考过程）
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"

# 流式输出（默认启用，用于实时显示）
stream_enabled=true
```

### 长时间推理支持

GLM-CMD 现在支持长时间深度推理，不受传统超时限制。

**技术实现：**

传统超时机制 (`CURLOPT_TIMEOUT`) 限制整个请求的总时长，这会导致深度思考过程被错误中断。GLM-CMD 改用**数据流超时**机制：

```ini
# 超时配置（基于数据流）
timeout=30  # 如果 30 秒内没有接收到任何数据，才判定为超时
```

**工作原理：**

- **传统超时**: 30秒后无论是否在接收数据都会中断
- **数据流超时**: 只要持续接收数据就不会超时

这意味着即使 AI 思考 60 秒、120 秒甚至更长时间，只要持续返回数据，请求就不会被中断。

**安全网机制：**

为了防止真正的死锁，GLM-CMD 设置了 10 倍于配置时间的总超时作为安全网：

```ini
timeout=30   # 数据流超时：30秒无数据
# 实际总超时：300秒 (30 * 10)
```

**配置示例：**

```ini
# 默认配置（推荐）
timeout=30

# 复杂任务（允许更长的静默期）
timeout=60

# 简单任务（快速失败）
timeout=15
```

**适用场景：**

- 复杂命令生成（需要多步推理）
- 大规模文件操作分析
- 复杂的正则表达式构建
- 多步骤系统管理任务

### 流式输出功能

流式输出功能允许 GLM-CMD 实时显示 AI 的响应，无需等待完整生成。

**启用流式输出：**

在配置文件中添加（默认已启用）：
```ini
stream_enabled=true
```

**工作原理：**

- **启用时**：AI 响应逐字/逐句实时显示，体验更流畅
- **禁用时**：等待完整响应后一次性显示，适合脚本使用

**对比示例：**

```bash
# 流式输出（stream_enabled=true）
$ glm-cmd "列出大文件"
[*] Processing your request...

[* Thinking Process]
用户想要查找大文件...
(思考过程实时显示)

[+] Generated Answer
要查找当前目录下所有大于100MB的文件...
(最终回答实时显示)

[Command] find . -type f -size +100M -exec ls -lh {} \;

# 非流式输出（stream_enabled=false）
$ glm-cmd "列出大文件"
[*] Processing your request...
[等待完整响应...]

[*] Thinking Process
要查找当前目录下所有大于100MB的文件...
[一次性显示]

[Command]
find . -type f -size +100M -exec ls -lh {} \;
```

### 对话记忆功能

对话记忆功能允许 GLM-CMD 记住最近的对话历史，支持连续提问和上下文理解。

**启用记忆功能：**

在配置文件中添加：
```ini
memory_enabled=true
memory_rounds=5
```

**使用示例：**

```bash
# 第一次查询
glm-cmd "列出当前目录的文件"
# AI 生成: ls

# 第二次查询（AI 记住了上一次的对话）
glm-cmd "只看前3个"
# AI 理解上下文，生成: ls | head -3

# 第三次查询
glm-cmd "按文件大小排序"
# AI 继续理解上下文，生成: ls | sort -k5 -hr
```

**工作原理：**

1. **持久化存储**: 对话历史保存在 `~/.glm-cmd/history.json`
2. **FIFO 机制**: 超过设定轮数时自动删除最旧的记录
3. **API 集成**: 历史记录作为标准的 messages 数组发送给 AI
4. **上下文理解**: AI 能理解指代关系（如"它"、"那个"）

**查看调试信息：**

```bash
glm-cmd -V "列出文件"
```

输出：
```
[DEBUG] Conversation History: 1 rounds
[DEBUG] Adding 1 rounds of conversation history to API request
[DEBUG] History[0]: User='列出当前目录的文件'
```

**配置参数：**

- `memory_enabled`: 是否启用记忆功能（`true`/`false`）
- `memory_rounds`: 保存的对话轮数（默认 5）

**注意事项：**

- 记忆功能只在 API 请求成功后保存
- 历史文件格式为标准 JSON，可手动查看或编辑
- 使用 verbose 模式可以查看记忆使用情况

### 查看系统信息

```bash
glm-cmd --info
```

显示内容：
- 操作系统信息
- Shell类型和版本
- 当前配置参数
- API Key（部分隐藏）

### 对话历史管理

GLM-CMD 支持查看和管理对话历史记录，方便了解之前的查询和 AI 的响应。

**查看对话历史：**

```bash
# 显示所有保存的对话历史
glm-cmd --history

# 或使用短选项
glm-cmd -H
```

输出示例：
```
Conversation History (5 rounds):
─────────────────────────────────────────

[Round 1]
User:      查看当前目录所有文件
Assistant: 用户需要查看当前目录下的"所有文件"...
Command: ls -la

[Round 2]
User:      不看隐藏文件
Assistant: 用户明确要求"不看隐藏文件"...
Command: ls -l

─────────────────────────────────────────
```

**清除对话历史：**

```bash
# 清除所有对话历史
glm-cmd --clear-history

# 或使用短选项
glm-cmd -c
```

**使用场景：**

1. **查看历史**：回顾之前的对话，了解 AI 的响应模式
2. **清除历史**：当对话内容过多或需要重新开始时清除
3. **隐私保护**：清除包含敏感信息的对话记录
4. **调试分析**：查看历史记录，验证记忆功能是否正常工作

**注意事项：**

- 对话历史仅在启用 `memory_enabled=true` 时才会保存
- 历史记录保存在 `~/.glm-cmd/history.json` 文件中
- 清除历史是永久性操作，无法恢复
- 查看历史不需要 API 请求，可离线使用

### 详细输出模式

```bash
glm-cmd -V "查找大文件"
```

显示详细的API请求和响应信息，方便调试。

### 环境变量覆盖

配置文件可以被环境变量覆盖：

```bash
# 临时使用不同模型
export GLM_CMD_MODEL="glm-4-plus"
glm-cmd "查找大文件"

# 临时使用不同端点
export GLM_CMD_ENDPOINT="https://open.bigmodel.cn/api/paas/v4"
glm-cmd "翻译这段文字"
```

## 模型选择建议

| 模型 | 特点 | 适用场景 |
|------|------|----------|
| **glm-4.7** | 标准模型，性能均衡 | 日常使用 |
| **glm-4-plus** | 能力更强 | 复杂命令 |
| **glm-4-flash** | 响应快速 | 简单命令 |
| **glm-4-air** | 轻量级 | 资源受限环境 |

## 命令行参数

```bash
glm-cmd [OPTIONS] "query"

Options:
  -h, --help          显示帮助信息
  -v, --version       显示版本信息
  -V, --verbose       启用详细输出
  -i, --info          显示系统信息
  -I, --init          运行初始化向导
  -H, --history       显示对话历史
  -c, --clear-history 清除对话历史
```

## 故障排除

### 问题：API Key未配置

**错误信息：**
```
Error: API Key not configured
```

**解决方法：**
```bash
# 方式1：运行初始化向导
glm-cmd --init

# 方式2：手动创建配置文件
mkdir -p ~/.glm-cmd
cat > ~/.glm-cmd/config.ini << EOF
api_key="your_api_key_here"
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
EOF
```

### 问题：配置文件未生效

**检查方法：**
```bash
# 查看配置文件是否存在
ls -la ~/.glm-cmd/config.ini

# 查看当前配置
glm-cmd --info
```

**解决方法：**
```bash
# 确保文件权限正确
chmod 600 ~/.glm-cmd/config.ini

# 检查配置文件格式
cat ~/.glm-cmd/config.ini
```

### 问题：编译错误

**错误：找不到 libcurl**

**解决方法：**
```bash
# macOS
brew install curl

# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# CentOS/RHEL
sudo yum install libcurl-devel
```

### 问题：命令生成失败

**检查步骤：**
1. 验证API Key是否正确
2. 检查网络连接
3. 确认端点配置正确
4. 启用详细模式查看日志

```bash
glm-cmd -V "测试"
```

## 常用场景速查

| 需求 | 命令示例 |
|------|----------|
| 查找大文件 | `glm-cmd "查找当前目录下所有大于100MB的文件"` |
| 端口查看 | `glm-cmd "查看占用8080端口的进程"` |
| 批量重命名 | `glm-cmd "将所有文件名改为小写"` |
| 文件合并 | `glm-cmd "合并所有txt文件到一个文件"` |
| 进程杀除 | `glm-cmd "杀死所有名为python的进程"` |
| 磁盘清理 | `glm-cmd "清理7天前的临时文件"` |
| 权限修改 | `glm-cmd "递归设置当前目录权限为755"` |
| 网络测试 | `glm-cmd "测试到baidu.com的连通性"` |
| Git操作 | `glm-cmd "撤销最后一次提交但保留修改"` |
| 日志查看 | `glm-cmd "查看最后100行错误日志"` |

## 安全建议

1. **保护API Key**
   ```bash
   # 设置配置文件权限为仅用户可读写
   chmod 600 ~/.glm-cmd/config.ini
   ```

2. **避免提交配置文件**
   ```bash
   # 添加到 .gitignore
   echo ".glm-cmd/" >> .gitignore
   echo "config.ini" >> .gitignore
   ```

3. **定期更换API Key**
   - 建议每3-6个月更换一次
   - 如有泄露嫌疑立即更换

4. **谨慎执行命令**
   - 生成后先查看命令内容
   - 确认无误再执行
   - 特别是删除、修改等危险操作

## 配置示例

### 基础配置

**~/.glm-cmd/config.ini**
```ini
api_key="your_api_key_here"
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
```

### 完整配置

```ini
api_key="sk.your_actual_api_key_here"
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
model="glm-4.7"
user_prompt="请用最简洁的命令"
memory_enabled=true
memory_rounds=5
stream_enabled=true
temperature=0.7
max_tokens=8192
timeout=30
```

### 高性能配置

```ini
api_key="sk.your_actual_api_key_here"
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
model="glm-4-plus"
temperature=0.5
max_tokens=8192
timeout=60
```

## 贡献

欢迎提交 Issue 和 Pull Request！

## 许可证

[MIT License](LICENSE)

## 相关链接

- [智谱AI开放平台](https://bigmodel.cn)
- [智谱AI API文档](https://open.bigmodel.cn/dev/api)
- [项目仓库](https://github.com/Y5neKO/glm-cmd)
