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

// ����洢���ٶȺͽ��ٶȵĽṹ��
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

#pragma pack(push, 1) // ��ȷ����
typedef struct {
    int16_t THR;  // ����ֵ (int16_t)
    int16_t YAW;  // ƫ��ֵ (int16_t)
    int16_t ROL;  // ���ֵ (int16_t)
    int16_t PIT;  // ����ֵ (int16_t)
} sReceiveRemote;
#pragma pack(pop)


/*��̬����*/
extern _st_Mpu MPU6050;
extern _st_AngE Angle;


extern int16_t magX, magY, magZ;
extern int8_t magOffset[3],magGain[3];

#endif // __CONFIG_H

