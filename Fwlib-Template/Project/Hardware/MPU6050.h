#ifndef __MPU6050_H
#define __MPU6050_H

#include "config.h"


// ����ȫ�ֱ������洢���ٶȺͽ��ٶ�
extern Axis3f accData;  // ���ٶ�����
extern Axis3f gyroData; // ���ٶ�����
extern  uint8_t BUF[8];


void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);
uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf);

void MPU6050_Init(void);
uint8_t MPU6050_GetID(void);
void MPU6050_PassMode(void);
void MPU6050_MasMode(void);
void MPU6050_SlaveRead(void);


void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);
void MPU6050_GetAccelData(int16_t *AccX, int16_t *AccY, int16_t *AccZ);
void MPU6050_GetGyroData(int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ);
void MpuGetData(void);
void GetOffSets(void);

#endif
