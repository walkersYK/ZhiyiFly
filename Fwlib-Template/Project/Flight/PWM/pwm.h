#ifndef _pwm_H
#define _pwm_H
void PWM_RCC(void);                  //时钟配置   
void PWM_GPIO(void);                 //管脚配置
void PWM_TIM3_Configuration(void);   //占空比时钟控制
void TIM3_NVIC_Configuration(void);  //优先级
void PWM_Init(void);                 //初始化 
#endif       

