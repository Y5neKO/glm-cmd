/*=============================================================================
 * GLM-CMD - Configuration File Parser (INI Format)
 *===========================================================================*/

#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <stdbool.h>
#include <stddef.h>

/* 配置文件最大路径长度 */
#define CONFIG_MAX_PATH 512

/* 配置文件结构体 */
typedef struct {
    char *api_key;
    char *model;
    char *endpoint;
    char *user_prompt;  /* 用户自定义提示词（前置） */
    bool memory_enabled;  /* 是否启用对话记忆 */
    int memory_rounds;   /* 记忆的对话轮数 */
    bool stream_enabled; /* 是否启用流式输出 */
    double temperature;
    int max_tokens;
    int timeout;
} ConfigFile;

/* 函数声明 */
ConfigFile* config_file_create(void);
void config_file_destroy(ConfigFile *cfg);

/* 查找配置文件 */
bool config_file_find_path(char *path, size_t path_size);

/* 读取配置文件 */
bool config_file_read(const char *path, ConfigFile *cfg);

/* 写入配置文件 */
bool config_file_write(const char *path, const ConfigFile *cfg);

/* 获取默认配置文件路径 */
bool config_file_get_default_path(char *path, size_t path_size);

/* 创建配置文件目录 */
bool config_file_create_directory(const char *path);

#endif /* CONFIG_PARSER_H */
