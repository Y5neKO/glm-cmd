/*=============================================================================
 * GLM-CMD - Conversation History Manager Implementation
 *===========================================================================*/

#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

// 尝试多种可能的 cJSON 头文件路径
#if __has_include(<cjson/cJSON.h>)
    #include <cjson/cJSON.h>
#elif __has_include(<cJSON.h>)
    #include <cJSON.h>
#else
    #include <cjson/cJSON.h>
#endif

/* 前向声明 JSON 转义辅助函数 */
static char* json_escape(const char *str);

ConversationHistory* history_create(const char *config_dir, int max_rounds) {
    ConversationHistory *history = (ConversationHistory *)calloc(1, sizeof(ConversationHistory));
    if (!history) {
        fprintf(stderr, "Error: Failed to allocate memory for history\n");
        return NULL;
    }

    /* 设置历史文件路径 */
    size_t path_len = strlen(config_dir) + 32;  /* 足够空间 */
    history->history_file = (char *)malloc(path_len);
    if (!history->history_file) {
        free(history);
        return NULL;
    }
    snprintf(history->history_file, path_len, "%s/history.json", config_dir);

    /* 设置最大轮数 */
    history->max_rounds = max_rounds;
    history->current_count = 0;

    /* 分配轮次数组 */
    history->rounds = (ConversationRound *)calloc(max_rounds, sizeof(ConversationRound));
    if (!history->rounds) {
        free(history->history_file);
        free(history);
        return NULL;
    }

    return history;
}

void history_destroy(ConversationHistory *history) {
    if (!history) return;

    /* 释放所有轮次数据 */
    for (int i = 0; i < history->current_count; i++) {
        if (history->rounds[i].user_input) {
            free(history->rounds[i].user_input);
        }
        if (history->rounds[i].assistant_response) {
            free(history->rounds[i].assistant_response);
        }
    }

    if (history->rounds) free(history->rounds);
    if (history->history_file) free(history->history_file);
    free(history);
}

bool history_add_round(ConversationHistory *history,
                      const char *user_input,
                      const char *assistant_response) {
    if (!history || !user_input || !assistant_response) return false;

    /* 如果已满,移除最旧的记录 */
    if (history->current_count >= history->max_rounds) {
        /* 释放最旧的记录 */
        free(history->rounds[0].user_input);
        free(history->rounds[0].assistant_response);

        /* 移动所有记录向前 */
        for (int i = 0; i < history->current_count - 1; i++) {
            history->rounds[i] = history->rounds[i + 1];
        }

        history->current_count--;
    }

    /* 添加新记录 */
    int index = history->current_count;
    history->rounds[index].user_input = strdup(user_input);
    history->rounds[index].assistant_response = strdup(assistant_response);

    if (!history->rounds[index].user_input || !history->rounds[index].assistant_response) {
        /* 内存分配失败 */
        if (history->rounds[index].user_input) free(history->rounds[index].user_input);
        if (history->rounds[index].assistant_response) free(history->rounds[index].assistant_response);
        return false;
    }

    history->current_count++;
    return true;
}

bool history_save(const ConversationHistory *history) {
    if (!history || !history->history_file) return false;

    /* 确保目录存在 */
    char *dir_copy = strdup(history->history_file);
    if (dir_copy) {
        char *last_slash = strrchr(dir_copy, '/');
        if (last_slash) {
            *last_slash = '\0';
            mkdir_cross(dir_copy);
        }
        free(dir_copy);
    }

    FILE *fp = fopen(history->history_file, "w");
    if (!fp) return false;

    /* 写入 JSON 格式 */
    fprintf(fp, "[\n");
    for (int i = 0; i < history->current_count; i++) {
        fprintf(fp, "  {\n");
        fprintf(fp, "    \"user\": %s,\n", json_escape(history->rounds[i].user_input));
        fprintf(fp, "    \"assistant\": %s\n", json_escape(history->rounds[i].assistant_response));
        fprintf(fp, "  }%s\n", i < history->current_count - 1 ? "," : "");
    }
    fprintf(fp, "]\n");

    fclose(fp);
    return true;
}

bool history_load(ConversationHistory *history) {
    if (!history || !history->history_file) return false;

    FILE *fp = fopen(history->history_file, "r");
    if (!fp) {
        /* 文件不存在是正常情况，首次运行时没有历史文件 */
        return true;
    }

    /* 读取整个文件内容 */
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (file_size <= 0) {
        fclose(fp);
        return true;  /* 空文件 */
    }

    char *json_content = (char *)malloc(file_size + 1);
    if (!json_content) {
        fclose(fp);
        return false;
    }

    size_t read_size = fread(json_content, 1, file_size, fp);
    json_content[read_size] = '\0';
    fclose(fp);

    /* 解析 JSON */
    cJSON *json = cJSON_Parse(json_content);
    free(json_content);

    if (!json) {
        /* JSON 解析失败，可能是文件损坏，返回 true 让程序继续运行 */
        return true;
    }

    /* 确保是数组 */
    if (!cJSON_IsArray(json)) {
        cJSON_Delete(json);
        return true;
    }

    /* 遍历数组，加载每一轮对话 */
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, json) {
        if (history->current_count >= history->max_rounds) {
            break;  /* 已达到最大轮数 */
        }

        /* 提取 user 和 assistant 字段 */
        cJSON *user_json = cJSON_GetObjectItem(item, "user");
        cJSON *assistant_json = cJSON_GetObjectItem(item, "assistant");

        if (user_json && cJSON_IsString(user_json) &&
            assistant_json && cJSON_IsString(assistant_json)) {

            /* 添加到历史 */
            int index = history->current_count;
            history->rounds[index].user_input = strdup(user_json->valuestring);
            history->rounds[index].assistant_response = strdup(assistant_json->valuestring);

            if (history->rounds[index].user_input && history->rounds[index].assistant_response) {
                history->current_count++;
            } else {
                /* 内存分配失败，清理 */
                if (history->rounds[index].user_input) {
                    free(history->rounds[index].user_input);
                }
                if (history->rounds[index].assistant_response) {
                    free(history->rounds[index].assistant_response);
                }
            }
        }
    }

    cJSON_Delete(json);
    return true;
}

bool history_clear(ConversationHistory *history) {
    if (!history) return false;

    /* 释放所有记录 */
    for (int i = 0; i < history->current_count; i++) {
        if (history->rounds[i].user_input) {
            free(history->rounds[i].user_input);
            history->rounds[i].user_input = NULL;
        }
        if (history->rounds[i].assistant_response) {
            free(history->rounds[i].assistant_response);
            history->rounds[i].assistant_response = NULL;
        }
    }

    history->current_count = 0;

    /* 删除历史文件 */
    if (history->history_file) {
        remove(history->history_file);
    }

    return true;
}

char* history_to_json(const ConversationHistory *history) {
    /* 将历史转换为 JSON 数组格式,用于 API 请求 */
    if (!history || history->current_count == 0) {
        return strdup("[]");
    }

    /* 估算所需空间 */
    size_t total_len = 2;  /* [ 和 ] */
    for (int i = 0; i < history->current_count; i++) {
        total_len += strlen(history->rounds[i].user_input);
        total_len += strlen(history->rounds[i].assistant_response);
        total_len += 100;  /* JSON 结构开销 */
    }

    char *json = (char *)malloc(total_len);
    if (!json) return NULL;

    char *ptr = json;
    ptr += sprintf(ptr, "[");

    for (int i = 0; i < history->current_count; i++) {
        if (i > 0) ptr += sprintf(ptr, ",");
        ptr += sprintf(ptr, "{\"role\":\"user\",\"content\":%s}",
                       json_escape(history->rounds[i].user_input));
        ptr += sprintf(ptr, ",{\"role\":\"assistant\",\"content\":%s}",
                       json_escape(history->rounds[i].assistant_response));
    }

    sprintf(ptr, "]");
    return json;
}

void history_print(const ConversationHistory *history) {
    if (!history) return;

    printf("Conversation History (%d rounds):\n", history->current_count);
    printf("─────────────────────────────────────────\n");

    for (int i = 0; i < history->current_count; i++) {
        printf("\n[Round %d]\n", i + 1);
        printf("User:      %s\n", history->rounds[i].user_input);
        printf("Assistant: %s\n", history->rounds[i].assistant_response);
    }

    printf("\n─────────────────────────────────────────\n");
}

/* JSON 字符串转义辅助函数实现 */
static char* json_escape(const char *str) {
    if (!str) return strdup("");

    /* 计算转义后的最大长度 */
    size_t len = strlen(str);
    char *escaped = (char *)malloc(len * 2 + 3);  /* 每个字符可能转义,加上引号 */
    if (!escaped) return strdup("\"\"");

    char *ptr = escaped;
    *ptr++ = '"';

    for (size_t i = 0; i < len; i++) {
        switch (str[i]) {
            case '"':  *ptr++ = '\\'; *ptr++ = '"'; break;
            case '\\': *ptr++ = '\\'; *ptr++ = '\\'; break;
            case '\n': *ptr++ = '\\'; *ptr++ = 'n'; break;
            case '\r': *ptr++ = '\\'; *ptr++ = 'r'; break;
            case '\t': *ptr++ = '\\'; *ptr++ = 't'; break;
            default:   *ptr++ = str[i]; break;
        }
    }

    *ptr++ = '"';
    *ptr = '\0';

    return escaped;
}
