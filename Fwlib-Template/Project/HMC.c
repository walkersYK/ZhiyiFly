#include "stm32f10x.h"   
#include "Delay.h"
#include "MyI2C.h"

/**************************************
起始信号
**************************************/
void HMC5883_Start()
{
    MyI2C_W_SDA(1);                    //拉高数据线
    SCL = 1;                    //拉高时钟线
    Delay_us(5);                 //延时
    SDA = 0;                    //产生下降沿
    Delay5us();                 //延时
    SCL = 0;                    //拉低时钟线
}

/**************************************
停止信号
**************************************/
void HMC5883_Stop()
{
    SDA = 0;                    //拉低数据线
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    SDA = 1;                    //产生上升沿
    Delay5us();                 //延时
}

/**************************************
发送应答信号
入口参数:ack (0:ACK 1:NAK)
**************************************/
void HMC5883_SendACK(bit ack)
{
    SDA = ack;                  //写应答信号
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    SCL = 0;                    //拉低时钟线
    Delay5us();                 //延时
}


/**************************************
接收应答信号
**************************************/
bit HMC5883_RecvACK()
{
    SCL = 1;                    //拉高时钟线
    Delay5us();                 //延时
    CY = SDA;                   //读应答信号
    SCL = 0;                    //拉低时钟线
    Delay5us();                 //延时

    return CY;
}

/**************************************
向IIC总线发送一个字节数据
**************************************/
void HMC5883_SendByte(BYTE dat)
{
    BYTE i;

    for (i=0; i<8; i++)         //8位计数器
    {
        dat <<= 1;              //移出数据的最高位
        SDA = CY;               //送数据口
        SCL = 1;                //拉高时钟线
        Delay5us();             //延时
        SCL = 0;                //拉低时钟线
        Delay5us();             //延时
    }
    HMC5883_RecvACK();
}

/**************************************
从IIC总线接收一个字节数据
**************************************/
BYTE HMC5883_RecvByte()
{
    BYTE i;
    BYTE dat = 0;

    SDA = 1;                    //使能内部上拉,准备读取数据,
    for (i=0; i<8; i++)         //8位计数器
    {
        dat <<= 1;
        SCL = 1;                //拉高时钟线
        Delay5us();             //延时
        dat |= SDA;             //读数据               
        SCL = 0;                //拉低时钟线
        Delay5us();             //延时
    }
    return dat;
}

//******************************************************
//
//连续读出HMC5883内部角度数据，地址范围0x3~0x5
//
//******************************************************
void Multiple_read_HMC5883(void)
{   uchar i;
    HMC5883_Start();                          //起始信号
    HMC5883_SendByte(SlaveAddress);           //发送设备地址+写信号
    HMC5883_SendByte(0x03);                   //发送存储单元地址，从0x3开始	
    HMC5883_Start();                          //起始信号
    HMC5883_SendByte(SlaveAddress+1);         //发送设备地址+读信号
	 for (i=0; i<6; i++)                      //连续读取6个地址数据，存储中BUF
    {
        BUF[i] = HMC5883_RecvByte();          //BUF[0]存储数据
        if (i == 5)
        {
           HMC5883_SendACK(1);                //最后一个数据需要回NOACK
        }
        else
        {
          HMC5883_SendACK(0);                //回应ACK
       }
   }
    HMC5883_Stop();                          //停止信号
    Delay5ms();
}


//初始化HMC5883，根据需要请参考pdf进行修改****
void Init_HMC5883()
{
     Single_Write_HMC5883(0x02,0x00);  //
}

//*********************************************************
//主程序********
//*********************************************************
void main()
{ // bit sign_bit;
   unsigned int i;
   int x,y,z;
   double angle;

   delay(500);			
   InitLcd();
   Init_HMC5883();
  while(1)            //循环
  { 
    Multiple_Read_HMC5883();      //连续读出数据，存储在BUF中
//---------显示X轴
    x=BUF[0] << 8 | BUF[1]; //Combine MSB and LSB of X Data output register
    z=BUF[2] << 8 | BUF[3]; //Combine MSB and LSB of Z Data output register
    y=BUF[4] << 8 | BUF[5]; //Combine MSB and LSB of Y Data output register

    angle= atan2((double)y,(double)x) * (180 / 3.14159265) + 180; // angle in degrees
    angle*=10;
    conversion(angle);       //计算数据和显示
	DisplayOneChar(2,0,'A'); 
    DisplayOneChar(3,0,':'); 
    DisplayOneChar(4,0,qian); 
    DisplayOneChar(5,0,bai); 
    DisplayOneChar(6,0,shi); 
    DisplayOneChar(7,0,'.'); 
	DisplayOneChar(8,0,ge); 

    for (i=0;i<10000;i++);   //延时                   
  }
} 
