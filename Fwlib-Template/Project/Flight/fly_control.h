#ifndef _FLYCONTROL_H
#define _FLYCONTROL_H
#include "config.h"
typedef enum {
    UNLOCK_0,
    UNLOCK_PROCESS,
    // 其他状态可以在这里添加
} Status_t;

void App_Flight_PosturePID(float dt);
void App_Flight_Motor(void);
void App_Flight_Move(void);
void App_Flight_PID_Init(void);

#define LIMIT(x, min, max) (x < min) ? min : ((x > max) ? max : x)
#endif

