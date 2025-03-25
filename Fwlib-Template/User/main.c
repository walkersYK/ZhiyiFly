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

// ң�����ݽ������
volatile uint8_t uart_rx_buffer[8] = {0}; // ���ջ�������8�ֽڣ�
volatile uint8_t uart_rx_index = 0;       // ��ǰ����λ��
volatile uint8_t packet_ready = 1;        // ���ݰ�������־
sReceiveRemote receiveRemote;            // �洢�������ң������


int i=0;//��ʱ����
#define MAX_LEN 20

volatile uint8_t ucTemp[MAX_LEN];  // ���ջ�����
volatile uint8_t _i = 0;           // ��ǰ����λ��
volatile uint8_t rxBuffer[MAX_LEN + 1]; // ��ѭ������������+1�����ַ�����ֹ����
volatile uint8_t rxFlag = 0;       // ���ݽ�����ɱ�־

uint8_t ID;								//�������ڴ��ID�ŵı���
int16_t AX, AY, AZ, GX, GY, GZ;			//�������ڴ�Ÿ������ݵı���

int main(void) {
    LED_Init(); // ��ʼ��Led����
    SystickConfig(1000); // systick����
    USART_Config();
    Delay_ms(1000);
    USARTx_SendString("AT+CIPMUX=1\r\n");
    Delay_ms(1000);
    USARTx_SendString("AT+CIPSERVER=1,8088\r\n");
    Delay_ms(500);

    /* ģ���ʼ�� */
    OLED_Init(); // OLED��ʼ��
    MPU6050_Init(); // MPU6050��ʼ��
		printf("<<<<<<<<<mpu6050��ʼִ��У׼>>>>>>>");
		GetOffSets();
		printf(">>>>>>>>>mpu6050У׼���<<<<<<<<<<");
		MPU6050_PassMode(); //��ģʽ
		Delay_ms(65);
		HMC5883L_Init();
	  PWM_Init();
		
    /* ��ʼ��PID���� */
    App_Flight_PID_Init();

    while (1) {
//			updateHMC5883(&Gax, &Gay, &Gaz);
//			MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
//			printf(">>>>>>>>>>>>>ԭʼ����<<<<<<<<<<<<<<<<<<\n");
//			printf("Raw AccX: %d\n", AX);
//			printf("Raw AccY: %d\n", AY);
//			printf("Raw AccZ: %d\n", AZ);
//			printf("Raw GyroX: %d\n", GX);
//			printf("Raw GyroY: %d\n", GY);
//			printf("Raw GyroZ: %d\n\n", GZ);
			MpuGetData();
		  imuUpdate(&MPU6050,&Angle,0.006f);
//			printf("===================��Ԫ�ؽ���ŷ����==============\r\n");
//			printf("������pitch = %.1f\r\n", Angle.pitch);
//			printf("������roll = %.1f\r\n", Angle.roll);
//			printf("ƫ����yaw = %.1f\r\n", Angle.yaw);
//			updateYaw(&Angle);		
			// OLED��ʾ��̬��
			OLED_ShowSignedNum(2, 1, (int)(Angle.pitch), 3); // ��ʾPitch
			OLED_ShowSignedNum(3, 1, (int)Angle.roll, 3);  // ��ʾRoll
			OLED_ShowSignedNum(4, 1, (int)Angle.yaw, 3);   // ��ʾYaw
					
			// ����ʵ�����ʱ���Ա�����Ļˢ�¹���
			Delay_ms(6); // ATTITUDE_UPDATE_DT������Ϊ��λ�ģ�����Ҫת���ɺ���
			
			/*ң�ز���*/
			 // 1. ��ȡң��������
			 if (rxFlag) { // ������ݽ�����ɱ�־
            rxFlag = 0; // �����־
				 
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
			
				 // 2. �����������ѡ��
//			#ifdef DEBUG
			// ��ȷ��ʽ�����װЭ�飩��
			char debugMsg[32];
			sprintf(debugMsg, "THR:%d YAW:%d\n", receiveRemote.THR, receiveRemote.YAW); 
			Delay_ms(2);
//			#endif
	// 4. ��ҡ������ת��Ϊ������̬��
			App_Flight_Move();
			// 5. ������̬PID
			float dt = 0.006f; // ��ȡʱ�������룩
			App_Flight_PosturePID(dt);

			// 6. �����������Ӧ��
			App_Flight_Motor();
    }
}

//��ʱ�ж�
void SysTick_Handler(void)
{
	i++;
	if(i==1000)
		{//��ʱ1000����
	  LED_PORT->ODR^=LED_PIN;
		//USARTx_SendString("#DEBUG#HART_BOOM");//��ʱ�����������ݣ�����Ͽ�����
		USARTx_SendString("AT+CIPSEND=0,20\r\n");
		}else if(i==2000){
			USARTx_SendString("1234567890");
			i=0;
		}
}

// �жϴ�����
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
