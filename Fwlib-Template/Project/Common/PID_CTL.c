#include "PID_CTL.h"

/* PID计算=kp*误差值 + ki * 累加（误差*T） + kd * （本次误差-上次误差）/T */

/**
 * @description: 进行PID计算
 * @param {sPID} *pid   pid结构体的地址
 * @param {float} dt    周期，单位秒
 * @return {*}
 */
void Com_PID_Update(sPID *pid, float dt)
{
    float bias = 0.0f;
    float div = 0.0f;
    /* 1、计算误差值 */
    bias = pid->measure - pid->expect;

    /* 2、计算误差的积分：累加 */
    pid->integral += bias * dt;

    /* 3、计算误差的微分：变化率 */
    div = (bias - pid->last_bias) / dt;

    /* 4、进行PID计算 */
    pid->out = pid->kp * bias + pid->ki * pid->integral + pid->kd * div;

    /* 5、保存本次误差值 */
    pid->last_bias = bias;
}

/**
 * @description: 进行串级PID计算
 * @param {sPID *} outer    外环PID
 * @param {sPID *} inner    内环PID
 * @param {float} dt        周期
 * @return {*}
 */
void Com_PID_Cascade(sPID *outer, sPID *inner, float dt)
{
    /* 外环的输出作为内环的期望值 */
    /* 1、计算外环PID */
    Com_PID_Update(outer, dt);

    /* 2、外环的输出 赋值给 内环的期望值 */
    inner->expect = outer->out;

    /* 3、计算内环PID */
    Com_PID_Update(inner, dt);
}


/**
 * @description: 批量复位PID
 * @param {sPID} *  保存pid结构体地址的数组的地址
 * @param {uint8_t} size 数组元素个数
 * @return {*}
 */
void Com_PID_Reset(sPID **pids, uint8_t size)
{
    /* 参数说明：sPID **pids
        sPID * 表示数组里的元素类型是 结构体地址
        第二个* 表示数组的地址
     */

    for (uint8_t i = 0; i < size; i++)
    {
        /* 只需要清理：积分值、上次误差值、输出 */
        pids[i] -> integral = 0.0f;
        pids[i] -> last_bias = 0.0f;
        pids[i] -> out = 0.0f;
    }
    
}
