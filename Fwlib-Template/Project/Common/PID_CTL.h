#ifndef __PID_CTL_H__
#define __PID_CTL_H__

#include "stm32f10x.h" 

typedef struct
{
    float kp;        // Pϵ��
    float ki;        // Iϵ��
    float kd;        // Dϵ��
    float measure;   // ����ֵ
    float expect;    // ����ֵ
    float integral;  // ���Ļ���
    float last_bias; // �ϴε����ֵ
    float out;       // pid������
} sPID;


void Com_PID_Update(sPID *pid, float dt);

void Com_PID_Cascade(sPID *outer, sPID *inner, float dt);

void Com_PID_Reset(sPID **pids, uint8_t size);

#endif /* __COM_PID_H__ */
