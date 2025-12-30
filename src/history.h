/*=============================================================================
 * GLM-CMD - Conversation History Manager
 *===========================================================================*/

#ifndef HISTORY_H
#define HISTORY_H

#include <stdbool.h>

/* 对话记录结构体 */
typedef struct {
    char *user_input;      /* 用户输入 */
    char *assistant_response;  /* AI响应 */
} ConversationRound;

/* 对话历史结构体 */
typedef struct {
    char *history_file;    /* 历史文件路径 */
    ConversationRound *rounds;  /* 对话轮次数组 */
    int max_rounds;        /* 最大保存轮数 */
    int current_count;     /* 当前轮数 */
} ConversationHistory;

/* 函数声明 */
ConversationHistory* history_create(const char *config_dir, int max_rounds);
void history_destroy(ConversationHistory *history);

/* 历史管理 */
bool history_load(ConversationHistory *history);
bool history_save(const ConversationHistory *history);
bool history_add_round(ConversationHistory *history,
                      const char *user_input,
                      const char *assistant_response);

/* 历志清除 */
bool history_clear(ConversationHistory *history);

/* 辅助函数 */
char* history_to_json(const ConversationHistory *history);
void history_print(const ConversationHistory *history);

#endif /* HISTORY_H */
