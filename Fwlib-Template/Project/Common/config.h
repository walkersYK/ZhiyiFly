#ifndef __CONFIG_H
#define __CONFIG_H

#include "stdio.h"
#include "stdint.h"
#include "stm32f10x.h"

typedef struct {
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    int16_t gyroX;
    int16_t gyroY;
    int16_t gyroZ;
} _st_Mpu;

typedef struct{
	float roll;
	float pitch;
	float yaw;
}_st_AngE;

// 定义存储加速度和角速度的结构体
typedef struct {
    float x;
    float y;
    float z;
} Axis3f;


typedef struct {
    float roll;
    float pitch;
    float yaw;
} Attitude;

typedef struct {
    Attitude attitude;
    Axis3f acc;
} state_t;

#pragma pack(push, 1) // 精确对齐
typedef struct {
    int16_t THR;  // 油门值 (int16_t)
    int16_t YAW;  // 偏航值 (int16_t)
    int16_t ROL;  // 横滚值 (int16_t)
    int16_t PIT;  // 俯仰值 (int16_t)
} sReceiveRemote;
#pragma pack(pop)


/*姿态解算*/
extern _st_Mpu MPU6050;
extern _st_AngE Angle;


extern int16_t magX, magY, magZ;
extern int8_t magOffset[3],magGain[3];

#endif // __CONFIG_H

