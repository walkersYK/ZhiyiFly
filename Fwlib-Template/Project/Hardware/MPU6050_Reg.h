#ifndef __MPU6050_REG_H
#define __MPU6050_REG_H

#define	MPU6050_SMPLRT_DIV		0x19
#define	MPU6050_CONFIG			0x1A
#define	MPU6050_GYRO_CONFIG		0x1B
#define	MPU6050_ACCEL_CONFIG	0x1C

#define	MPU6050_ACCEL_XOUT_H	0x3B
#define	MPU6050_ACCEL_XOUT_L	0x3C
#define	MPU6050_ACCEL_YOUT_H	0x3D
#define	MPU6050_ACCEL_YOUT_L	0x3E
#define	MPU6050_ACCEL_ZOUT_H	0x3F
#define	MPU6050_ACCEL_ZOUT_L	0x40
#define	MPU6050_TEMP_OUT_H		0x41
#define	MPU6050_TEMP_OUT_L		0x42
#define	MPU6050_GYRO_XOUT_H		0x43
#define	MPU6050_GYRO_XOUT_L		0x44
#define	MPU6050_GYRO_YOUT_H		0x45
#define	MPU6050_GYRO_YOUT_L		0x46
#define	MPU6050_GYRO_ZOUT_H		0x47
#define	MPU6050_GYRO_ZOUT_L		0x48

#define	MPU6050_PWR_MGMT_1		0x6B
#define	MPU6050_PWR_MGMT_2		0x6C
#define	MPU6050_WHO_AM_I		0x75

#define MPU6050_INT_PIN_CFG 		0X37
#define MPU6050_USER_CTRL       	0X6A //����ģʽ
#define MPU6050_I2C_MST_CTRL    	0X24
#define MPU6050_I2C_SLV0_ADDR   	0X25
#define MPU6050_I2C_SLV0_REG    	0X26
#define MPU6050_I2C_SLV0_CTRL   	0X27


#define	MPU6050_EXT_SENS_DATA_00	0X49
#define	MPU6050_EXT_SENS_DATA_01	0X4A
#define	MPU6050_EXT_SENS_DATA_02	0X4B
#define	MPU6050_EXT_SENS_DATA_03	0X4C
#define	MPU6050_EXT_SENS_DATA_04	0X4D
#define	MPU6050_EXT_SENS_DATA_05	0X4E

#endif
