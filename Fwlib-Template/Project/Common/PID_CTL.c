#include "PID_CTL.h"

/* PID����=kp*���ֵ + ki * �ۼӣ����*T�� + kd * ���������-�ϴ���/T */

/**
 * @description: ����PID����
 * @param {sPID} *pid   pid�ṹ��ĵ�ַ
 * @param {float} dt    ���ڣ���λ��
 * @return {*}
 */
void Com_PID_Update(sPID *pid, float dt)
{
    float bias = 0.0f;
    float div = 0.0f;
    /* 1���������ֵ */
    bias = pid->measure - pid->expect;

    /* 2���������Ļ��֣��ۼ� */
    pid->integral += bias * dt;

    /* 3����������΢�֣��仯�� */
    div = (bias - pid->last_bias) / dt;

    /* 4������PID���� */
    pid->out = pid->kp * bias + pid->ki * pid->integral + pid->kd * div;

    /* 5�����汾�����ֵ */
    pid->last_bias = bias;
}

/**
 * @description: ���д���PID����
 * @param {sPID *} outer    �⻷PID
 * @param {sPID *} inner    �ڻ�PID
 * @param {float} dt        ����
 * @return {*}
 */
void Com_PID_Cascade(sPID *outer, sPID *inner, float dt)
{
    /* �⻷�������Ϊ�ڻ�������ֵ */
    /* 1�������⻷PID */
    Com_PID_Update(outer, dt);

    /* 2���⻷����� ��ֵ�� �ڻ�������ֵ */
    inner->expect = outer->out;

    /* 3�������ڻ�PID */
    Com_PID_Update(inner, dt);
}


/**
 * @description: ������λPID
 * @param {sPID} *  ����pid�ṹ���ַ������ĵ�ַ
 * @param {uint8_t} size ����Ԫ�ظ���
 * @return {*}
 */
void Com_PID_Reset(sPID **pids, uint8_t size)
{
    /* ����˵����sPID **pids
        sPID * ��ʾ�������Ԫ�������� �ṹ���ַ
        �ڶ���* ��ʾ����ĵ�ַ
     */

    for (uint8_t i = 0; i < size; i++)
    {
        /* ֻ��Ҫ��������ֵ���ϴ����ֵ����� */
        pids[i] -> integral = 0.0f;
        pids[i] -> last_bias = 0.0f;
        pids[i] -> out = 0.0f;
    }
    
}
