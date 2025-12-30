/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * Configuration Header
 *===========================================================================*/

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/* 默认配置 */
#define DEFAULT_MODEL "glm-4.7"
#define DEFAULT_TEMP 0.7
#define DEFAULT_MAX_TOKENS 2048
#define DEFAULT_TIMEOUT 30

/* 常用端点 */
#define ENDPOINT_CODING "https://open.bigmodel.cn/api/coding/paas/v4"
#define ENDPOINT_STANDARD "https://open.bigmodel.cn/api/paas/v4"

/* API 配置结构体 */
typedef struct {
    char *api_key;
    char *model;
    char *endpoint;
    char *user_prompt;  /* 用户自定义提示词（前置） */
    double temperature;
    int max_tokens;
    int timeout;
    bool verbose;
} Config;

/* 函数声明 */
Config* config_create(void);
void config_destroy(Config *cfg);
bool config_load_from_env(Config *cfg);
bool config_load_from_file(Config *cfg);
bool config_load(Config *cfg);
void config_print(const Config *cfg);

#endif /* CONFIG_H */
