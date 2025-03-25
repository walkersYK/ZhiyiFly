#ifndef __IMU_H
#define __IMU_H
#include <stdbool.h>
#include "MPU6050.h"
#include "config.h"

#define squa(Sq)(((float)Sq) * ((float)Sq))
#define DEG2RAD		0.017453293f	/* 度转弧度 π/180 */
#define RAD2DEG		57.29578f		/* 弧度转度 180/π */

/************************ 
 * 6轴数据融合函数	
*************************/
void imuUpdate(const _st_Mpu *pMpu, _st_AngE *state , float dt);
void updateYaw(_st_AngE *state);

float GetAccz(void);
#endif
