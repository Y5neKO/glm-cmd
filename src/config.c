/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * Configuration Implementation
 *===========================================================================*/

#include "config.h"
#include "config_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Config* config_create(void) {
    Config *cfg = (Config *)malloc(sizeof(Config));
    if (!cfg) {
        fprintf(stderr, "Error: Failed to allocate memory for config\n");
        return NULL;
    }

    /* 初始化默认值 */
    cfg->api_key = NULL;
    cfg->model = strdup(DEFAULT_MODEL);
    cfg->endpoint = NULL;  /* 端点必须通过配置文件或环境变量设置 */
    cfg->user_prompt = NULL;
    cfg->memory_enabled = DEFAULT_MEMORY_ENABLED;
    cfg->memory_rounds = DEFAULT_MEMORY_ROUNDS;
    cfg->temperature = DEFAULT_TEMP;
    cfg->max_tokens = DEFAULT_MAX_TOKENS;
    cfg->timeout = DEFAULT_TIMEOUT;
    cfg->verbose = false;

    return cfg;
}

void config_destroy(Config *cfg) {
    if (!cfg) return;

    if (cfg->api_key) free(cfg->api_key);
    if (cfg->model) free(cfg->model);
    if (cfg->endpoint) free(cfg->endpoint);
    if (cfg->user_prompt) free(cfg->user_prompt);

    free(cfg);
}

/* 从 ConfigFile 复制到 Config */
static void copy_from_config_file(Config *cfg, const ConfigFile *file_cfg) {
    if (file_cfg->api_key) {
        if (cfg->api_key) free(cfg->api_key);
        cfg->api_key = strdup(file_cfg->api_key);
    }

    if (file_cfg->model) {
        free(cfg->model);
        cfg->model = strdup(file_cfg->model);
    }

    if (file_cfg->endpoint) {
        free(cfg->endpoint);
        cfg->endpoint = strdup(file_cfg->endpoint);
    }

    if (file_cfg->user_prompt) {
        if (cfg->user_prompt) free(cfg->user_prompt);
        cfg->user_prompt = strdup(file_cfg->user_prompt);
    }

    cfg->memory_enabled = file_cfg->memory_enabled;
    cfg->memory_rounds = file_cfg->memory_rounds;

    cfg->temperature = file_cfg->temperature;
    cfg->max_tokens = file_cfg->max_tokens;
    cfg->timeout = file_cfg->timeout;
}

bool config_load_from_env(Config *cfg) {
    const char *env_val;

    /* 读取 API Key */
    env_val = getenv("GLM_CMD_API_KEY");
    if (env_val && strlen(env_val) > 0) {
        cfg->api_key = strdup(env_val);
    } else {
        fprintf(stderr, "Error: GLM_CMD_API_KEY environment variable not set\n");
        return false;
    }

    /* 读取模型名称 */
    env_val = getenv("GLM_CMD_MODEL");
    if (env_val && strlen(env_val) > 0) {
        free(cfg->model);
        cfg->model = strdup(env_val);
    }

    /* 读取端点 */
    env_val = getenv("GLM_CMD_ENDPOINT");
    if (env_val && strlen(env_val) > 0) {
        free(cfg->endpoint);
        cfg->endpoint = strdup(env_val);
    }

    /* 读取温度 */
    env_val = getenv("GLM_CMD_TEMP");
    if (env_val && strlen(env_val) > 0) {
        cfg->temperature = atof(env_val);
    }

    /* 读取最大 tokens */
    env_val = getenv("GLM_CMD_MAX_TOKENS");
    if (env_val && strlen(env_val) > 0) {
        cfg->max_tokens = atoi(env_val);
    }

    /* 读取超时 */
    env_val = getenv("GLM_CMD_TIMEOUT");
    if (env_val && strlen(env_val) > 0) {
        cfg->timeout = atoi(env_val);
    }

    /* 读取用户自定义提示词 */
    env_val = getenv("GLM_CMD_USER_PROMPT");
    if (env_val && strlen(env_val) > 0) {
        if (cfg->user_prompt) free(cfg->user_prompt);
        cfg->user_prompt = strdup(env_val);
    }

    return true;
}

bool config_load_from_file(Config *cfg) {
    char config_path[CONFIG_MAX_PATH];

    /* 查找配置文件 */
    if (!config_file_find_path(config_path, sizeof(config_path))) {
        /* 配置文件不存在，不是错误，返回 true 使用默认值 */
        return true;
    }

    /* 读取配置文件 */
    ConfigFile *file_cfg = config_file_create();
    if (!file_cfg) {
        fprintf(stderr, "Warning: Failed to allocate memory for config file\n");
        return true;
    }

    if (!config_file_read(config_path, file_cfg)) {
        fprintf(stderr, "Warning: Failed to read config file: %s\n", config_path);
        config_file_destroy(file_cfg);
        return true;
    }

    /* 复制配置 */
    copy_from_config_file(cfg, file_cfg);
    config_file_destroy(file_cfg);

    return true;
}

bool config_load(Config *cfg) {
    /* 优先级：配置文件 > 环境变量 > 默认值 */

    /* 1. 首先尝试从配置文件加载 */
    if (!config_load_from_file(cfg)) {
        return false;
    }

    /* 2. 然后环境变量会覆盖配置文件的值 */
    const char *env_val;

    env_val = getenv("GLM_CMD_API_KEY");
    if (env_val && strlen(env_val) > 0) {
        if (cfg->api_key) free(cfg->api_key);
        cfg->api_key = strdup(env_val);
    }

    env_val = getenv("GLM_CMD_MODEL");
    if (env_val && strlen(env_val) > 0) {
        free(cfg->model);
        cfg->model = strdup(env_val);
    }

    env_val = getenv("GLM_CMD_ENDPOINT");
    if (env_val && strlen(env_val) > 0) {
        free(cfg->endpoint);
        cfg->endpoint = strdup(env_val);
    }

    env_val = getenv("GLM_CMD_TEMP");
    if (env_val && strlen(env_val) > 0) {
        cfg->temperature = atof(env_val);
    }

    env_val = getenv("GLM_CMD_MAX_TOKENS");
    if (env_val && strlen(env_val) > 0) {
        cfg->max_tokens = atoi(env_val);
    }

    env_val = getenv("GLM_CMD_TIMEOUT");
    if (env_val && strlen(env_val) > 0) {
        cfg->timeout = atoi(env_val);
    }

    /* 读取用户自定义提示词 */
    env_val = getenv("GLM_CMD_USER_PROMPT");
    if (env_val && strlen(env_val) > 0) {
        if (cfg->user_prompt) free(cfg->user_prompt);
        cfg->user_prompt = strdup(env_val);
    }

    env_val = getenv("GLM_CMD_VERBOSE");
    if (env_val && (strcmp(env_val, "1") == 0 || strcmp(env_val, "true") == 0)) {
        cfg->verbose = true;
    }

    /* 3. 检查是否有 API Key */
    if (!cfg->api_key || strlen(cfg->api_key) == 0) {
        fprintf(stderr, "Error: API Key not configured. Please set GLM_CMD_API_KEY environment variable\n");
        fprintf(stderr, "or create a config file at ~/.glm-cmd/config.ini\n");
        return false;
    }

    /* 4. 检查是否有端点 */
    if (!cfg->endpoint || strlen(cfg->endpoint) == 0) {
        fprintf(stderr, "Error: API endpoint not configured. Please set GLM_CMD_ENDPOINT environment variable\n");
        fprintf(stderr, "or add 'endpoint' to your config file at ~/.glm-cmd/config.ini\n");
        fprintf(stderr, "\nAvailable endpoints:\n");
        fprintf(stderr, "  - Coding endpoint:   %s\n", ENDPOINT_CODING);
        fprintf(stderr, "  - Standard endpoint: %s\n", ENDPOINT_STANDARD);
        return false;
    }

    return true;
}

void config_print(const Config *cfg) {
    if (!cfg) return;

    printf("Configuration:\n");
    printf("  Model: %s\n", cfg->model);
    printf("  Endpoint: %s\n", cfg->endpoint);
    printf("  Temperature: %.1f\n", cfg->temperature);
    printf("  Max Tokens: %d\n", cfg->max_tokens);
    printf("  Timeout: %d seconds\n", cfg->timeout);

    if (cfg->api_key) {
        /* 隐藏部分 API Key */
        size_t key_len = strlen(cfg->api_key);
        if (key_len > 8) {
            printf("  API Key: %.*s***%s\n", 4, cfg->api_key, cfg->api_key + key_len - 4);
        } else {
            printf("  API Key: ***\n");
        }
    }
}
