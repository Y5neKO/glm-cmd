# GLM-CMD 依赖安装指南

本文档说明如何在各种操作系统上安装 GLM-CMD 所需的依赖库。

## 必需依赖

GLM-CMD 需要以下两个库：

1. **libcurl** - HTTP 客户端库
2. **cJSON** - JSON 解析库

## macOS

### 使用 Homebrew（推荐）

```bash
# 安装依赖
brew install curl cjson

# 编译项目
make
```

### 使用 MacPorts

```bash
# 安装依赖
sudo port install curl libcjson

# 编译项目
make
```

## Ubuntu/Debian

```bash
# 更新包列表
sudo apt-get update

# 安装依赖
sudo apt-get install build-essential libcurl4-openssl-dev libcjson-dev

# 编译项目
make
```

## CentOS/RHEL/Fedora

### CentOS/RHEL

```bash
# 安装 EPEL 仓库（如果尚未安装）
sudo yum install epel-release

# 安装依赖
sudo yum groupinstall "Development Tools"
sudo yum install libcurl-devel cjson-devel

# 编译项目
make
```

### Fedora

```bash
# 安装依赖
sudo dnf install gcc make libcurl-devel cjson-devel

# 编译项目
make
```

## Arch Linux

```bash
# 安装依赖
sudo pacman -S base-devel curl cjson

# 编译项目
make
```

## Windows

### 使用 MSYS2

1. 下载并安装 [MSYS2](https://www.msys2.org/)

2. 在 MSYS2 终端中安装依赖：
```bash
# 更新系统
pacman -Syu

# 安装依赖
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-curl mingw-w64-x86_64-cjson make

# 编译项目
make
```

### 使用 vcpkg

```bash
# 安装 vcpkg（如果尚未安装）
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat

# 安装依赖
./vcpkg install curl:x64-windows cjson:x64-windows

# 设置环境变量
set VCPKG_ROOT=C:\path\to\vcpkg
set PATH=%VCPKG_ROOT%;%PATH%

# 编译项目（可能需要修改 Makefile）
make
```

## 从源码安装 cJSON

如果您的系统没有提供 cJSON 包，可以从源码安装：

### Linux/macOS

```bash
# 克隆仓库
git clone https://github.com/DaveGamble/cJSON.git
cd cJSON

# 编译
mkdir build && cd build
cmake .. -DENABLE_CJSON_UTILS=On -DENABLE_CJSON_TEST=Off
make

# 安装
sudo make install

# 更新动态链接库缓存（Linux）
sudo ldconfig

# 返回 glm-cmd 目录并编译
cd /path/to/glm-cmd
make
```

### macOS 使用 Homebrew 安装本地版本

```bash
# 从源码安装
brew install --build-from-source cjson

# 或者直接安装
brew install cjson
```

## 验证安装

运行以下命令验证依赖是否正确安装：

```bash
# 检查 pkg-config 能否找到库
pkg-config --exists libcurl && echo "✓ libcurl found" || echo "✗ libcurl not found"
pkg-config --exists cjson && echo "✓ cjson found" || echo "✗ cjson not found"

# 或使用 make 提供的检查命令
make check-deps
```

## 常见问题

### Q: 找不到 cJSON.h 头文件

**A**: 确保 cJSON 开发包已安装：
- Ubuntu/Debian: `sudo apt-get install libcjson-dev`
- CentOS/RHEL: `sudo yum install cjson-devel`
- macOS: `brew install cjson`

### Q: 找不到 libcjson.so

**A**: 设置库路径或安装开发包：

```bash
# Linux
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/cjson.conf
sudo ldconfig

# macOS（通常不需要，Homebrew 会自动处理）
```

### Q: 编译时提示找不到 pkg-config

**A**: 安装 pkg-config：

```bash
# Ubuntu/Debian
sudo apt-get install pkg-config

# CentOS/RHEL
sudo yum install pkgconfig

# macOS
brew install pkg-config
```

### Q: macOS 上使用系统的 curl

**A**: macOS 系统自带 curl，但版本可能较老。建议使用 Homebrew 安装的版本：

```bash
brew install curl
# Makefile 会自动处理 Homebrew 的路径
```

## 下一步

依赖安装完成后，请参阅 [README.md](README.md) 了解如何编译和使用 GLM-CMD。

## 获取帮助

如果遇到问题：

1. 检查 Makefile 中的路径设置
2. 确保 pkg-config 能找到所有库
3. 查看 [故障排除](README.md#troubleshooting) 部分
4. 提交 Issue 到项目仓库
