/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * System Information Header (Cross-Platform)
 *===========================================================================*/

#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <stdbool.h>

/* 操作系统类型 */
typedef enum {
    OS_UNKNOWN,
    OS_LINUX,
    OS_MACOS,
    OS_WINDOWS,
    OS_BSD
} OSType;

/* Shell 类型 */
typedef enum {
    SHELL_UNKNOWN,
    SHELL_BASH,
    SHELL_ZSH,
    SHELL_FISH,
    SHELL_POWERSHELL,
    SHELL_CMD
} ShellType;

/* 系统信息结构体 */
typedef struct {
    OSType os_type;
    ShellType shell_type;
    char *os_name;
    char *os_version;
    char *shell_name;
    char *shell_version;
    char *arch;
    char *hostname;
} SystemInfo;

/* 函数声明 */
SystemInfo* system_info_create(void);
void system_info_destroy(SystemInfo *info);
bool system_info_detect(SystemInfo *info);
const char* os_type_to_string(OSType type);
const char* shell_type_to_string(ShellType type);
void system_info_print(const SystemInfo *info);
char* system_info_to_prompt(const SystemInfo *info);

#endif /* SYSTEM_INFO_H */
