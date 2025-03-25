#ifndef __HMC5881L_H
#define __HMC5881L_H

#include "config.h"
#include "Delay.h"

/**
  * HMC5883L¼Ä´æÆ÷
  */
#define HMC5883L_ADDRESS              0x3C // 0x1E << 1
#define HMC5883L_READ_ADDRESS         (0x3D)
#define HMC5883L_WRITE_ADDRESS        (0x3C)
#define HMC5883L_REG_CONFIG_A         0x00
#define HMC5883L_REG_CONFIG_B         0x01
#define HMC5883L_REG_MODE             0x02
#define HMC5883L_REG_OUT_X_M          0x03
#define HMC5883L_REG_OUT_X_L          0x04
#define HMC5883L_REG_OUT_Z_M          0x05
#define HMC5883L_REG_OUT_Z_L          0x06
#define HMC5883L_REG_OUT_Y_M          0x07
#define HMC5883L_REG_OUT_Y_L          0x08

void HMC5883L_Init(void);
void updateHMC5883(int16_t *GaX, int16_t *GaY, int16_t *GaZ);
void HMC5883L_WriteReg(uint8_t RegAddress, uint8_t Data);
uint8_t HMC5883L_ReadReg(uint8_t RegAddress);

#endif
