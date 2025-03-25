#include "MyI2C.h"
#include "HMC5883L.h"
#include "stm32f10x.h"    
#include "stdio.h"   

uint8_t BUF[8];
int16_t magX, magY, magZ;
int8_t magOffset[3],magGain[3];
/**
  * 函    数：HMC5883L初始化
  * 参    数：无
  * 返 回 值：无
  * 注意事项：无
  */
void HMC5883L_Init(void)
{
	HMC5883L_WriteReg(HMC5883L_REG_CONFIG_A, 0X70); //// 设置平均样本数、输出速率
	HMC5883L_WriteReg(HMC5883L_REG_CONFIG_B, 0X20); // 设置增益
	HMC5883L_WriteReg(HMC5883L_REG_MODE, 0X00);
}

void Multiple_read_HMC5883(void)
{   uint8_t i;
    MyI2C_Start();                          //起始信号
    MyI2C_SendByte(HMC5883L_ADDRESS);           //发送设备地址+写信号
    MyI2C_SendByte(0x03);                   //发送存储单元地址，从0x3开始	
    MyI2C_Start();                          //起始信号
    MyI2C_SendByte(HMC5883L_ADDRESS+1);         //发送设备地址+读信号
	 for (i=0; i<6; i++)                      //连续读取6个地址数据，存储中BUF
    {
        BUF[i] = MyI2C_ReceiveByte();          //BUF[0]存储数据
        if (i == 5)
        {
           MyI2C_SendAck(1);                //最后一个数据需要回NOACK
        }
        else
        {
          MyI2C_SendAck(0);                //回应ACK
       }
   }
    MyI2C_Stop();                          //停止信号
    Delay_ms(5);
}

//**************************************
//从HMC5883连续读取6个数据放在BUF中
//**************************************
void updateHMC5883(int16_t *GaX, int16_t *GaY, int16_t *GaZ)
{
//			Multiple_read_HMC5883();      //连续读出数据，存储在BUF中
//			//---------显示X轴
//			*GaX=BUF[0] << 8 | BUF[1]; //Combine MSB and LSB of X Data output register
//			*GaY=BUF[2] << 8 | BUF[3]; //Combine MSB and LSB of Z Data output register
//			*GaZ=BUF[4] << 8 | BUF[5]; //Combine MSB and LSB of Y Data output register
		uint8_t DataH, DataL;
		Delay_ms(20);
		DataH = HMC5883L_ReadReg(HMC5883L_REG_OUT_X_M);	
		DataL = HMC5883L_ReadReg(HMC5883L_REG_OUT_X_L);		
		*GaX = (DataH << 8) | DataL;						
		Delay_ms(20);
		DataH = HMC5883L_ReadReg(HMC5883L_REG_OUT_Y_M);		
		DataL = HMC5883L_ReadReg(HMC5883L_REG_OUT_Y_L);		
		*GaY = (DataH << 8) | DataL;						
		Delay_ms(20);
		DataH = HMC5883L_ReadReg(HMC5883L_REG_OUT_Z_M);	
		DataL = HMC5883L_ReadReg(HMC5883L_REG_OUT_Z_L);		
		*GaZ = (DataH << 8) | DataL;	
}

//指数移动平均值处理和过滤磁力数据
#define alpha 0.2
float EMA(float rawValue, float filteredValue)
{
    return alpha * rawValue + (1 - alpha) * filteredValue;
}
/**************************
  * function       ：HMC5883L写寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  ************************/
void HMC5883L_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(HMC5883L_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(Data);				//发送要写入寄存器的数据
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_Stop();						//I2C终止
}
/**
  * 函    数：HMC5883L读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
uint8_t HMC5883L_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(HMC5883L_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	
	MyI2C_Start();						//I2C重复起始
	MyI2C_SendByte(HMC5883L_ADDRESS | 0x01);	//发送从机地址，读写位为1，表示即将读取
	MyI2C_ReceiveAck();					//接收应答
	Data = MyI2C_ReceiveByte();			//接收指定寄存器的数据
	MyI2C_SendAck(1);					//发送应答，给从机非应答，终止从机的数据输出
	MyI2C_Stop();						//I2C终止
	
	return Data;
}
/************
校准
*************/
int max(int a, int b) {  
    return (a > b) ? a : b;  
}
int min(int a, int b) {  
    return (a < b) ? a : b;  
}
#define DESIRED_RANGE 100.0f // ±50μT, total range is 100μT (from -50μT to +50μT)
void calibrateMag() {
    // ... [初始化传感器] ...

    int16_t magPosOff[3], magNegOff[3];
    float avgMag[3] = {0, 0, 0};
    const int samples = 100; // 收集的样本数量

    for (int i = 0; i < samples; ++i) {
        updateHMC5883(&magX, &magY, &magZ);
        avgMag[0] += magX;
        avgMag[1] += magY;
        avgMag[2] += magZ;
        Delay_ms(10); // 给传感器时间稳定
    }

    // 计算平均值
    for (int i = 0; i < 3; ++i) {
        avgMag[i] /= samples;
    }

    // 硬铁偏移即为平均值
    magOffset[0] = avgMag[0];
    magOffset[1] = avgMag[1];
    magOffset[2] = avgMag[2];

    // 找到最大最小值用于软铁校准
    magPosOff[0] = magPosOff[1] = magPosOff[2] = INT16_MIN;
    magNegOff[0] = magNegOff[1] = magNegOff[2] = INT16_MAX;

    for (int i = 0; i < samples; ++i) {
        updateHMC5883(&magX, &magY, &magZ);
        magPosOff[0] = max(magPosOff[0], magX);
        magPosOff[1] = max(magPosOff[1], magY);
        magPosOff[2] = max(magPosOff[2], magZ);
        magNegOff[0] = min(magNegOff[0], magX);
        magNegOff[1] = min(magNegOff[1], magY);
        magNegOff[2] = min(magNegOff[2], magZ);
        Delay_ms(10);
    }

     // 计算软铁增益
    for (int i = 0; i < 3; ++i) {
        float range = magPosOff[i] - magNegOff[i];
        if (range == 0) {
            // 避免除以零的情况
            magGain[i] = 1.0f;
        } else {
            magGain[i] = DESIRED_RANGE / range;
        }
    }
}
