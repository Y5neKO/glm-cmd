/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * System Information Implementation (Cross-Platform)
 *===========================================================================*/

#include "system_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#ifdef __linux__
    #include <sys/utsname.h>
    #include <errno.h>
#endif

#ifdef __APPLE__
    #include <sys/utsname.h>
    #include <sys/sysctl.h>
    #include <TargetConditionals.h>
#endif

SystemInfo* system_info_create(void) {
    SystemInfo *info = (SystemInfo *)calloc(1, sizeof(SystemInfo));
    if (!info) {
        fprintf(stderr, "Error: Failed to allocate memory for system info\n");
        return NULL;
    }

    info->os_type = OS_UNKNOWN;
    info->shell_type = SHELL_UNKNOWN;
    info->os_name = NULL;
    info->os_version = NULL;
    info->shell_name = NULL;
    info->shell_version = NULL;
    info->arch = NULL;
    info->hostname = NULL;

    return info;
}

void system_info_destroy(SystemInfo *info) {
    if (!info) return;

    if (info->os_name) free(info->os_name);
    if (info->os_version) free(info->os_version);
    if (info->shell_name) free(info->shell_name);
    if (info->shell_version) free(info->shell_version);
    if (info->arch) free(info->arch);
    if (info->hostname) free(info->hostname);

    free(info);
}

bool system_info_detect(SystemInfo *info) {
    if (!info) return false;

    char buffer[256];

    /* 检测操作系统类型 */
#ifdef __APPLE__
    info->os_type = OS_MACOS;
    info->os_name = strdup("macOS");

    /* 获取 macOS 版本 */
    struct utsname uts;
    if (uname(&uts) == 0) {
        info->os_version = strdup(uts.release);
        info->arch = strdup(uts.machine);
    }
#elif defined(__linux__)
    info->os_type = OS_LINUX;

    /* 读取 /etc/os-release 获取发行版信息 */
    FILE *fp = fopen("/etc/os-release", "r");
    if (fp) {
        while (fgets(buffer, sizeof(buffer), fp)) {
            if (strncmp(buffer, "PRETTY_NAME=", 12) == 0) {
                /* 去除引号和换行符 */
                char *start = strchr(buffer, '"');
                if (start) {
                    start++;
                    char *end = strchr(start, '"');
                    if (end) *end = '\0';
                    info->os_name = strdup(start);
                }
                break;
            }
        }
        fclose(fp);
    }

    /* 获取内核版本和架构 */
    struct utsname uts;
    if (uname(&uts) == 0) {
        if (!info->os_name) info->os_name = strdup("Linux");
        info->os_version = strdup(uts.release);
        info->arch = strdup(uts.machine);
    }
#elif defined(_WIN32) || defined(_WIN64)
    info->os_type = OS_WINDOWS;
    info->os_name = strdup("Windows");

    /* 获取 Windows 版本 */
    OSVERSIONINFOA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    #pragma warning(push)
    #pragma warning(disable:4996)
    GetVersionExA(&osvi);
    #pragma warning(pop)

    snprintf(buffer, sizeof(buffer), "%d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion);
    info->os_version = strdup(buffer);

    /* 获取架构 */
    SYSTEM_INFO si;
    GetNativeSystemInfo(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
        info->arch = strdup("x86_64");
    } else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64) {
        info->arch = strdup("arm64");
    } else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
        info->arch = strdup("x86");
    } else {
        info->arch = strdup("unknown");
    }
#else
    info->os_type = OS_UNKNOWN;
    info->os_name = strdup("Unknown");
#endif

    /* 获取主机名 */
#ifdef _WIN32
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        info->hostname = strdup(buffer);
    }
#else
    if (gethostname(buffer, sizeof(buffer)) == 0) {
        info->hostname = strdup(buffer);
    }
#endif

    /* 检测 Shell 类型 */
    const char *shell_env = getenv("SHELL");
    if (shell_env) {
        if (strstr(shell_env, "bash")) {
            info->shell_type = SHELL_BASH;
            info->shell_name = strdup("bash");
        } else if (strstr(shell_env, "zsh")) {
            info->shell_type = SHELL_ZSH;
            info->shell_name = strdup("zsh");
        } else if (strstr(shell_env, "fish")) {
            info->shell_type = SHELL_FISH;
            info->shell_name = strdup("fish");
        } else {
            info->shell_name = strdup(shell_env);
        }
    } else {
#ifdef _WIN32
        info->shell_type = SHELL_POWERSHELL;
        info->shell_name = strdup("PowerShell");
#else
        info->shell_type = SHELL_UNKNOWN;
        info->shell_name = strdup("Unknown");
#endif
    }

    return true;
}

const char* os_type_to_string(OSType type) {
    switch (type) {
        case OS_LINUX: return "Linux";
        case OS_MACOS: return "macOS";
        case OS_WINDOWS: return "Windows";
        case OS_BSD: return "BSD";
        default: return "Unknown";
    }
}

const char* shell_type_to_string(ShellType type) {
    switch (type) {
        case SHELL_BASH: return "Bash";
        case SHELL_ZSH: return "Zsh";
        case SHELL_FISH: return "Fish";
        case SHELL_POWERSHELL: return "PowerShell";
        case SHELL_CMD: return "CMD";
        default: return "Unknown";
    }
}

void system_info_print(const SystemInfo *info) {
    if (!info) return;

    printf("System Information:\n");
    printf("  OS: %s\n", os_type_to_string(info->os_type));
    if (info->os_name) printf("  OS Name: %s\n", info->os_name);
    if (info->os_version) printf("  OS Version: %s\n", info->os_version);
    printf("  Shell: %s\n", shell_type_to_string(info->shell_type));
    if (info->shell_name) printf("  Shell Name: %s\n", info->shell_name);
    if (info->arch) printf("  Architecture: %s\n", info->arch);
    if (info->hostname) printf("  Hostname: %s\n", info->hostname);
}

char* system_info_to_prompt(const SystemInfo *info) {
    if (!info) return NULL;

    char buffer[1024];
    int offset = 0;

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                      "## System Context\n\n");

    if (info->os_name) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                          "- Operating System: %s", info->os_name);
    }

    if (info->os_version) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                          " (%s)\n", info->os_version);
    } else {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset, "\n");
    }

    if (info->arch) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                          "- Architecture: %s\n", info->arch);
    }

    if (info->shell_name) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                          "- Default Shell: %s\n", info->shell_name);
    }

    if (info->hostname) {
        offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                          "- Hostname: %s\n", info->hostname);
    }

    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                      "\n## Command Compatibility\n\n");
    offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                      "Generate commands that are compatible with the detected system.\n");

    return strdup(buffer);
}
