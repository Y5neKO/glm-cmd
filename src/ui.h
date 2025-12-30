/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * UI Utilities Header
 *===========================================================================*/

#ifndef UI_H
#define UI_H

#include <stdbool.h>

/* 颜色代码 */
#ifdef _WIN32
    #define COLOR_RESET   ""
    #define COLOR_RED     ""
    #define COLOR_GREEN   ""
    #define COLOR_YELLOW  ""
    #define COLOR_BLUE    ""
    #define COLOR_MAGENTA ""
    #define COLOR_CYAN    ""
    #define COLOR_GRAY    ""
#else
    #define COLOR_RESET   "\033[0m"
    #define COLOR_RED     "\033[91m"
    #define COLOR_GREEN   "\033[92m"
    #define COLOR_YELLOW  "\033[93m"
    #define COLOR_BLUE    "\033[94m"
    #define COLOR_MAGENTA "\033[95m"
    #define COLOR_CYAN    "\033[96m"
    #define COLOR_GRAY    "\033[90m"
#endif

/* 函数声明 */
void print_banner(void);
void print_thinking(const char *thinking);
void print_command(const char *command);
void print_error(const char *message);
void print_success(const char *message);
void print_info(const char *message);
void print_warning(const char *message);
bool ask_confirmation(const char *prompt);
void print_usage(const char *program_name);

#endif /* UI_H */
