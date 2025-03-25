/*

*/

#ifndef _USARTX_H
#define _USARTX_H
#include "stm32f10x.h"
//串口1
#define USARTx 								USART1 
#define USART_CLK 						RCC_APB2Periph_USART1   //USART1时钟
#define USART_APBxClkCmd		  RCC_APB2PeriphClockCmd  //APB2时钟使能
#define USART_BAUDRATE  		  115200                  //波特率
//USART GPIO
#define USART_GPIO_CLK			  RCC_APB2Periph_GPIOA    //GPIOA时钟
#define USART_GPIO_APBxClkCmd RCC_APB2PeriphClockCmd  //APB2时钟使能
//USART引脚
#define USART_TX_GPIO_PORT    GPIOA
#define USART_TX_GPIO_Pin     GPIO_Pin_9
#define USART_RX_GPIO_PORT    GPIOA
#define USART_RX_GPIO_Pin     GPIO_Pin_10

#define USART_IRQ             USART1_IRQn
void USARTx_SendString(char* str);
void USART_Config(void);
void USARTx_SendByte(uint8_t ch);
#endif
