#ifndef TRANSLATOR_APP_H
#define TRANSLATOR_APP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Call after HAL_Init/SystemClock_Config/MX_GPIO_Init/MX_CAN1_Init/MX_CAN2_Init. */
bool Translator_AppInit(void);
void Translator_AppPoll(void);

#ifdef __cplusplus
}
#endif
#endif
