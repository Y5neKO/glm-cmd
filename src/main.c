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
    bool run_init = false;
    char *user_input = NULL;

    /* 命令行选项 */
    static struct option long_options[] = {
        {"help",      no_argument,       0, 'h'},
        {"version",   no_argument,       0, 'v'},
        {"verbose",   no_argument,       0, 'V'},
        {"info",      no_argument,       0, 'i'},
        {"init",      no_argument,       0, 'I'},
        {0, 0, 0, 0}
    };

    /* 解析命令行参数 */
    while ((opt = getopt_long(argc, argv, "hvViI", long_options, NULL)) != -1) {
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

    printf("%s🤔 Processing your request...%s\n\n", COLOR_BLUE, COLOR_RESET);

    /* 调试: 显示历史状态 */
    if (cfg->verbose && history) {
        printf("%s[DEBUG] Conversation History: %d rounds%s\n", COLOR_YELLOW, history->current_count, COLOR_RESET);
    }

    bool success = api_send_request(cfg, sys_info, history, user_input, response);

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
        return 1;
    }

    /* 保存对话到历史（如果启用） */
    if (history && response->success && response->command) {
        /* 构建完整的响应文本（包含思考过程和命令） */
        char *full_response = NULL;
        if (response->thinking_process && strlen(response->thinking_process) > 0) {
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

    /* 显示结果 */
    printf("\n");
    if (response->thinking_process) {
        print_thinking(response->thinking_process);
    }

    if (response->command) {
        print_command(response->command);

        /* 询问是否执行 */
        printf("\n");
        if (ask_confirmation("Do you want to execute this command?")) {
            printf("\n");
            printf("%s▶️ Executing command...%s\n\n", COLOR_GREEN, COLOR_RESET);
            int ret = system(response->command);
            printf("\n");
            if (ret == 0) {
                print_success("Command executed successfully");
            } else {
                printf("%s⚠️  Command exited with code: %d%s\n",
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
