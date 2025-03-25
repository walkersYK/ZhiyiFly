#include"led.h"
void LED_Init(void){	
  RCC_APB2PeriphClockCmd(LED_GPIO_CLOCK,ENABLE);//¿ªÆôGPIOBµÄÊ±ÖÓ
	GPIO_InitTypeDef GPIO_Init_Struct;
	GPIO_Init_Struct.GPIO_Pin=LED_PIN;
	GPIO_Init_Struct.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init_Struct.GPIO_Speed=GPIO_Speed_50MHz;
  GPIO_Init(LED_PORT,&GPIO_Init_Struct);	
}
