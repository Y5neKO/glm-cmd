/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * API Client Implementation
 *===========================================================================*/

#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

// 尝试多种可能的 cJSON 头文件路径
#if __has_include(<cjson/cJSON.h>)
    #include <cjson/cJSON.h>
#elif __has_include(<cJSON.h>)
    #include <cJSON.h>
#else
    #include <cjson/cJSON.h>
#endif

/* 写入回调函数结构体 */
typedef struct {
    char *data;
    size_t size;
} WriteCallbackData;

/* 写入回调函数 */
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    WriteCallbackData *mem = (WriteCallbackData *)userp;

    char *ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Error: Not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = '\0';

    return realsize;
}

ApiResponse* api_response_create(void) {
    ApiResponse *response = (ApiResponse *)calloc(1, sizeof(ApiResponse));
    if (!response) {
        fprintf(stderr, "Error: Failed to allocate memory for API response\n");
        return NULL;
    }

    response->raw_response = NULL;
    response->thinking_process = NULL;
    response->command = NULL;
    response->success = false;
    response->error_message = NULL;

    return response;
}

void api_response_destroy(ApiResponse *response) {
    if (!response) return;

    if (response->raw_response) free(response->raw_response);
    if (response->thinking_process) free(response->thinking_process);
    if (response->command) free(response->command);
    if (response->error_message) free(response->error_message);

    free(response);
}

char* build_system_prompt(const SystemInfo *sys_info) {
    char *sys_context = NULL;

    if (sys_info) {
        sys_context = system_info_to_prompt(sys_info);
    }

    /* 基础系统提示词 */
    const char *base_prompt =
        "你是一个专业的命令行助手，擅长将自然语言转换为精确的 shell 命令。\n\n"
        "## 你的任务\n"
        "1. **理解用户意图**：分析用户的需求，识别要执行的操作\n"
        "2. **思考过程**：展示你的推理过程，包括：\n"
        "   - 分析用户需求的关键要素\n"
        "   - 考虑不同的实现方案\n"
        "   - 选择最优方案的理由\n"
        "   - 潜在的风险和注意事项\n"
        "3. **生成命令**：生成简洁、高效、安全的 shell 命令\n\n"
        "## 输出格式要求\n"
        "你必须严格按照以下格式输出：\n\n"
        "**思考过程：**\n"
        "[详细描述你的分析和推理过程]\n\n"
        "**命令：**\n"
        "```bash\n"
        "[生成的命令，不要包含任何解释文字]\n"
        "```\n\n"
        "## 注意事项\n"
        "- 命令必须实用、安全、符合最佳实践\n"
        "- 优先使用现代工具和语法\n"
        "- 根据系统上下文生成兼容的命令\n"
        "- 避免破坏性操作，必要时添加确认选项\n"
        "- 对于复杂操作，提供带注释的版本\n";

    size_t prompt_len = strlen(base_prompt);
    if (sys_context) {
        prompt_len += strlen(sys_context) + 1;
    }

    char *full_prompt = (char *)malloc(prompt_len + 1);
    if (!full_prompt) {
        if (sys_context) free(sys_context);
        return NULL;
    }

    snprintf(full_prompt, prompt_len + 1, "%s\n%s",
             sys_context ? sys_context : "",
             base_prompt);

    if (sys_context) free(sys_context);

    return full_prompt;
}

char* build_request_body(const Config *cfg, const SystemInfo *sys_info,
                         const char *user_input) {
    cJSON *json = cJSON_CreateObject();
    if (!json) {
        fprintf(stderr, "Error: Failed to create JSON object\n");
        return NULL;
    }

    /* 添加 model */
    cJSON_AddStringToObject(json, "model", cfg->model);

    /* 添加 messages */
    cJSON *messages = cJSON_CreateArray();
    if (!messages) {
        fprintf(stderr, "Error: Failed to create messages array\n");
        cJSON_Delete(json);
        return NULL;
    }

    /* System message */
    cJSON *system_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(system_msg, "role", "system");

    char *system_prompt = build_system_prompt(sys_info);
    if (!system_prompt) {
        cJSON_Delete(system_msg);
        cJSON_Delete(messages);
        cJSON_Delete(json);
        return NULL;
    }

    cJSON_AddStringToObject(system_msg, "content", system_prompt);
    cJSON_AddItemToArray(messages, system_msg);

    /* User message */
    cJSON *user_msg = cJSON_CreateObject();
    cJSON_AddStringToObject(user_msg, "role", "user");

    /* 如果有用户自定义提示词,则前置到用户输入 */
    char *final_content = NULL;
    if (cfg->user_prompt && strlen(cfg->user_prompt) > 0) {
        size_t total_len = strlen(cfg->user_prompt) + strlen(user_input) + 4; // +4 for ": " and null terminator
        final_content = (char *)malloc(total_len);
        if (final_content) {
            snprintf(final_content, total_len, "%s: %s", cfg->user_prompt, user_input);
            cJSON_AddStringToObject(user_msg, "content", final_content);
            free(final_content);
        } else {
            /* 内存分配失败,使用原始输入 */
            cJSON_AddStringToObject(user_msg, "content", user_input);
        }
    } else {
        cJSON_AddStringToObject(user_msg, "content", user_input);
    }

    cJSON_AddItemToArray(messages, user_msg);

    cJSON_AddItemToObject(json, "messages", messages);

    /* 添加 temperature */
    cJSON_AddNumberToObject(json, "temperature", cfg->temperature);

    /* 添加 max_tokens */
    cJSON_AddNumberToObject(json, "max_tokens", cfg->max_tokens);

    /* 添加 stream (false for simplicity) */
    cJSON_AddBoolToObject(json, "stream", false);

    /* 转换为字符串 */
    char *json_string = cJSON_PrintUnformatted(json);

    /* 清理 */
    free(system_prompt);
    cJSON_Delete(json);

    return json_string;
}

bool api_send_request(const Config *cfg, const SystemInfo *sys_info,
                      const char *user_input, ApiResponse *response) {
    if (!cfg || !user_input || !response) {
        fprintf(stderr, "Error: Invalid parameters\n");
        return false;
    }

    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    WriteCallbackData write_data = {0};

    /* 初始化 curl */
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Error: Failed to initialize curl\n");
        response->error_message = strdup("Failed to initialize curl");
        return false;
    }

    /* 构建请求 URL */
    char url[512];
    snprintf(url, sizeof(url), "%s/chat/completions", cfg->endpoint);

    /* 构建请求体 */
    char *request_body = build_request_body(cfg, sys_info, user_input);
    if (!request_body) {
        fprintf(stderr, "Error: Failed to build request body\n");
        response->error_message = strdup("Failed to build request body");
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return false;
    }

    if (cfg->verbose) {
        printf("\n=== Request ===\n");
        printf("URL: %s\n", url);
        printf("Body: %s\n", request_body);
        printf("===============\n\n");
    }

    /* 设置 headers */
    char auth_header[256];
    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", cfg->api_key);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    /* 设置 curl 选项 */
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, cfg->timeout);

    /* 发送请求 */
    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        fprintf(stderr, "Error: curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        response->error_message = strdup(curl_easy_strerror(res));
        free(request_body);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return false;
    }

    /* 保存原始响应 */
    response->raw_response = write_data.data;

    if (cfg->verbose) {
        printf("\n=== Response ===\n");
        printf("%s\n", response->raw_response);
        printf("================\n\n");
    }

    /* 解析响应 */
    cJSON *json = cJSON_Parse(response->raw_response);
    if (!json) {
        fprintf(stderr, "Error: Failed to parse response JSON\n");
        response->error_message = strdup("Failed to parse response");
        free(request_body);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return false;
    }

    /* 检查错误 */
    cJSON *error = cJSON_GetObjectItem(json, "error");
    if (error) {
        cJSON *message = cJSON_GetObjectItem(error, "message");
        if (message && cJSON_IsString(message)) {
            fprintf(stderr, "API Error: %s\n", message->valuestring);
            response->error_message = strdup(message->valuestring);
        }
        cJSON_Delete(json);
        free(request_body);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return false;
    }

    /* 提取内容 */
    cJSON *choices = cJSON_GetObjectItem(json, "choices");
    if (choices && cJSON_IsArray(choices)) {
        cJSON *choice = cJSON_GetArrayItem(choices, 0);
        if (choice) {
            cJSON *message = cJSON_GetObjectItem(choice, "message");
            if (message) {
                cJSON *content = cJSON_GetObjectItem(message, "content");
                if (content && cJSON_IsString(content)) {
                    /* 解析内容，提取思考过程和命令 */
                    const char *response_text = content->valuestring;

                    /* 查找思考过程 */
                    const char *thinking_start = strstr(response_text, "**思考过程：**");
                    const char *thinking_end = strstr(response_text, "**命令：**");

                    if (thinking_start && thinking_end) {
                        thinking_start += strlen("**思考过程：**");
                        size_t thinking_len = thinking_end - thinking_start;
                        response->thinking_process = (char *)malloc(thinking_len + 1);
                        if (response->thinking_process) {
                            strncpy(response->thinking_process, thinking_start, thinking_len);
                            response->thinking_process[thinking_len] = '\0';

                            /* 去除首尾空白 */
                            char *start = response->thinking_process;
                            char *end = start + thinking_len - 1;
                            while (start < end && (*start == ' ' || *start == '\n' || *start == '\r')) start++;
                            while (end > start && (*end == ' ' || *end == '\n' || *end == '\r')) end--;
                            *(end + 1) = '\0';

                            if (start != response->thinking_process) {
                                memmove(response->thinking_process, start, strlen(start) + 1);
                            }
                        }
                    }

                    /* 查找命令 */
                    const char *cmd_start = strstr(response_text, "```bash");
                    const char *cmd_end = NULL;

                    if (cmd_start) {
                        cmd_start += strlen("```bash");
                        cmd_end = strstr(cmd_start, "```");
                    }

                    if (cmd_start && cmd_end && cmd_end > cmd_start) {
                        size_t cmd_len = cmd_end - cmd_start;
                        response->command = (char *)malloc(cmd_len + 1);
                        if (response->command) {
                            strncpy(response->command, cmd_start, cmd_len);
                            response->command[cmd_len] = '\0';

                            /* 去除首尾空白 */
                            char *start = response->command;
                            char *end = start + cmd_len - 1;
                            while (start < end && (*start == ' ' || *start == '\n' || *start == '\r')) start++;
                            while (end > start && (*end == ' ' || *end == '\n' || *end == '\r')) end--;
                            *(end + 1) = '\0';

                            if (start != response->command) {
                                memmove(response->command, start, strlen(start) + 1);
                            }
                        }
                    }

                    response->success = true;
                }
            }
        }
    }

    /* 清理 */
    cJSON_Delete(json);
    free(request_body);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    return response->success;
}
