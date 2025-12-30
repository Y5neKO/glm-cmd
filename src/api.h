/*=============================================================================
 * GLM-CMD - Natural Language to Command Tool
 * API Client Header
 *===========================================================================*/

#ifndef API_H
#define API_H

#include "config.h"
#include "system_info.h"
#include "history.h"
#include <stdbool.h>

/* 流式回调函数类型
 * content: 接收到的内容片段
 * is_done: 是否为最后一个片段
 * userdata: 用户自定义数据
 */
typedef void (*StreamCallback)(const char *content, bool is_done, void *userdata);

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
                      const ConversationHistory *history,
                      const char *user_input, ApiResponse *response);
bool api_send_request_stream(const Config *cfg, const SystemInfo *sys_info,
                              const ConversationHistory *history,
                              const char *user_input, StreamCallback callback,
                              void *userdata, ApiResponse *response);
char* build_system_prompt(const SystemInfo *sys_info);
char* build_request_body(const Config *cfg, const SystemInfo *sys_info,
                         const ConversationHistory *history,
                         const char *user_input);
char* build_request_body_stream(const Config *cfg, const SystemInfo *sys_info,
                                 const ConversationHistory *history,
                                 const char *user_input);

#endif /* API_H */
