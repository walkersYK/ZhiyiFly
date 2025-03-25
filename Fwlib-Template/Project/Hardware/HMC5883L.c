#include "MyI2C.h"
#include "HMC5883L.h"
#include "stm32f10x.h"    
#include "stdio.h"   

uint8_t BUF[8];
int16_t magX, magY, magZ;
int8_t magOffset[3],magGain[3];
/**
  * ��    ����HMC5883L��ʼ��
  * ��    ������
  * �� �� ֵ����
  * ע�������
  */
void HMC5883L_Init(void)
{
	HMC5883L_WriteReg(HMC5883L_REG_CONFIG_A, 0X70); //// ����ƽ�����������������
	HMC5883L_WriteReg(HMC5883L_REG_CONFIG_B, 0X20); // ��������
	HMC5883L_WriteReg(HMC5883L_REG_MODE, 0X00);
}

void Multiple_read_HMC5883(void)
{   uint8_t i;
    MyI2C_Start();                          //��ʼ�ź�
    MyI2C_SendByte(HMC5883L_ADDRESS);           //�����豸��ַ+д�ź�
    MyI2C_SendByte(0x03);                   //���ʹ洢��Ԫ��ַ����0x3��ʼ	
    MyI2C_Start();                          //��ʼ�ź�
    MyI2C_SendByte(HMC5883L_ADDRESS+1);         //�����豸��ַ+���ź�
	 for (i=0; i<6; i++)                      //������ȡ6����ַ���ݣ��洢��BUF
    {
        BUF[i] = MyI2C_ReceiveByte();          //BUF[0]�洢����
        if (i == 5)
        {
           MyI2C_SendAck(1);                //���һ��������Ҫ��NOACK
        }
        else
        {
          MyI2C_SendAck(0);                //��ӦACK
       }
   }
    MyI2C_Stop();                          //ֹͣ�ź�
    Delay_ms(5);
}

//**************************************
//��HMC5883������ȡ6�����ݷ���BUF��
//**************************************
void updateHMC5883(int16_t *GaX, int16_t *GaY, int16_t *GaZ)
{
//			Multiple_read_HMC5883();      //�����������ݣ��洢��BUF��
//			//---------��ʾX��
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

//ָ���ƶ�ƽ��ֵ����͹��˴�������
#define alpha 0.2
float EMA(float rawValue, float filteredValue)
{
    return alpha * rawValue + (1 - alpha) * filteredValue;
}
/**************************
  * function       ��HMC5883Lд�Ĵ���
  * ��    ����RegAddress �Ĵ�����ַ����Χ���ο�MPU6050�ֲ�ļĴ�������
  * ��    ����Data Ҫд��Ĵ��������ݣ���Χ��0x00~0xFF
  * �� �� ֵ����
  ************************/
void HMC5883L_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	MyI2C_Start();						//I2C��ʼ
	MyI2C_SendByte(HMC5883L_ADDRESS);	//���ʹӻ���ַ����дλΪ0����ʾ����д��
	MyI2C_ReceiveAck();					//����Ӧ��
	MyI2C_SendByte(RegAddress);			//���ͼĴ�����ַ
	MyI2C_ReceiveAck();					//����Ӧ��
	MyI2C_SendByte(Data);				//����Ҫд��Ĵ���������
	MyI2C_ReceiveAck();					//����Ӧ��
	MyI2C_Stop();						//I2C��ֹ
}
/**
  * ��    ����HMC5883L���Ĵ���
  * ��    ����RegAddress �Ĵ�����ַ����Χ���ο�MPU6050�ֲ�ļĴ�������
  * �� �� ֵ����ȡ�Ĵ��������ݣ���Χ��0x00~0xFF
  */
uint8_t HMC5883L_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	
	MyI2C_Start();						//I2C��ʼ
	MyI2C_SendByte(HMC5883L_ADDRESS);	//���ʹӻ���ַ����дλΪ0����ʾ����д��
	MyI2C_ReceiveAck();					//����Ӧ��
	MyI2C_SendByte(RegAddress);			//���ͼĴ�����ַ
	MyI2C_ReceiveAck();					//����Ӧ��
	
	MyI2C_Start();						//I2C�ظ���ʼ
	MyI2C_SendByte(HMC5883L_ADDRESS | 0x01);	//���ʹӻ���ַ����дλΪ1����ʾ������ȡ
	MyI2C_ReceiveAck();					//����Ӧ��
	Data = MyI2C_ReceiveByte();			//����ָ���Ĵ���������
	MyI2C_SendAck(1);					//����Ӧ�𣬸��ӻ���Ӧ����ֹ�ӻ����������
	MyI2C_Stop();						//I2C��ֹ
	
	return Data;
}
/************
У׼
*************/
int max(int a, int b) {  
    return (a > b) ? a : b;  
}
int min(int a, int b) {  
    return (a < b) ? a : b;  
}
#define DESIRED_RANGE 100.0f // ��50��T, total range is 100��T (from -50��T to +50��T)
void calibrateMag() {
    // ... [��ʼ��������] ...

    int16_t magPosOff[3], magNegOff[3];
    float avgMag[3] = {0, 0, 0};
    const int samples = 100; // �ռ�����������

    for (int i = 0; i < samples; ++i) {
        updateHMC5883(&magX, &magY, &magZ);
        avgMag[0] += magX;
        avgMag[1] += magY;
        avgMag[2] += magZ;
        Delay_ms(10); // ��������ʱ���ȶ�
    }

    // ����ƽ��ֵ
    for (int i = 0; i < 3; ++i) {
        avgMag[i] /= samples;
    }

    // Ӳ��ƫ�Ƽ�Ϊƽ��ֵ
    magOffset[0] = avgMag[0];
    magOffset[1] = avgMag[1];
    magOffset[2] = avgMag[2];

    // �ҵ������Сֵ��������У׼
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

     // ������������
    for (int i = 0; i < 3; ++i) {
        float range = magPosOff[i] - magNegOff[i];
        if (range == 0) {
            // �������������
            magGain[i] = 1.0f;
        } else {
            magGain[i] = DESIRED_RANGE / range;
        }
    }
}
