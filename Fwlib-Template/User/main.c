#include"led.h"
#include"systick.h"
#include"usartx.h"
#include"at_cmd.h"
#include "stdio.h"
#include <string.h>

#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "HMC5883L.h"
#include "config.h"
#include "imu.h"
#include "fly_control.h"
#include "Delay.h"
#include "pwm.h"

extern int16_t Gax, Gay, Gaz;

// 遥控数据接收相关
volatile uint8_t uart_rx_buffer[8] = {0}; // 接收缓冲区（8字节）
volatile uint8_t uart_rx_index = 0;       // 当前接收位置
volatile uint8_t packet_ready = 1;        // 数据包就绪标志
sReceiveRemote receiveRemote;            // 存储解析后的遥控数据


int i=0;//定时计数
#define MAX_LEN 20

volatile uint8_t ucTemp[MAX_LEN];  // 接收缓冲区
volatile uint8_t _i = 0;           // 当前接收位置
volatile uint8_t rxBuffer[MAX_LEN + 1]; // 主循环处理缓冲区（+1用于字符串终止符）
volatile uint8_t rxFlag = 0;       // 数据接收完成标志

uint8_t ID;								//定义用于存放ID号的变量
int16_t AX, AY, AZ, GX, GY, GZ;			//定义用于存放各个数据的变量

int main(void) {
    LED_Init(); // 初始化Led配置
    SystickConfig(1000); // systick配置
    USART_Config();
    Delay_ms(1000);
    USARTx_SendString("AT+CIPMUX=1\r\n");
    Delay_ms(1000);
    USARTx_SendString("AT+CIPSERVER=1,8088\r\n");
    Delay_ms(500);

    /* 模块初始化 */
    OLED_Init(); // OLED初始化
    MPU6050_Init(); // MPU6050初始化
		printf("<<<<<<<<<mpu6050开始执行校准>>>>>>>");
		GetOffSets();
		printf(">>>>>>>>>mpu6050校准完成<<<<<<<<<<");
		MPU6050_PassMode(); //旁模式
		Delay_ms(65);
		HMC5883L_Init();
	  PWM_Init();
		
    /* 初始化PID参数 */
    App_Flight_PID_Init();

    while (1) {
//			updateHMC5883(&Gax, &Gay, &Gaz);
//			MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
//			printf(">>>>>>>>>>>>>原始数据<<<<<<<<<<<<<<<<<<\n");
//			printf("Raw AccX: %d\n", AX);
//			printf("Raw AccY: %d\n", AY);
//			printf("Raw AccZ: %d\n", AZ);
//			printf("Raw GyroX: %d\n", GX);
//			printf("Raw GyroY: %d\n", GY);
//			printf("Raw GyroZ: %d\n\n", GZ);
			MpuGetData();
		  imuUpdate(&MPU6050,&Angle,0.006f);
//			printf("===================四元素解算欧拉角==============\r\n");
//			printf("俯仰角pitch = %.1f\r\n", Angle.pitch);
//			printf("翻滚角roll = %.1f\r\n", Angle.roll);
//			printf("偏航角yaw = %.1f\r\n", Angle.yaw);
//			updateYaw(&Angle);		
			// OLED显示姿态角
			OLED_ShowSignedNum(2, 1, (int)(Angle.pitch), 3); // 显示Pitch
			OLED_ShowSignedNum(3, 1, (int)Angle.roll, 3);  // 显示Roll
			OLED_ShowSignedNum(4, 1, (int)Angle.yaw, 3);   // 显示Yaw
					
			// 添加适当的延时，以避免屏幕刷新过快
			Delay_ms(6); // ATTITUDE_UPDATE_DT是以秒为单位的，所以要转换成毫秒
			
			/*遥控测试*/
			 // 1. 读取遥控器数据
			 if (rxFlag) { // 检测数据接收完成标志
            rxFlag = 0; // 清除标志
				 
            printf("Raw Bytes: ");
						for (int i=0; i<sizeof(sReceiveRemote); i++) {
								printf("%02X ", rxBuffer[i]);
						}
						printf("\n");

						sReceiveRemote remoteData;
						memcpy(&remoteData, rxBuffer, sizeof(remoteData));
						USARTx_SendString("AT+CIPSEND=0,20\r\n");
						Delay_ms(1000);
						USARTx_SendString(remoteData.THR);
						printf("THR=%d YAW=%d ROL=%d PIT=%d\n", 
									 remoteData.THR, remoteData.YAW, 
									 remoteData.ROL, remoteData.PIT);
        }
	
			ReadRemoteControl(&receiveRemote);
			
				 // 2. 调试输出（可选）
//			#ifdef DEBUG
			// 正确方式（需封装协议）：
			char debugMsg[32];
			sprintf(debugMsg, "THR:%d YAW:%d\n", receiveRemote.THR, receiveRemote.YAW); 
			Delay_ms(2);
//			#endif
	// 4. 将摇杆输入转换为期望姿态角
			App_Flight_Move();
			// 5. 计算姿态PID
			float dt = 0.006f; // 获取时间间隔（秒）
			App_Flight_PosturePID(dt);

			// 6. 计算电机输出并应用
			App_Flight_Motor();
    }
}

//定时中断
void SysTick_Handler(void)
{
	i++;
	if(i==1000)
		{//定时1000毫秒
	  LED_PORT->ODR^=LED_PIN;
		//USARTx_SendString("#DEBUG#HART_BOOM");//定时发送心跳数据，避免断开连接
		USARTx_SendString("AT+CIPSEND=0,20\r\n");
		}else if(i==2000){
			USARTx_SendString("1234567890");
			i=0;
		}
}

// 中断处理函数
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        uint8_t received = USART_ReceiveData(USART1);
        
        if (received == '\n' || _i >= MAX_LEN - 1) {
            ucTemp[_i] = '\0'; 
            for (int p = 0; p <= _i; p++) {
                rxBuffer[p] = ucTemp[p];
            }
            
            rxFlag = 1;
            _i = 0;
        } else {
            ucTemp[_i] = received;
            _i++;
        }
    }
}
