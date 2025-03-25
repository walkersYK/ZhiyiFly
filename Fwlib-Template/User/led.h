#ifndef _LED_H
#define _LED_H
#include"stm32f10x.h"
#define LED_PORT GPIOC
#define LED_PIN  GPIO_Pin_13
#define LED_GPIO_CLOCK RCC_APB2Periph_GPIOC
void LED_Init(void);
#endif
