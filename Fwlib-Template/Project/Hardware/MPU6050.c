#include "stm32f10x.h"                  // Device header
#include "MyI2C.h"
#include "Delay.h"
#include "MPU6050_Reg.h"
#include "MPU6050.h"
#include "Filter.h"
#include "fly_control.h"

#define MPU6050_ADDRESS		0xD0		//MPU6050的I2C从机地址

// 创建用于存储数据的结构体变量
Axis3f gyroData;
Axis3f accData;

_st_Mpu MPU6050;   //MPU6050原始数据
_st_AngE Angle;
short MpuOffset[6];		//MPU6050补偿数值

// 偏移量数组，假设已经通过校准获得

static volatile int16_t *pMpu = (int16_t *)&MPU6050;


/*************************************
*function    : mpu设置直通模式
*flash 			： EXT_SENS_DATA
*************************************/
void MPU6050_PassMode(void)
{
	MPU6050_WriteReg(MPU6050_USER_CTRL, 0x00);	//禁用主机
	MPU6050_WriteReg(MPU6050_INT_PIN_CFG, 0x02); //启动旁路
}
/*************************************
*function    : mpu设置主模式
*flash 			： EXT_SENS_DATA
*************************************/
void MPU6050_MasMode(void)
{
	MyI2C_Init();		
	
	MPU6050_WriteReg(MPU6050_INT_PIN_CFG, 0x00);	
	MPU6050_WriteReg(MPU6050_USER_CTRL, 0x22);	
	MPU6050_WriteReg(MPU6050_I2C_MST_CTRL, 0x0D);	
}



/**************************
  * function       ：MPU6050写寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 参    数：Data 要写入寄存器的数据，范围：0x00~0xFF
  * 返 回 值：无
  ************************/
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(MPU6050_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(Data);				//发送要写入寄存器的数据
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_Stop();						//I2C终止
}
/**
  * 函    数：MPU6050读寄存器
  * 参    数：RegAddress 寄存器地址，范围：参考MPU6050手册的寄存器描述
  * 返 回 值：读取寄存器的数据，范围：0x00~0xFF
  */
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();						//I2C起始
	MyI2C_SendByte(MPU6050_ADDRESS);	//发送从机地址，读写位为0，表示即将写入
	MyI2C_ReceiveAck();					//接收应答
	MyI2C_SendByte(RegAddress);			//发送寄存器地址
	MyI2C_ReceiveAck();					//接收应答
	
	MyI2C_Start();						//I2C重复起始
	MyI2C_SendByte(MPU6050_ADDRESS | 0x01);	//发送从机地址，读写位为1，表示即将读取
	MyI2C_ReceiveAck();					//接收应答
	Data = MyI2C_ReceiveByte();			//接收指定寄存器的数据
	MyI2C_SendAck(1);					//发送应答，给从机非应答，终止从机的数据输出
	MyI2C_Stop();						//I2C终止
	
	return Data;
}

/*********************
* @functioon     : I2C连续写
*********************/
u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i; 
  MyI2C_Start(); 
	MyI2C_SendByte(MPU6050_ADDRESS);//发送器件地址+写命令	
	if(MyI2C_ReceiveAck())	//等待应答
	{
		MyI2C_Stop();		 
		return 1;		
	}
    MyI2C_SendByte(reg);	//写寄存器地址
    MyI2C_ReceiveAck();		//等待应答
	for(i=0;i<len;i++)
	{
		MyI2C_SendByte(buf[i]);	//发送数据
		if(MyI2C_ReceiveAck())		//等待ACK
		{
			MyI2C_Stop();	 
			return 1;		 
		}		
	}    
    MyI2C_Stop();	 
	return 0;	
} 
/*************************************
 @function        :连续读
**************************************/
void MPU6050_ReadRegs(uint8_t StartReg, uint8_t *pData, uint16_t Length)
{
    MyI2C_Start();                      // I2C起始
    MyI2C_SendByte(MPU6050_ADDRESS);    // 发送从机地址，读写位为0，表示即将写入
    MyI2C_ReceiveAck();                 // 接收应答
    MyI2C_SendByte(StartReg);           // 发送起始寄存器地址
    MyI2C_ReceiveAck();                 // 接收应答

    MyI2C_Start();                      // I2C重复起始
    MyI2C_SendByte(MPU6050_ADDRESS | 0x01); // 发送从机地址，读写位为1，表示即将读取
    MyI2C_ReceiveAck();                 // 接收应答
    
    for (uint16_t i = 0; i < Length; i++)
    {
        pData[i] = MyI2C_ReceiveByte(); // 接收数据
        
        if (i == Length - 1)            // 如果是最后一个字节
        {
            MyI2C_SendAck(1);           // 发送非应答（NACK），终止从机的数据输出
        }
        else
        {
            MyI2C_SendAck(0);           // 发送应答（ACK）
        }
    }

    MyI2C_Stop();                       // I2C终止
}

/**
  * 函    数：MPU6050初始化
  * 参    数：无
  * 返 回 值：无
  */
void MPU6050_Init(void)
{
	MyI2C_Init();									//先初始化底层的I2C
	
	/*MPU6050寄存器初始化，需要对照MPU6050手册的寄存器描述配置，此处仅配置了部分重要的寄存器*/
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);		//电源管理寄存器1，取消睡眠模式，选择时钟源为X轴陀螺仪
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);		//电源管理寄存器2，保持默认值0，所有轴均不待机
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x0F);		//采样率分频寄存器，配置采样率
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06);			//配置寄存器，配置DLPF
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);	//陀螺仪配置寄存器，选择满量程为±2000°/s
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x00);	//加速度计配置寄存器，选择满量程为±16g
	MPU6050_PassMode();
}

/**
  * 函    数：MPU6050获取ID号
  * 参    数：无
  * 返 回 值：MPU6050的ID号
  */
uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);		//返回WHO_AM_I寄存器的值
}

/**
  * 函    数：MPU6050获取数据
  * 参    数：AccX AccY AccZ 加速度计X、Y、Z轴的数据，使用输出参数的形式返回，范围：-32768~32767
  * 参    数：GyroX GyroY GyroZ 陀螺仪X、Y、Z轴的数据，使用输出参数的形式返回，范围：-32768~32767
  * 返 回 值：无
  */
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	uint8_t DataH, DataL;								//定义数据高8位和低8位的变量
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);		//读取加速度计X轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);		//读取加速度计X轴的低8位数据
	*AccX = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);		//读取加速度计Y轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);		//读取加速度计Y轴的低8位数据
	*AccY = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);		//读取加速度计Z轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);		//读取加速度计Z轴的低8位数据
	*AccZ = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);		//读取陀螺仪X轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);		//读取陀螺仪X轴的低8位数据
	*GyroX = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);		//读取陀螺仪Y轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);		//读取陀螺仪Y轴的低8位数据
	*GyroY = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);		//读取陀螺仪Z轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);		//读取陀螺仪Z轴的低8位数据
	*GyroZ = (DataH << 8) | DataL;						//数据拼接，通过输出参数返回
}


/**
  * 函    数：读取MPU6050数据并加滤波
  * 参    数：无
  * 返 回 值：无
  */
void MpuGetData(void)
{
    uint8_t i;
    uint8_t buffer[12];
		int16_t rawMpu[6]; 
	/*1.读取*/
    // 读取加速度计数据（寄存器地址0x3B）
    MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H, buffer, 6); // 读取前6个字节到buffer
    // 读取陀螺仪数据（寄存器地址0x43） 
    MPU6050_ReadRegs(MPU6050_GYRO_XOUT_H, &buffer[6], 6); // 读取后6个字节到buffer
	/* 2. 校准*/
    for(i = 0; i < 6; i++)
    {
        // 将数据转换为16位整数，并减去偏移量
       rawMpu[i] = (((int16_t)buffer[i<<1] << 8) | buffer[(i<<1)+1])-MpuOffset[i];	
        
        if(i < 3)  // 加速度计数据（角加速度）
        {
            // 应用卡尔曼滤波
            kalmanfiter(&EKF[i], (float)rawMpu[i]);
            pMpu[i] = (int16_t)EKF[i].Out;	
        }
        else  // 角速度数据（角速度）
        {
            uint8_t k = i - 3;
            // 修改互补滤波系数（原0.15→0.3）
						const float factor = 0.3f;
						// 静态定义互补滤波器缓冲区
						static float tBuff[3] = {0};  
            // 应用一阶互补滤波
            pMpu[i] = tBuff[k] * (1 - factor) + rawMpu[i] * factor;
						tBuff[k] = pMpu[i]; // 添加状态保存
        }
    }
		/*3.滤波*/
		/*3.1 加速度进行滤波（噪声更大，容易受抖动的影响）：一维卡尔曼滤波*/
		/* 4. 打印滤波前后的数据 */
//		printf("============= 滤波前后数据  ============= \r\n");
//    printf("Raw AccX: %d, Filtered AccX: %d\n", rawMpu[0], pMpu[0]);
//    printf("Raw AccY: %d, Filtered AccY: %d\n", rawMpu[1], pMpu[1]);
//    printf("Raw AccZ: %d, Filtered AccZ: %d\n", rawMpu[2], pMpu[2]);
//    printf("Raw GyroX: %d, Filtered GyroX: %d\n", rawMpu[3], pMpu[3]);
//    printf("Raw GyroY: %d, Filtered GyroY: %d\n", rawMpu[4], pMpu[4]);
//    printf("Raw GyroZ: %d, Filtered GyroZ: %d\n", rawMpu[5], pMpu[5]);
}

void GetOffSets(void)
{
	const int8_t MIN_GYRO_BIAS = -5; 
	const int8_t MAX_GYRO_BIAS = 5;
	
	uint8_t gryo_temp = 30; //次数
	
	short last_gyro[3] = {0}; //上一次的角速度
	short gryo_bias[3] = {0}; //本次计算的差值
	
	int32_t bias_sum_buff[6] = {0}; //保存六轴差值的和
	
	/* 判断是否处于静止状态*/
	while( gryo_temp -- )
	{
		do{
		/*判断逻辑*/
			Delay_ms(2); //至少2ms 采样率500hz ,一个周期2ms更新一次数据
			MpuGetData();
			gryo_bias[0] = MPU6050.gyroX - last_gyro[0];
			gryo_bias[1] = MPU6050.gyroY - last_gyro[1];
			gryo_bias[2] = MPU6050.gyroZ - last_gyro[2];
			
			/* 将本次的值，保存到数组*/
			last_gyro[0] = MPU6050.gyroX;
			last_gyro[1] = MPU6050.gyroY;
			last_gyro[2] = MPU6050.gyroZ;
			
		}while(
			gryo_bias[0] < MIN_GYRO_BIAS || gryo_bias[0] >MAX_GYRO_BIAS
			|| gryo_bias[1] < MIN_GYRO_BIAS || gryo_bias[1] >MAX_GYRO_BIAS
			|| gryo_bias[2] < MIN_GYRO_BIAS || gryo_bias[2] >MAX_GYRO_BIAS
		); //不通过的条件
	}
	/*读取多次数据，每次求出一个差值*/
	for (uint16_t i = 0; i<356; i++)
	{
		//前100次不要
		Delay_ms(2); //至少2ms 采样率500hz ,一个周期2ms更新一次数据
		MpuGetData();
		if(i >= 100)
		{
			bias_sum_buff[0] += (MPU6050.accX - 0);
			bias_sum_buff[1] += (MPU6050.accY - 0);
			bias_sum_buff[2] += (MPU6050.accZ- 16384);
			bias_sum_buff[3] += (MPU6050.gyroX - 0);
			bias_sum_buff[4] += (MPU6050.gyroY - 0);
			bias_sum_buff[5] += (MPU6050.gyroZ - 0);
		}
	}
	/* 对多次差值取平均值*/
	for (uint8_t i = 0; i<6; i++)
	{
		MpuOffset[i] = bias_sum_buff[i] >> 8;
		
	}
	/*将平均值放到MpuoffStes数组*/
	
}