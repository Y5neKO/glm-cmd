/*=============================================================================
 * GLM-CMD - Configuration Initialization
 *===========================================================================*/

#ifndef CONFIG_INIT_H
#define CONFIG_INIT_H

#include <stdbool.h>

/* 函数声明 */
bool config_init_interactive(void);
bool config_init_with_values(const char *api_key, const char *endpoint,
                              const char *model, const char *user_prompt,
                              double temperature, int max_tokens, int timeout);

#endif /* CONFIG_INIT_H */
