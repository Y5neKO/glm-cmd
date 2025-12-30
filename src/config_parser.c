/*=============================================================================
 * GLM-CMD - Configuration File Parser Implementation (INI Format)
 *===========================================================================*/

#include "config_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef _WIN32
    #include <direct.h>
    #include <shlobj.h>
    #define mkdir_cross(path) _mkdir(path)
#else
    #include <unistd.h>
    #include <pwd.h>
    #define mkdir_cross(path) mkdir(path, 0755)
#endif

/* 去除字符串首尾空白 */
static char* trim(char *str) {
    char *start = str;
    char *end;

    if (!str || !*str) return str;

    /* 去除前导空白 */
    while (isspace((unsigned char)*start)) start++;

    /* 全部是空白 */
    if (*start == '\0') return start;

    /* 去除尾部空白 */
    end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    return start;
}

/* 解析键值对 */
static bool parse_key_value(char *line, char **key, char **value) {
    char *equals = strchr(line, '=');

    if (!equals) return false;

    *equals = '\0';
    *key = trim(line);
    *value = trim(equals + 1);

    return **key != '\0' && **value != '\0';
}

/* 去除引号 */
static char* unquote(char *str) {
    size_t len = strlen(str);

    if (len >= 2 && str[0] == '"' && str[len - 1] == '"') {
        str[len - 1] = '\0';
        return str + 1;
    }

    return str;
}

ConfigFile* config_file_create(void) {
    ConfigFile *cfg = (ConfigFile *)calloc(1, sizeof(ConfigFile));
    if (!cfg) return NULL;

    cfg->api_key = NULL;
    cfg->model = NULL;
    cfg->endpoint = NULL;
    cfg->user_prompt = NULL;
    cfg->temperature = 0.7;
    cfg->max_tokens = 2048;
    cfg->timeout = 30;

    return cfg;
}

void config_file_destroy(ConfigFile *cfg) {
    if (!cfg) return;

    if (cfg->api_key) free(cfg->api_key);
    if (cfg->model) free(cfg->model);
    if (cfg->endpoint) free(cfg->endpoint);
    if (cfg->user_prompt) free(cfg->user_prompt);

    free(cfg);
}

bool config_file_get_default_path(char *path, size_t path_size) {
    if (!path || path_size == 0) return false;

    const char *home = NULL;

#ifdef _WIN32
    char home_dir[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, home_dir))) {
        home = home_dir;
    }
#else
    home = getenv("HOME");
    if (!home) {
        struct passwd *pw = getpwuid(getuid());
        if (pw) home = pw->pw_dir;
    }
#endif

    if (!home) return false;

    snprintf(path, path_size, "%s/.glm-cmd/config.ini", home);

    return true;
}

bool config_file_create_directory(const char *path) {
    if (!path) return false;

    char dir_path[CONFIG_MAX_PATH];
    char *last_slash;

    /* 复制路径 */
    strncpy(dir_path, path, sizeof(dir_path) - 1);
    dir_path[sizeof(dir_path) - 1] = '\0';

    /* 找到最后一个斜杠 */
    last_slash = strrchr(dir_path, '/');
#ifdef _WIN32
    char *backslash = strrchr(dir_path, '\\');
    if (backslash && (!last_slash || backslash > last_slash)) {
        last_slash = backslash;
    }
#endif

    if (last_slash) {
        *last_slash = '\0';

        /* 检查目录是否存在 */
#ifdef _WIN32
        struct _stat st;
        if (_stat(dir_path, &st) != 0) {
#else
        struct stat st;
        if (stat(dir_path, &st) != 0) {
#endif
            /* 创建目录 */
            if (mkdir_cross(dir_path) != 0) {
                return false;
            }
        }
    }

    return true;
}

bool config_file_read(const char *path, ConfigFile *cfg) {
    if (!path || !cfg) return false;

    FILE *fp = fopen(path, "r");
    if (!fp) return false;

    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        char *trimmed = trim(line);

        /* 跳过空行和注释 */
        if (*trimmed == '\0' || *trimmed == '#' || *trimmed == ';') continue;

        /* 解析键值对 */
        char *key, *value;
        if (parse_key_value(trimmed, &key, &value)) {
            char *unquoted_value = unquote(value);

            /* API Key */
            if (strcmp(key, "api_key") == 0) {
                if (cfg->api_key) free(cfg->api_key);
                cfg->api_key = strdup(unquoted_value);
            }
            /* Model */
            else if (strcmp(key, "model") == 0) {
                if (cfg->model) free(cfg->model);
                cfg->model = strdup(unquoted_value);
            }
            /* Endpoint */
            else if (strcmp(key, "endpoint") == 0) {
                if (cfg->endpoint) free(cfg->endpoint);
                cfg->endpoint = strdup(unquoted_value);
            }
            /* User Prompt */
            else if (strcmp(key, "user_prompt") == 0) {
                if (cfg->user_prompt) free(cfg->user_prompt);
                cfg->user_prompt = strdup(unquoted_value);
            }
            /* Temperature */
            else if (strcmp(key, "temperature") == 0) {
                cfg->temperature = atof(unquoted_value);
            }
            /* Max Tokens */
            else if (strcmp(key, "max_tokens") == 0) {
                cfg->max_tokens = atoi(unquoted_value);
            }
            /* Timeout */
            else if (strcmp(key, "timeout") == 0) {
                cfg->timeout = atoi(unquoted_value);
            }
        }
    }

    fclose(fp);
    return true;
}

bool config_file_write(const char *path, const ConfigFile *cfg) {
    if (!path || !cfg) return false;

    /* 确保目录存在 */
    if (!config_file_create_directory(path)) {
        return false;
    }

    FILE *fp = fopen(path, "w");
    if (!fp) return false;

    fprintf(fp, "# GLM-CMD Configuration File\n");
    fprintf(fp, "# Generated automatically - Edit with care\n");
    fprintf(fp, "\n");

    if (cfg->api_key) {
        fprintf(fp, "# API Key for Zhipu AI (required)\n");
        fprintf(fp, "api_key=\"%s\"\n", cfg->api_key);
        fprintf(fp, "\n");
    }

    if (cfg->model) {
        fprintf(fp, "# Model name (default: glm-4.7)\n");
        fprintf(fp, "model=\"%s\"\n", cfg->model);
        fprintf(fp, "\n");
    }

    if (cfg->endpoint) {
        fprintf(fp, "# API endpoint (default: Coding endpoint)\n");
        fprintf(fp, "endpoint=\"%s\"\n", cfg->endpoint);
        fprintf(fp, "\n");
    }

    if (cfg->user_prompt) {
        fprintf(fp, "# User custom prompt (prefixed to each user input)\n");
        fprintf(fp, "# Leave empty if not needed\n");
        fprintf(fp, "user_prompt=\"%s\"\n", cfg->user_prompt);
        fprintf(fp, "\n");
    }

    fprintf(fp, "# Temperature parameter (0.0 - 2.0, default: 0.7)\n");
    fprintf(fp, "temperature=%.1f\n", cfg->temperature);
    fprintf(fp, "\n");

    fprintf(fp, "# Maximum tokens (default: 2048)\n");
    fprintf(fp, "max_tokens=%d\n", cfg->max_tokens);
    fprintf(fp, "\n");

    fprintf(fp, "# Request timeout in seconds (default: 30)\n");
    fprintf(fp, "timeout=%d\n", cfg->timeout);

    fclose(fp);
    return true;
}

bool config_file_find_path(char *path, size_t path_size) {
    if (!path || path_size == 0) return false;

    /* 1. 检查环境变量指定的路径 */
    const char *env_path = getenv("GLM_CMD_CONFIG");
    if (env_path && strlen(env_path) > 0) {
        strncpy(path, env_path, path_size - 1);
        path[path_size - 1] = '\0';

        /* 检查文件是否存在 */
#ifdef _WIN32
        struct _stat st;
        if (_stat(path, &st) == 0) return true;
#else
        struct stat st;
        if (stat(path, &st) == 0) return true;
#endif
    }

    /* 2. 检查默认路径 */
    if (config_file_get_default_path(path, path_size)) {
#ifdef _WIN32
        struct _stat st;
        if (_stat(path, &st) == 0) return true;
#else
        struct stat st;
        if (stat(path, &st) == 0) return true;
#endif
    }

    return false;
}
