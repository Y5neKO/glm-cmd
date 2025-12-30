#!/bin/bash

# ============================================================================
# GLM-CMD Build Script
# ============================================================================

set -e

# 颜色
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}Building GLM-CMD...${NC}"

# 设置 pkg-config 路径
export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:/usr/local/lib/pkgconfig"

if [[ "$OSTYPE" == "darwin"* ]]; then
    export LDFLAGS="-L/opt/homebrew/lib -L/usr/local/lib"
fi

# 检查依赖
echo -e "${BLUE}Checking dependencies...${NC}"

if ! command -v pkg-config &> /dev/null; then
    echo "Error: pkg-config not found"
    exit 1
fi

if ! pkg-config --exists libcurl; then
    echo "Error: libcurl not found"
    exit 1
fi

if ! pkg-config --exists libcjson; then
    echo "Error: libcjson not found"
    exit 1
fi

echo -e "${GREEN}✓ All dependencies found${NC}"

# 获取编译参数
CFLAGS="-Wall -Wextra -O2 -std=c11"
CFLAGS+=" $(pkg-config --cflags libcurl)"
CFLAGS+=" $(pkg-config --cflags libcjson)"
CFLAGS+=" -Isrc"

LIBS="$(pkg-config --libs libcurl)"
LIBS+=" $(pkg-config --libs libcjson)"

if [[ "$OSTYPE" == "darwin"* ]]; then
    LIBS+=" -framework CoreFoundation -framework SystemConfiguration"
fi

# 清理旧文件
echo -e "${BLUE}Cleaning old build artifacts...${NC}"
rm -f src/*.o glm-cmd

# 编译
echo -e "${BLUE}Compiling...${NC}"

for src in src/*.c; do
    obj="${src%.c}.o"
    echo "  Compiling $src..."
    cc $CFLAGS -c "$src" -o "$obj"
done

# 链接
echo -e "${BLUE}Linking...${NC}"
cc src/*.o -o glm-cmd $LIBS

echo -e "${GREEN}✓ Build complete: glm-cmd${NC}"
echo ""
echo "Install with: sudo make install"
echo "Or run directly: ./glm-cmd"
