#ifndef __PID_CTL_H__
#define __PID_CTL_H__

#include "stm32f10x.h" 

typedef struct
{
    float kp;        // P系数
    float ki;        // I系数
    float kd;        // D系数
    float measure;   // 测量值
    float expect;    // 期望值
    float integral;  // 误差的积分
    float last_bias; // 上次的误差值
    float out;       // pid计算结果
} sPID;


void Com_PID_Update(sPID *pid, float dt);

void Com_PID_Cascade(sPID *outer, sPID *inner, float dt);

void Com_PID_Reset(sPID **pids, uint8_t size);

#endif /* __COM_PID_H__ */
