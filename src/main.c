/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * Main Entry Point
 *===========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "config.h"
#include "config_init.h"
#include "system_info.h"
#include "api.h"
#include "history.h"
#include "ui.h"

#ifdef _WIN32
    #include <direct.h>
    #include <shlobj.h>
    #define mkdir_cross(path) _mkdir(path)
#else
    #include <unistd.h>
    #include <pwd.h>
    #include <sys/stat.h>
    #define mkdir_cross(path) mkdir(path, 0755)
#endif

#define VERSION "1.0.0"

/* 流式输出数据结构 */
typedef struct {
    char *reasoning_buffer;      /* 思考过程缓冲区 */
    size_t reasoning_size;       /* 思考过程缓冲区大小 */
    size_t reasoning_pos;        /* 思考过程当前位置 */

    char *answer_buffer;         /* 最终回答缓冲区 */
    size_t answer_size;          /* 最终回答缓冲区大小 */
    size_t answer_pos;           /* 最终回答当前位置 */

    bool reasoning_started;      /* 思考过程是否已开始 */
    bool answer_started;         /* 最终回答是否已开始 */
    FILE *tty;                   /* 终端文件描述符 */
} StreamUserData;

/* 辅助函数：追加内容到缓冲区 */
static void append_to_buffer(char **buffer, size_t *buffer_size,
                            size_t *buffer_pos, const char *content) {
    size_t content_len = strlen(content);

    /* 初始化缓冲区（如果需要） */
    if (*buffer == NULL) {
        *buffer_size = 4096;
        *buffer = (char *)malloc(*buffer_size);
        if (!*buffer) {
            fprintf(stderr, "Error: Failed to allocate buffer\n");
            return;
        }
        *buffer_pos = 0;
        (*buffer)[0] = '\0';
    }

    /* 扩展缓冲区（如果需要） */
    while (*buffer_pos + content_len + 1 >= *buffer_size) {
        size_t new_size = *buffer_size * 2;
        char *new_buffer = (char *)realloc(*buffer, new_size);
        if (!new_buffer) {
            fprintf(stderr, "Error: Failed to realloc buffer\n");
            return;
        }
        *buffer = new_buffer;
        *buffer_size = new_size;
    }

    /* 追加内容到缓冲区 */
    strcat(*buffer + *buffer_pos, content);
    *buffer_pos += content_len;
}

/* 流式回调函数 */
static void stream_callback(const char *content, StreamContentType content_type, void *userdata) {
    StreamUserData *data = (StreamUserData *)userdata;

    /* 处理流式结束标记 */
    if (content_type == STREAM_CONTENT_DONE) {
        if (data->reasoning_started || data->answer_started) {
            printf("\n\n");
        }
        return;
    }

    /* 处理思考过程 */
    if (content_type == STREAM_CONTENT_REASONING) {
        append_to_buffer(&data->reasoning_buffer, &data->reasoning_size,
                        &data->reasoning_pos, content);

        /* 显示标题（仅首次） */
        if (!data->reasoning_started) {
            printf("%s[✿ Thinking Process]%s\n", COLOR_CYAN, COLOR_RESET);
            data->reasoning_started = true;
        }

        /* 流式输出思考过程（灰色） */
        printf("%s%s%s", COLOR_GRAY, content, COLOR_RESET);
        fflush(stdout);
        return;
    }

    /* 处理最终回答 */
    if (content_type == STREAM_CONTENT_ANSWER) {
        append_to_buffer(&data->answer_buffer, &data->answer_size,
                        &data->answer_pos, content);

        /* 显示标题（仅首次） */
        if (!data->answer_started) {
            /* 如果思考过程已结束，先添加换行 */
            if (data->reasoning_started) {
                printf("\n");
            }
            printf("%s[✓ Generated Answer]%s\n", COLOR_GREEN, COLOR_RESET);
            data->answer_started = true;
        }

        /* 流式输出最终回答（黄色） */
        printf("%s%s%s", COLOR_YELLOW, content, COLOR_RESET);
        fflush(stdout);
        return;
    }
}

/* 打印版本信息 */
static void print_version(void) {
    printf("GLM-CMD version %s\n", VERSION);
    printf("Natural Language to Command Tool\n");
    printf("Powered by Zhipu AI (https://bigmodel.cn)\n");
    printf("\n");
    printf("Copyright (C) 2025\n");
    printf("License MIT\n");
}

/* 主函数 */
int main(int argc, char *argv[]) {
    int opt;
    bool show_help = false;
    bool show_version = false;
    bool show_info = false;
    bool show_history = false;
    bool clear_history = false;
    bool run_init = false;
    char *user_input = NULL;

    /* 命令行选项 */
    static struct option long_options[] = {
        {"help",          no_argument,       0, 'h'},
        {"version",       no_argument,       0, 'v'},
        {"verbose",       no_argument,       0, 'V'},
        {"info",          no_argument,       0, 'i'},
        {"init",          no_argument,       0, 'I'},
        {"history",       no_argument,       0,  'H'},
        {"clear-history", no_argument,       0,  'c'},
        {0, 0, 0, 0}
    };

    /* 解析命令行参数 */
    while ((opt = getopt_long(argc, argv, "hvViIHc", long_options, NULL)) != -1) {
        switch (opt) {
            case 'h':
                show_help = true;
                break;
            case 'v':
                show_version = true;
                break;
            case 'V':
                break;  /* 已在 Config 中处理 */
            case 'i':
                show_info = true;
                break;
            case 'I':
                run_init = true;
                break;
            case 'H':
                show_history = true;
                break;
            case 'c':
                clear_history = true;
                break;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }

    /* 显示版本信息 */
    if (show_version) {
        print_version();
        return 0;
    }

    /* 显示帮助信息 */
    if (show_help) {
        print_usage(argv[0]);
        return 0;
    }

    /* 运行初始化向导 */
    if (run_init) {
        if (!config_init_interactive()) {
            return 1;
        }
        return 0;
    }

    /* 显示历史记录 */
    if (show_history) {
        /* 创建配置 */
        Config *cfg = config_create();
        if (!cfg) {
            fprintf(stderr, "Error: Failed to create configuration\n");
            return 1;
        }

        /* 加载配置 */
        if (!config_load(cfg)) {
            config_destroy(cfg);
            return 1;
        }

        /* 创建历史管理器 */
        if (cfg->memory_enabled) {
            char config_dir[512];
            const char *home = getenv("HOME");
            if (home) {
                snprintf(config_dir, sizeof(config_dir), "%s/.glm-cmd", home);
                ConversationHistory *history = history_create(config_dir, cfg->memory_rounds);
                if (history) {
                    history_load(history);
                    printf("Conversation History (%d rounds):\n", history->current_count);
                    printf("========================================\n\n");
                    history_print(history);
                    history_destroy(history);
                }
            }
        } else {
            printf("Conversation memory is disabled.\n");
            printf("Enable it by setting memory_enabled=true in config.ini\n");
        }

        config_destroy(cfg);
        return 0;
    }

    /* 清除历史记录 */
    if (clear_history) {
        /* 创建配置 */
        Config *cfg = config_create();
        if (!cfg) {
            fprintf(stderr, "Error: Failed to create configuration\n");
            return 1;
        }

        /* 加载配置 */
        if (!config_load(cfg)) {
            config_destroy(cfg);
            return 1;
        }

        /* 创建历史管理器并清除 */
        char config_dir[512];
        const char *home = getenv("HOME");
        if (home) {
            snprintf(config_dir, sizeof(config_dir), "%s/.glm-cmd", home);
            ConversationHistory *history = history_create(config_dir, cfg->memory_rounds);
            if (history) {
                history_clear(history);
                printf("Conversation history cleared successfully.\n");
                history_destroy(history);
            }
        }

        config_destroy(cfg);
        return 0;
    }

    /* 创建配置 */
    Config *cfg = config_create();
    if (!cfg) {
        fprintf(stderr, "Error: Failed to create configuration\n");
        return 1;
    }

    /* 加载配置（优先级：配置文件 > 环境变量 > 默认值） */
    if (!config_load(cfg)) {
        config_destroy(cfg);
        return 1;
    }

    /* 处理 verbose 选项 */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--verbose") == 0) {
            cfg->verbose = true;
            break;
        }
    }

    /* 检测系统信息 */
    SystemInfo *sys_info = system_info_create();
    if (!sys_info) {
        fprintf(stderr, "Error: Failed to create system info\n");
        config_destroy(cfg);
        return 1;
    }

    if (!system_info_detect(sys_info)) {
        fprintf(stderr, "Warning: Failed to detect some system information\n");
    }

    /* 创建对话历史管理器（如果启用） */
    ConversationHistory *history = NULL;
    if (cfg->memory_enabled) {
        /* 获取配置目录路径 */
        const char *home = NULL;
        char config_dir[512];

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

        if (home) {
            snprintf(config_dir, sizeof(config_dir), "%s/.glm-cmd", home);
            history = history_create(config_dir, cfg->memory_rounds);
            if (history) {
                history_load(history);
            } else {
                fprintf(stderr, "Warning: Failed to create conversation history\n");
            }
        }
    }

    /* 显示系统信息 */
    if (show_info) {
        print_banner();
        system_info_print(sys_info);
        printf("\n");
        config_print(cfg);
        system_info_destroy(sys_info);
        config_destroy(cfg);
        if (history) history_destroy(history);
        return 0;
    }

    /* 获取用户输入 */
    if (optind < argc) {
        /* 从命令行参数获取输入 */
        size_t total_len = 0;
        for (int i = optind; i < argc; i++) {
            total_len += strlen(argv[i]) + 1;
        }

        user_input = (char *)malloc(total_len);
        if (!user_input) {
            fprintf(stderr, "Error: Failed to allocate memory for input\n");
            system_info_destroy(sys_info);
            config_destroy(cfg);
            return 1;
        }

        user_input[0] = '\0';
        for (int i = optind; i < argc; i++) {
            strcat(user_input, argv[i]);
            if (i < argc - 1) {
                strcat(user_input, " ");
            }
        }
    } else {
        /* 从标准输入读取 */
        print_banner();

        printf("%sEnter your command description (Ctrl+D to finish):%s\n",
               COLOR_BLUE, COLOR_RESET);
        printf("%s", COLOR_RESET);

        char buffer[4096];
        size_t total_size = 0;
        user_input = (char *)malloc(1);
        user_input[0] = '\0';

        while (fgets(buffer, sizeof(buffer), stdin)) {
            size_t buffer_len = strlen(buffer);
            char *new_input = (char *)realloc(user_input, total_size + buffer_len + 1);
            if (!new_input) {
                fprintf(stderr, "Error: Failed to allocate memory for input\n");
                free(user_input);
                system_info_destroy(sys_info);
                config_destroy(cfg);
                return 1;
            }
            user_input = new_input;
            strcat(user_input, buffer);
            total_size += buffer_len;
        }

        /* 去除末尾的换行符 */
        while (total_size > 0 &&
               (user_input[total_size - 1] == '\n' ||
                user_input[total_size - 1] == '\r')) {
            user_input[--total_size] = '\0';
        }

        printf("\n");
    }

    /* 检查输入是否为空 */
    if (!user_input || strlen(user_input) == 0) {
        print_error("No input provided");
        if (user_input) free(user_input);
        system_info_destroy(sys_info);
        config_destroy(cfg);
        return 1;
    }

    /* 显示输入 */
    if (cfg->verbose) {
        printf("\n=== Input ===\n");
        printf("%s\n", user_input);
        printf("============\n\n");
    }

    /* 发送 API 请求 */
    ApiResponse *response = api_response_create();
    if (!response) {
        fprintf(stderr, "Error: Failed to create API response\n");
        free(user_input);
        system_info_destroy(sys_info);
        config_destroy(cfg);
        return 1;
    }

    printf("%s[*] Processing your request...%s\n\n", COLOR_BLUE, COLOR_RESET);

    /* 调试: 显示历史状态 */
    if (cfg->verbose && history) {
        printf("%s[DEBUG] Conversation History: %d rounds%s\n", COLOR_YELLOW, history->current_count, COLOR_RESET);
    }

    bool success;

    /* 流式输出数据 */
    StreamUserData stream_data = {0};
    stream_data.reasoning_buffer = NULL;
    stream_data.answer_buffer = NULL;
    stream_data.reasoning_started = false;
    stream_data.answer_started = false;
    stream_data.tty = stdout;

    /* 根据配置选择使用流式或非流式 API */
    if (cfg->stream_enabled) {
        /* 使用流式 API */
        success = api_send_request_stream(cfg, sys_info, history, user_input,
                                          stream_callback, &stream_data, response);
    } else {
        /* 使用非流式 API */
        success = api_send_request(cfg, sys_info, history, user_input, response);
    }

    if (!success) {
        printf("\n");
        if (response->error_message) {
            print_error(response->error_message);
        } else {
            print_error("Failed to get response from API");
        }
        api_response_destroy(response);
        free(user_input);
        system_info_destroy(sys_info);
        if (history) history_destroy(history);
        config_destroy(cfg);
        if (stream_data.reasoning_buffer) free(stream_data.reasoning_buffer);
        if (stream_data.answer_buffer) free(stream_data.answer_buffer);
        return 1;
    }

    /* 流式模式：从 answer_buffer 提取命令 */
    if (cfg->stream_enabled && stream_data.answer_buffer) {
        if (cfg->verbose) {
            printf("[DEBUG] answer_buffer size: %zu bytes\n", stream_data.answer_pos);
            printf("[DEBUG] answer_buffer content preview (first 500 chars):\n%.500s\n[DEBUG END]\n",
                   stream_data.answer_buffer);
        }

        /* 在回答缓冲区中查找最后一个命令块（更可靠） */
        const char *last_cmd_start = NULL;
        const char *last_cmd_end = NULL;
        const char *search_pos = stream_data.answer_buffer;

        /* 查找所有 ```bash 块，取最后一个 */
        while (true) {
            const char *cmd_start = strstr(search_pos, "```bash");
            if (!cmd_start) break;

            cmd_start += strlen("```bash");  /* 跳过标记本身 */
            const char *cmd_end = strstr(cmd_start, "```");

            if (cmd_end && cmd_end > cmd_start) {
                /* 找到一个有效的代码块，记录它 */
                last_cmd_start = cmd_start;
                last_cmd_end = cmd_end;
                search_pos = cmd_end + 3;  /* 继续搜索 */
            } else if (cmd_start) {
                /* 找到了开始但没有结束标记 - 可能是不完整的响应 */
                /* 检查 cmd_start 后面是否有实际内容 */
                const char *content_start = cmd_start;
                while (content_start < cmd_start + 100 && *content_start &&
                       (*content_start == ' ' || *content_start == '\n' || *content_start == '\r')) {
                    content_start++;
                }

                if (*content_start && content_start < stream_data.answer_buffer + stream_data.answer_pos) {
                    /* 有内容但没有结束标记，使用到最后作为命令 */
                    last_cmd_start = cmd_start;
                    last_cmd_end = stream_data.answer_buffer + stream_data.answer_pos;
                    if (cfg->verbose) {
                        printf("[DEBUG] Found unclosed code block, using remaining content\n");
                        printf("[DEBUG] Command preview: %.200s\n", cmd_start);
                    }
                }
                break;
            }
        }

        if (last_cmd_start && last_cmd_end && last_cmd_end > last_cmd_start) {
            size_t cmd_len = last_cmd_end - last_cmd_start;
            char *command = (char *)malloc(cmd_len + 1);
            if (command) {
                strncpy(command, last_cmd_start, cmd_len);
                command[cmd_len] = '\0';

                /* 去除首尾空白 */
                char *start = command;
                char *end = start + cmd_len - 1;
                while (start < end && (*start == ' ' || *start == '\n' || *start == '\r')) start++;
                while (end > start && (*end == ' ' || *end == '\n' || *end == '\r')) end--;
                *(end + 1) = '\0';

                if (start != command) {
                    memmove(command, start, strlen(start) + 1);
                }

                /* 验证命令不为空且有效 */
                if (strlen(start) > 0) {
                    /* 设置命令到响应中 */
                    response->command = command;
                    if (cfg->verbose) {
                        printf("[DEBUG] Successfully extracted command: %s\n", command);
                    }
                } else {
                    if (cfg->verbose) {
                        printf("[DEBUG] Extracted command is empty after trimming\n");
                    }
                    free(command);
                }
            }
        } else {
            if (cfg->verbose) {
                printf("[DEBUG] No valid ```bash code block found in answer_buffer\n");
                printf("[DEBUG] Searched for '```bash' but found none or incomplete\n");
            }
        }
    }

    /* 保存对话到历史（如果启用） */
    if (history && response->success && response->command) {
        /* 构建完整的响应文本（包含思考过程和命令） */
        char *full_response = NULL;

        if (cfg->stream_enabled && (stream_data.reasoning_buffer || stream_data.answer_buffer)) {
            /* 流式模式：组合思考过程和回答 */
            size_t total_len = 1;  /* 至少包含 null 终止符 */
            if (stream_data.reasoning_buffer) {
                total_len += strlen(stream_data.reasoning_buffer) + 20;  /* +20 for labels */
            }
            if (stream_data.answer_buffer) {
                total_len += strlen(stream_data.answer_buffer) + 20;
            }

            full_response = (char *)malloc(total_len);
            if (full_response) {
                full_response[0] = '\0';
                if (stream_data.reasoning_buffer) {
                    strcat(full_response, "Thinking: ");
                    strcat(full_response, stream_data.reasoning_buffer);
                }
                if (stream_data.answer_buffer) {
                    if (stream_data.reasoning_buffer) {
                        strcat(full_response, "\n\n");
                    }
                    strcat(full_response, "Answer: ");
                    strcat(full_response, stream_data.answer_buffer);
                }
            }
        } else if (response->thinking_process && strlen(response->thinking_process) > 0) {
            /* 非流式模式：使用 response 中的内容 */
            size_t len = strlen(response->thinking_process) + strlen(response->command) + 20;
            full_response = (char *)malloc(len);
            if (full_response) {
                snprintf(full_response, len, "%s\n\nCommand: %s",
                        response->thinking_process, response->command);
            }
        } else {
            full_response = strdup(response->command);
        }

        if (full_response) {
            history_add_round(history, user_input, full_response);
            history_save(history);
            free(full_response);
        }
    }

    /* 清理流式缓冲区 */
    if (stream_data.reasoning_buffer) {
        free(stream_data.reasoning_buffer);
    }
    if (stream_data.answer_buffer) {
        free(stream_data.answer_buffer);
    }

    /* 显示结果（非流式模式需要显示，流式模式已经实时显示了） */
    if (!cfg->stream_enabled) {
        printf("\n");
        if (response->thinking_process) {
            print_thinking(response->thinking_process);
        }

        if (response->command) {
            print_command(response->command);
        }
    } else {
        /* 流式模式：只是显示命令部分的标题 */
        if (response->command) {
            printf("\n\n");
            print_command(response->command);
        }
    }

    if (response->command) {
        /* 询问是否执行 */
        printf("\n");
        if (ask_confirmation("Do you want to execute this command?")) {
            printf("\n");
            printf("%s[>] Executing command...%s\n\n", COLOR_GREEN, COLOR_RESET);
            int ret = system(response->command);
            printf("\n");
            if (ret == 0) {
                print_success("Command executed successfully");
            } else {
                printf("%s[!] Command exited with code: %d%s\n",
                       COLOR_YELLOW, ret, COLOR_RESET);
            }
        } else {
            print_info("Command execution cancelled");
        }
    } else {
        print_error("Failed to extract command from response");
        if (cfg->verbose && response->raw_response) {
            printf("\nRaw response:\n%s\n", response->raw_response);
        }
        api_response_destroy(response);
        free(user_input);
        system_info_destroy(sys_info);
        if (history) history_destroy(history);
        config_destroy(cfg);
        return 1;
    }

    /* 清理 */
    api_response_destroy(response);
    free(user_input);
    system_info_destroy(sys_info);
    if (history) history_destroy(history);
    config_destroy(cfg);

    return 0;
}
