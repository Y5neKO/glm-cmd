<p align="center">
  English | <a href="README.md">简体中文</a>
</p>

# GLM-CMD

Natural Language to Command Tool - Automatically generate shell commands by describing requirements in natural language

## Key Features

- **Natural Language Input** - Describe requirements in Chinese, English, or other natural languages, automatically generate corresponding commands
- **Command Generation** - Convert natural language into executable shell commands
- **Thought Process Display** - Show command generation logic for easy learning and understanding
- **Cross-Platform Support** - Support Linux, macOS, Windows
- **Configuration File** - Support `~/.glm-cmd/config.ini` configuration
- **One-Click Execution** - Confirm and execute generated commands

## Quick Start

### 1. Install Dependencies

> 💡 **Tip**: For detailed dependency installation guide, see [DEPENDENCIES.md](DEPENDENCIES.md)

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

### 2. Compile and Install

```bash
# Clone repository
git clone https://github.com/Y5neKO/glm-cmd.git
cd glm-cmd

# Compile
./build.sh

# Install (optional)
sudo make install
```

### 3. Initialize Configuration

```bash
# Run initialization wizard
./glm-cmd --init
```

Follow the prompts to enter:
1. **API Key** - Get from [Zhipu AI Open Platform](https://bigmodel.cn)
2. **Endpoint Selection** - Recommend Coding endpoint (specialized for command generation)
3. **Model Name** - Such as `glm-4.7`, `glm-4-plus`, etc.
4. **Temperature Parameter** - Control randomness, default 0.7
5. **Max Tokens** - Response length, default 2048
6. **Timeout** - Request timeout, default 30 seconds

### 4. Usage Examples

```bash
# Find large files
glm-cmd "find all files larger than 100MB in current directory and sort by size"

# Check port usage
glm-cmd "check which process is using port 8080"

# Undo last commit
glm-cmd "undo last commit but keep changes"
```

## How It Works

1. Input natural language requirement description (Chinese, English, etc.)
2. Detect system information (OS, Shell, Architecture, etc.)
3. Call Zhipu AI API to generate commands
4. Display generation logic (optional)
5. Confirm and execute the command

## Configuration Guide

### Configuration File Location

Default location: `~/.glm-cmd/config.ini`

Custom location:
```bash
export GLM_CMD_CONFIG="/path/to/config.ini"
```

### Configuration Options

**Required Configuration:**

```ini
# API Key (get from https://bigmodel.cn)
api_key="your_api_key_here"

# API Endpoint (must configure)
# Recommend using Coding endpoint (specialized for command generation)
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
```

**Optional Configuration:**

```ini
# Model Name (free input)
model="glm-4.7"           # Standard model
# model="glm-4-plus"      # Enhanced model
# model="glm-4-flash"     # Fast model
# model="glm-4-air"       # Lightweight model

# Temperature parameter (0.0-2.0, default 0.7)
temperature=0.7

# Max tokens (default 2048)
max_tokens=2048

# Timeout (seconds, default 30)
timeout=30
```

**Tip**: Use `--verbose` or `-V` parameter to enable detailed output.

### Endpoint Selection Guide

**1. Coding Endpoint (Recommended):**
```
https://open.bigmodel.cn/api/coding/paas/v4
```
- Specialized for command generation and code recommendation
- Suitable for development and operations scenarios
- Higher command accuracy

**2. Standard Endpoint:**
```
https://open.bigmodel.cn/api/paas/v4
```
- General natural language tasks
- Text processing and analysis

## Usage Examples

### File Operations

```bash
# Find large files
glm-cmd "find all files larger than 100MB in current directory"

# Batch rename
glm-cmd "rename all JPG files to lowercase"

# Merge files
glm-cmd "merge all txt files into one file"
```

### Process Management

```bash
# Check port usage
glm-cmd "check which process is using port 8080"

# Kill processes
glm-cmd "kill all processes named python"

# View process resource usage
glm-cmd "show all processes sorted by memory usage"
```

### System Maintenance

```bash
# Clean temporary files
glm-cmd "clean temporary files older than 7 days"

# Disk space check
glm-cmd "show disk usage for each directory"

# Network test
glm-cmd "test connectivity to baidu.com"
```

### Git Operations

```bash
# Undo commit
glm-cmd "undo last commit but keep changes"

# View history
glm-cmd "view detailed information of last 3 commits"

# Branch management
glm-cmd "delete all merged local branches"
```

## Advanced Features

### View System Information

```bash
glm-cmd --info
```

Display content:
- Operating system information
- Shell type and version
- Current configuration parameters
- API Key (partially hidden)

### Verbose Output Mode

```bash
glm-cmd -V "find large files"
```

Display detailed API request and response information for debugging.

### Environment Variable Override

Configuration file can be overridden by environment variables:

```bash
# Temporarily use different model
export GLM_CMD_MODEL="glm-4-plus"
glm-cmd "find large files"

# Temporarily use different endpoint
export GLM_CMD_ENDPOINT="https://open.bigmodel.cn/api/paas/v4"
glm-cmd "translate this text"
```

## Model Selection Guide

| Model | Characteristics | Use Cases |
|------|----------------|-----------|
| **glm-4.7** | Standard model, balanced performance | Daily use |
| **glm-4-plus** | Stronger capabilities | Complex commands |
| **glm-4-flash** | Fast response | Simple commands |
| **glm-4-air** | Lightweight | Resource-constrained environments |

## Command Line Arguments

```bash
glm-cmd [OPTIONS] "query"

Options:
  -h, --help      Display help information
  -v, --version   Display version information
  -V, --verbose   Enable verbose output
  --info          Display system information
  --init          Run initialization wizard
  -I              Same as --init
```

## Troubleshooting

### Problem: API Key Not Configured

**Error Message:**
```
Error: API Key not configured
```

**Solution:**
```bash
# Method 1: Run initialization wizard
glm-cmd --init

# Method 2: Manually create configuration file
mkdir -p ~/.glm-cmd
cat > ~/.glm-cmd/config.ini << EOF
api_key="your_api_key_here"
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
EOF
```

### Problem: Configuration File Not Taking Effect

**Check Method:**
```bash
# Check if configuration file exists
ls -la ~/.glm-cmd/config.ini

# View current configuration
glm-cmd --info
```

**Solution:**
```bash
# Ensure correct file permissions
chmod 600 ~/.glm-cmd/config.ini

# Check configuration file format
cat ~/.glm-cmd/config.ini
```

### Problem: Compilation Error

**Error: libcurl not found**

**Solution:**
```bash
# macOS
brew install curl

# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev

# CentOS/RHEL
sudo yum install libcurl-devel
```

### Problem: Command Generation Failed

**Check Steps:**
1. Verify API Key is correct
2. Check network connection
3. Confirm endpoint configuration is correct
4. Enable verbose mode to view logs

```bash
glm-cmd -V "test"
```

## Common Use Cases Quick Reference

| Requirement | Command Example |
|-------------|-----------------|
| Find large files | `glm-cmd "find all files larger than 100MB in current directory"` |
| Check port | `glm-cmd "check which process is using port 8080"` |
| Batch rename | `glm-cmd "rename all files to lowercase"` |
| Merge files | `glm-cmd "merge all txt files into one file"` |
| Kill processes | `glm-cmd "kill all processes named python"` |
| Disk cleanup | `glm-cmd "clean temporary files older than 7 days"` |
| Modify permissions | `glm-cmd "recursively set current directory permissions to 755"` |
| Network test | `glm-cmd "test connectivity to baidu.com"` |
| Git operations | `glm-cmd "undo last commit but keep changes"` |
| View logs | `glm-cmd "view last 100 lines of error logs"` |

## Security Recommendations

1. **Protect API Key**
   ```bash
   # Set configuration file permissions to user read/write only
   chmod 600 ~/.glm-cmd/config.ini
   ```

2. **Avoid Committing Configuration Files**
   ```bash
   # Add to .gitignore
   echo ".glm-cmd/" >> .gitignore
   echo "config.ini" >> .gitignore
   ```

3. **Regularly Rotate API Key**
   - Recommend changing every 3-6 months
   - Change immediately if suspected leakage

4. **Cautious Command Execution**
   - Review command content after generation
   - Confirm before execution
   - Especially for dangerous operations like deletion and modification

## Configuration Examples

### Basic Configuration

**~/.glm-cmd/config.ini**
```ini
api_key="your_api_key_here"
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
```

### Complete Configuration

```ini
api_key="sk.your_actual_api_key_here"
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
model="glm-4.7"
temperature=0.7
max_tokens=2048
timeout=30
```

### High-Performance Configuration

```ini
api_key="sk.your_actual_api_key_here"
endpoint="https://open.bigmodel.cn/api/coding/paas/v4"
model="glm-4-plus"
temperature=0.5
max_tokens=4096
timeout=60
```

## Contributing

Issues and Pull Requests are welcome!

## License

[MIT License](LICENSE)

## Related Links

- [Zhipu AI Open Platform](https://bigmodel.cn)
- [Zhipu AI API Documentation](https://open.bigmodel.cn/dev/api)
- [Project Repository](https://github.com/Y5neKO/glm-cmd)
