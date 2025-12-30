/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * API Client Header
 *===========================================================================*/

#ifndef API_H
#define API_H

#include "config.h"
#include "system_info.h"
#include <stdbool.h>

/* API 响应结构体 */
typedef struct {
    char *raw_response;
    char *thinking_process;
    char *command;
    bool success;
    char *error_message;
} ApiResponse;

/* 函数声明 */
ApiResponse* api_response_create(void);
void api_response_destroy(ApiResponse *response);
bool api_send_request(const Config *cfg, const SystemInfo *sys_info,
                      const char *user_input, ApiResponse *response);
char* build_system_prompt(const SystemInfo *sys_info);
char* build_request_body(const Config *cfg, const SystemInfo *sys_info,
                         const char *user_input);

#endif /* API_H */
