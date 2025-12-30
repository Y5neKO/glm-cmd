/*=============================================================================
 * GLM-CMD - Configuration Initialization Implementation
 *===========================================================================*/

#include "config_init.h"
#include "config_parser.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WIN32
    #include <direct.h>
    #include <shlobj.h>
    #include <windows.h>
#else
    #include <sys/stat.h>
    #include <pwd.h>
#endif

/* 获取配置文件路径 */
static bool get_config_path(char *path, size_t path_size) {
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

    if (!home) {
        fprintf(stderr, "Error: Unable to determine home directory\n");
        return false;
    }

    snprintf(path, path_size, "%s/.glm-cmd/config.ini", home);
    return true;
}

/* 交互式读取输入 */
static bool read_input(const char *prompt, char *buffer, size_t buffer_size, bool show_hint) {
    if (show_hint) {
        printf("%s", prompt);
        fflush(stdout);
    }

    if (fgets(buffer, buffer_size, stdin) == NULL) {
        return false;
    }

    /* 去除换行符 */
    size_t len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
        buffer[--len] = '\0';
    }

    return true;
}

bool config_init_with_values(const char *api_key, const char *endpoint,
                              const char *model, const char *user_prompt,
                              double temperature, int max_tokens, int timeout) {
    char config_path[CONFIG_MAX_PATH];

    /* 获取配置文件路径 */
    if (!get_config_path(config_path, sizeof(config_path))) {
        return false;
    }

    /* 检查配置文件是否已存在 */
#ifdef _WIN32
    struct _stat st;
    if (_stat(config_path, &st) == 0) {
#else
    struct stat st;
    if (stat(config_path, &st) == 0) {
#endif
        print_warning("Configuration file already exists");
        printf("Location: %s\n", config_path);

        if (!ask_confirmation("Do you want to overwrite it?")) {
            print_info("Initialization cancelled");
            return false;
        }
    }

    /* 创建配置目录 */
    if (!config_file_create_directory(config_path)) {
        print_error("Failed to create configuration directory");
        return false;
    }

    /* 创建配置文件内容 */
    ConfigFile *cfg = config_file_create();
    if (!cfg) {
        print_error("Failed to allocate memory for configuration");
        return false;
    }

    /* 设置值 */
    cfg->api_key = strdup(api_key);
    cfg->endpoint = strdup(endpoint);
    cfg->model = strdup(model ? model : "glm-4.7");
    cfg->user_prompt = user_prompt ? strdup(user_prompt) : NULL;
    cfg->temperature = temperature;
    cfg->max_tokens = max_tokens;
    cfg->timeout = timeout;

    /* 写入配置文件 */
    if (!config_file_write(config_path, cfg)) {
        print_error("Failed to write configuration file");
        config_file_destroy(cfg);
        return false;
    }

    config_file_destroy(cfg);

    printf("\n");
    print_success("Configuration initialized successfully!");
    printf("\n");
    printf("Configuration file created at:\n");
    printf("  %s\n", config_path);
    printf("\n");
    printf("You can now use glm-cmd directly:\n");
    printf("  glm-cmd \"your command description\"\n");
    printf("\n");
    printf("To modify configuration, edit the file above.\n");
    printf("For more information, run: glm-cmd --help\n");
    printf("\n");

    return true;
}

bool config_init_interactive(void) {
    char api_key[256];
    char endpoint[256];
    char model[64] = "glm-4.7";
    char user_prompt[512] = "";
    char temp_input[16];
    double temperature = 0.7;
    int max_tokens = 2048;
    int timeout = 30;

    /* 显示欢迎信息 */
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║  GLM-CMD Configuration Wizard                            ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("This wizard will help you set up your GLM-CMD configuration.\n");
    printf("\n");

    /* 输入 API Key */
    printf("Step 1/6: API Key\n");
    printf("─────────────────────────────────────────\n");
    printf("Get your API key from: https://bigmodel.cn\n");
    printf("\n");

    while (1) {
        if (!read_input("Enter your API Key: ", api_key, sizeof(api_key), true)) {
            print_error("Failed to read input");
            return false;
        }

        if (strlen(api_key) > 0) {
            break;
        }

        print_error("API Key cannot be empty. Please try again.\n");
    }

    printf("\n");

    /* 选择端点 */
    printf("Step 2/6: API Endpoint\n");
    printf("─────────────────────────────────────────\n");
    printf("Select an endpoint:\n");
    printf("\n");
    printf("  [1] Coding Endpoint (Recommended)\n");
    printf("      https://open.bigmodel.cn/api/coding/paas/v4\n");
    printf("      Use for: Code generation, command-line tools, scripts\n");
    printf("\n");
    printf("  [2] Standard Endpoint\n");
    printf("      https://open.bigmodel.cn/api/paas/v4\n");
    printf("      Use for: General tasks, text processing, translation\n");
    printf("\n");
    printf("  [3] Custom Endpoint\n");
    printf("      Enter your own endpoint URL\n");
    printf("\n");

    while (1) {
        char input[16];
        if (!read_input("Choose endpoint [1/2/3] (default: 1): ", input, sizeof(input), true)) {
            print_error("Failed to read input");
            return false;
        }

        if (strlen(input) == 0 || strcmp(input, "1") == 0) {
            strcpy(endpoint, "https://open.bigmodel.cn/api/coding/paas/v4");
            printf("Using: Coding Endpoint\n");
            break;
        } else if (strcmp(input, "2") == 0) {
            strcpy(endpoint, "https://open.bigmodel.cn/api/paas/v4");
            printf("Using: Standard Endpoint\n");
            break;
        } else if (strcmp(input, "3") == 0) {
            while (1) {
                if (!read_input("Enter custom endpoint URL: ", endpoint, sizeof(endpoint), true)) {
                    print_error("Failed to read input");
                    return false;
                }

                if (strlen(endpoint) > 0) {
                    printf("Using: %s\n", endpoint);
                    break;
                }

                print_error("Endpoint cannot be empty. Please try again.\n");
            }
            break;
        } else {
            print_warning("Invalid choice. Please enter 1, 2 or 3.");
        }
    }

    printf("\n");

    /* 输入模型名称 */
    printf("Step 3/6: Model Name\n");
    printf("─────────────────────────────────────────\n");
    printf("Enter the model name you want to use.\n");
    printf("\n");
    printf("Common models:\n");
    printf("  - glm-4.7 (Standard model)\n");
    printf("  - glm-4-plus (Enhanced capabilities)\n");
    printf("  - glm-4-flash (Fast response)\n");
    printf("  - glm-4-air (Lightweight)\n");
    printf("\n");
    printf("You can use any available model name.\n");
    printf("\n");

    while (1) {
        if (!read_input("Enter model name: ", model, sizeof(model), true)) {
            print_error("Failed to read input");
            return false;
        }

        if (strlen(model) > 0) {
            printf("Using: %s\n", model);
            break;
        }

        print_error("Model name cannot be empty. Please try again.\n");
    }

    printf("\n");

    /* 设置温度参数 */
    printf("Step 4/6: Temperature\n");
    printf("─────────────────────────────────────────\n");
    printf("Temperature controls the randomness of the response.\n");
    printf("\n");
    printf("  - Lower (0.0 - 0.3): More focused and deterministic\n");
    printf("  - Medium (0.4 - 0.8): Balanced (default: 0.7)\n");
    printf("  - Higher (0.9 - 2.0): More creative and random\n");
    printf("\n");

    while (1) {
        if (!read_input("Enter temperature [0.0-2.0] (default: 0.7): ", temp_input, sizeof(temp_input), true)) {
            print_error("Failed to read input");
            return false;
        }

        if (strlen(temp_input) == 0) {
            temperature = 0.7;
            printf("Using: 0.7 (default)\n");
            break;
        }

        temperature = atof(temp_input);
        if (temperature >= 0.0 && temperature <= 2.0) {
            printf("Using: %.1f\n", temperature);
            break;
        }

        print_warning("Invalid temperature. Please enter a value between 0.0 and 2.0.\n");
    }

    printf("\n");

    /* 设置最大 tokens */
    printf("Step 5/6: Max Tokens\n");
    printf("─────────────────────────────────────────\n");
    printf("Maximum number of tokens in the API response.\n");
    printf("\n");
    printf("  - Recommended: 2048 (default)\n");
    printf("  - Long responses: 4096\n");
    printf("  - Short responses: 1024\n");
    printf("\n");

    while (1) {
        if (!read_input("Enter max tokens (default: 2048): ", temp_input, sizeof(temp_input), true)) {
            print_error("Failed to read input");
            return false;
        }

        if (strlen(temp_input) == 0) {
            max_tokens = 2048;
            printf("Using: 2048 (default)\n");
            break;
        }

        max_tokens = atoi(temp_input);
        if (max_tokens > 0 && max_tokens <= 32768) {
            printf("Using: %d\n", max_tokens);
            break;
        }

        print_warning("Invalid value. Please enter a number between 1 and 32768.\n");
    }

    printf("\n");

    /* 设置超时时间 */
    printf("Step 6/6: Request Timeout\n");
    printf("─────────────────────────────────────────\n");
    printf("Request timeout in seconds.\n");
    printf("\n");
    printf("  - Fast: 10 seconds\n");
    printf("  - Normal: 30 seconds (default)\n");
    printf("  - Slow: 60 seconds\n");
    printf("\n");

    while (1) {
        if (!read_input("Enter timeout in seconds (default: 30): ", temp_input, sizeof(temp_input), true)) {
            print_error("Failed to read input");
            return false;
        }

        if (strlen(temp_input) == 0) {
            timeout = 30;
            printf("Using: 30 seconds (default)\n");
            break;
        }

        timeout = atoi(temp_input);
        if (timeout > 0 && timeout <= 300) {
            printf("Using: %d seconds\n", timeout);
            break;
        }

        print_warning("Invalid value. Please enter a number between 1 and 300.\n");
    }

    /* 用户自定义提示词 */
    printf("\n");
    printf("─────────────────────────────────────────\n");
    printf("User Custom Prompt\n");
    printf("─────────────────────────────────────────\n");
    printf("You can add a custom prompt that will be prefixed to every user input.\n");
    printf("This is useful for adding context or specific instructions.\n");
    printf("Leave empty if you don't need this feature.\n");
    printf("\n");
    printf("Examples:\n");
    printf("  - \"Always explain the command before showing it\"\n");
    printf("  - \"Generate bash commands only\"\n");
    printf("  - \"Use safe options only, never destructive commands\"\n");
    printf("\n");

    if (!read_input("Enter your custom prompt (optional, press Enter to skip): ",
                    user_prompt, sizeof(user_prompt), true)) {
        print_error("Failed to read input");
        return false;
    }

    if (strlen(user_prompt) == 0) {
        printf("No custom prompt configured.\n");
    } else {
        printf("Custom prompt: %s\n", user_prompt);
    }

    printf("\n");

    /* 确认配置 */
    printf("Configuration Summary:\n");
    printf("─────────────────────────────────────────\n");
    printf("API Key:       %.*s***%s\n", 4, api_key, api_key + strlen(api_key) - 4);
    printf("Endpoint:      %s\n", endpoint);
    printf("Model:         %s\n", model);
    if (strlen(user_prompt) > 0) {
        printf("User Prompt:   %s\n", user_prompt);
    }
    printf("Temperature:   %.1f\n", temperature);
    printf("Max Tokens:    %d\n", max_tokens);
    printf("Timeout:       %d seconds\n", timeout);
    printf("\n");

    if (!ask_confirmation("Save this configuration?")) {
        print_info("Initialization cancelled");
        return false;
    }

    /* 保存配置 */
    return config_init_with_values(api_key, endpoint, model,
                                    strlen(user_prompt) > 0 ? user_prompt : NULL,
                                    temperature, max_tokens, timeout);
}
