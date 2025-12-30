/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * UI Utilities Implementation
 *===========================================================================*/

#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void print_banner(void) {
    printf("\n");
    printf("%s", COLOR_CYAN);
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║                                                          ║\n");
    printf("║   GLM-CMD - Natural Language to Command Tool             ║\n");
    printf("║   Convert natural language to shell commands             ║\n");
    printf("║                                                          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("%s", COLOR_RESET);
    printf("\n");
}

void print_thinking(const char *thinking) {
    if (!thinking || strlen(thinking) == 0) return;

    printf("%s[Thinking Process]%s\n", COLOR_BLUE, COLOR_RESET);
    printf("%s%s%s\n", COLOR_GRAY, thinking, COLOR_RESET);
    printf("\n");
}

void print_command(const char *command) {
    if (!command || strlen(command) == 0) {
        printf("%s[Error] No command generated%s\n", COLOR_RED, COLOR_RESET);
        return;
    }

    printf("%s─────────────────────────────────────────────────────────%s\n", COLOR_CYAN, COLOR_RESET);
    printf("%s[Generated Command]%s\n", COLOR_GREEN, COLOR_RESET);
    printf("\n");
    printf("%s%s%s\n", COLOR_YELLOW, command, COLOR_RESET);
    printf("\n");
    printf("%s─────────────────────────────────────────────────────────%s\n", COLOR_CYAN, COLOR_RESET);
}

void print_error(const char *message) {
    if (!message) return;
    fprintf(stderr, "%s[Error] %s%s\n", COLOR_RED, message, COLOR_RESET);
}

void print_success(const char *message) {
    if (!message) return;
    printf("%s[Success] %s%s\n", COLOR_GREEN, message, COLOR_RESET);
}

void print_info(const char *message) {
    if (!message) return;
    printf("%s[Info] %s%s\n", COLOR_BLUE, message, COLOR_RESET);
}

void print_warning(const char *message) {
    if (!message) return;
    printf("%s[Warning] %s%s\n", COLOR_YELLOW, message, COLOR_RESET);
}

bool ask_confirmation(const char *prompt) {
    char buffer[16];

    printf("%s%s %s[y/N]: %s", COLOR_YELLOW, prompt, COLOR_RESET, COLOR_RESET);
    fflush(stdout);

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return false;
    }

    /* 去除换行符 */
    size_t len = strlen(buffer);
    while (len > 0 && (buffer[len - 1] == '\n' || buffer[len - 1] == '\r')) {
        buffer[--len] = '\0';
    }

    /* 转换为小写 */
    for (size_t i = 0; i < len; i++) {
        buffer[i] = tolower(buffer[i]);
    }

    return (strcmp(buffer, "y") == 0 || strcmp(buffer, "yes") == 0);
}

void print_usage(const char *program_name) {
    printf("Usage: %s [OPTIONS] \"natural language query\"\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -v, --version           Show version information\n");
    printf("  -V, --verbose           Enable verbose output\n");
    printf("  -i, --info              Show system and configuration information\n");
    printf("  -I, --init              Initialize configuration (interactive wizard)\n");
    printf("  -H, --history           Show conversation history\n");
    printf("  -c, --clear-history     Clear conversation history\n");
    printf("\n");
    printf("Environment Variables:\n");
    printf("  GLM_CMD_API_KEY         API key for Zhipu AI (required)\n");
    printf("  GLM_CMD_MODEL           Model name (default: glm-4.7)\n");
    printf("  GLM_CMD_ENDPOINT        API endpoint (required)\n");
    printf("  GLM_CMD_TEMP            Temperature (default: 0.7)\n");
    printf("  GLM_CMD_MAX_TOKENS      Max tokens (default: 2048)\n");
    printf("  GLM_CMD_TIMEOUT         Timeout in seconds (default: 30)\n");
    printf("\n");
    printf("Examples:\n");
    printf("  %s \"List all files larger than 100MB\"\n", program_name);
    printf("  %s \"Find processes using port 8080\"\n", program_name);
    printf("  %s \"Rename all files to lowercase\"\n", program_name);
    printf("  %s --init               # Run configuration wizard\n", program_name);
    printf("  %s --history             # Show conversation history\n", program_name);
    printf("  %s --clear-history       # Clear conversation history\n", program_name);
    printf("\n");
    printf("For more information, visit: https://github.com/Y5neKO/glm-cmd\n");
}
