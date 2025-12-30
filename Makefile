# ============================================================================
# GLM-CMD Makefile
# ============================================================================

# 设置 pkg-config 路径（支持多种系统）
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    export PKG_CONFIG_PATH := /opt/homebrew/lib/pkgconfig:/usr/local/lib/pkgconfig:$(PKG_CONFIG_PATH)
    EXTRA_LDFLAGS := -L/opt/homebrew/lib -L/usr/local/lib
else
    EXTRA_LDFLAGS :=
endif

# 编译器和选项
CC ?= gcc
BASE_CFLAGS = -Wall -Wextra -O2 -std=c11 -D_DEFAULT_SOURCE
LDFLAGS ?=

# 目标文件
TARGET = glm-cmd

# 源文件
SRCDIR = src
SOURCES = $(wildcard $(SRCDIR)/*.c)

# 目标文件
OBJECTS = $(SOURCES:.c=.o)

# 头文件目录
INCLUDES = -I$(SRCDIR)

# 使用 pkg-config 获取库的编译参数
CURL_CFLAGS := $(shell pkg-config --cflags libcurl 2>/dev/null || echo "")
CURL_LIBS := $(shell pkg-config --libs libcurl 2>/dev/null || echo "-lcurl")

# cJSON 处理 - 尝试多个常见路径
CJSON_FOUND := $(shell pkg-config --exists cjson 2>/dev/null && echo "yes" || echo "no")
ifeq ($(CJSON_FOUND),yes)
    CJSON_CFLAGS := $(shell pkg-config --cflags cjson)
    CJSON_LIBS := $(shell pkg-config --libs cjson)
else
    # 回退方案: 尝试常见的安装路径
    CJSON_CHECK_DIRS := /opt/homebrew /usr/local /usr
    CJSON_CFLAGS := $(shell for dir in $(CJSON_CHECK_DIRS); do \
        if [ -f "$$dir/include/cjson/cJSON.h" ]; then \
            echo "-I$$dir/include/cjson"; \
            break; \
        elif [ -f "$$dir/include/cJSON.h" ]; then \
            echo "-I$$dir/include"; \
            break; \
        fi; \
    done)
    CJSON_LIBS := -lcjson
endif

# 合并编译和链接参数
CFLAGS = $(BASE_CFLAGS) $(CURL_CFLAGS) $(CJSON_CFLAGS)
LIBS = $(CURL_LIBS) $(CJSON_LIBS)
LDFLAGS += $(EXTRA_LDFLAGS)

# 安装目录
PREFIX ?= /usr/local
BINDIR = $(PREFIX)/bin

# 操作系统检测
ifeq ($(OS),Windows_NT)
    TARGET = glm-cmd.exe
    LIBS = -lcurl -lcjson -lws2_32
    RM = del /Q
    MKDIR = if not exist $(subst /,\,$(1)) mkdir $(subst /,\,$(1))
    INSTALL = copy /Y
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        LDFLAGS += -framework CoreFoundation -framework SystemConfiguration
    endif
    RM = rm -f
    MKDIR = mkdir -p $(1)
    INSTALL = install -m 0755 $(1) $(2)/
endif

# 默认目标
all: $(TARGET)

# 编译目标文件
%.o: %.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 链接可执行文件
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(TARGET) $(LIBS)
	@echo "Build complete: $(TARGET)"

# 清理
clean:
	@echo "Cleaning build artifacts..."
	$(RM) $(OBJECTS) $(TARGET)

# 安装
install: $(TARGET)
	@echo "Installing $(TARGET) to $(BINDIR)..."
	$(call MKDIR,$(DESTDIR)$(BINDIR))
	$(call INSTALL,$(TARGET),$(DESTDIR)$(BINDIR))
	@echo "Installation complete"

# 卸载
uninstall:
	@echo "Uninstalling $(TARGET) from $(BINDIR)..."
	$(RM) $(DESTDIR)$(BINDIR)/$(TARGET)
	@echo "Uninstall complete"

# 调试版本
debug: BASE_CFLAGS += -g -DDEBUG
debug: clean $(TARGET)

# 发布版本
release: BASE_CFLAGS += -s -O3
release: clean $(TARGET)

# 检查依赖
check-deps:
	@echo "Checking dependencies..."
	@command -v $(CC) >/dev/null 2>&1 || { \
	    echo "Error: Compiler not found. Please install $(CC)."; \
	    exit 1; \
	}
	@echo "  ✓ Compiler: $(CC)"
	@pkg-config --exists libcurl || { \
	    echo "Error: libcurl not found. Please install libcurl-dev."; \
	    exit 1; \
	}
	@echo "  ✓ libcurl"
	@pkg-config --exists cjson || { \
	    echo "Error: cjson not found. Please install libcjson-dev."; \
	    exit 1; \
	}
	@echo "  ✓ cjson"
	@echo "All dependencies satisfied!"

# 帮助
help:
	@echo "GLM-CMD Makefile"
	@echo ""
	@echo "Targets:"
	@echo "  all        - Build the project (default)"
	@echo "  clean      - Remove build artifacts"
	@echo "  install    - Install the binary to $(BINDIR)"
	@echo "  uninstall  - Remove the installed binary"
	@echo "  debug      - Build with debug symbols"
	@echo "  release    - Build optimized release version"
	@echo "  check-deps - Check if required dependencies are installed"
	@echo "  help       - Show this help message"

.PHONY: all clean install uninstall debug release check-deps help
