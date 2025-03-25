#include "pwm.h"
#include "stm32f10x.h"
//**********************配置系统时钟*********************************
void PWM_RCC(void)
{ 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);    //PWM引脚时钟配置
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);    //PWM引脚时钟配置
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);     //打开time3的中断时钟
}
//**********************配置GPIO管脚*********************************
void PWM_GPIO(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
	//PWM管脚PWM1配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//PWM管脚PWM2配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	//PWM管脚PWM3配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	//PWM管脚PWM4配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
//**********************时钟中断配置函数*********************************
void PWM_TIM3_Configuration(void)
{ 
  TIM_OCInitTypeDef  TIM_OCInitStructure;                      //定义结构体变量
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3,DISABLE);         //关闭映射功能   
  TIM_TimeBaseStructure.TIM_Period=7200;                       //计数7200,PWM频率10KHz  
  TIM_TimeBaseStructure.TIM_Prescaler=0;                       //不分频
  TIM_TimeBaseStructure.TIM_ClockDivision=0;                   //不滤波
  TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;    //向上计数模式
  TIM_TimeBaseInit(TIM3,&TIM_TimeBaseStructure);               //初始化
	TIM_Cmd(TIM3,ENABLE);                                        //打开定时器外设
//***配置PWM1**********	
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;            //PWM模式1
  TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;  //打开PWM使能
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     //输出极性
	TIM_OC1Init(TIM3, & TIM_OCInitStructure); 	                 //初始化  使用通道1
	TIM_OC1PreloadConfig(TIM3,TIM_OCPreload_Enable);             //打开PWM中断使能，否则只能执行一次
	
	//***配置PWM2**********	
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;            //PWM模式1
  TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;  //打开PWM使能
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     //输出极性
	TIM_OC2Init(TIM3, & TIM_OCInitStructure); 	                 //初始化  使用通道2
	TIM_OC2PreloadConfig(TIM3,TIM_OCPreload_Enable);             //打开PWM中断使能，否则只能执行一次
	
	//***配置PWM3**********	
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;            //PWM模式1
  TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;  //打开PWM使能
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     //输出极性
	TIM_OC3Init(TIM3, & TIM_OCInitStructure); 	                 //初始化  使用通道3
	TIM_OC3PreloadConfig(TIM3,TIM_OCPreload_Enable);             //打开PWM中断使能，否则只能执行一次
	
	//***配置PWM4**********	
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;            //PWM模式1
  TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;  //打开PWM使能
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;     //输出极性
	TIM_OC4Init(TIM3, & TIM_OCInitStructure); 	                 //初始化  使用通道4
	TIM_OC4PreloadConfig(TIM3,TIM_OCPreload_Enable);             //打开PWM中断使能，否则只能执行一次
}
//**************************配置优先级***********************************
void TIM3_NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;                        //为结构体定义结构体变量
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);             //对优先级进行分组
  NVIC_InitStructure.NVIC_IRQChannel =TIM3_IRQn;              
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;   //抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;          //响应优先级为0         
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             //使能
  NVIC_Init(&NVIC_InitStructure);                             //初始化
}
//**********************红外发射初始化函数*********************************
void PWM_Init(void)
{
  PWM_RCC();                  //PWM时钟配置   
  PWM_GPIO();                 //PWM管脚配置
  PWM_TIM3_Configuration();   //占空比时钟控制 
	TIM3_NVIC_Configuration();  //优先级配置
	TIM_SetCompare1(TIM3,0);    //防止上电就乱动
	TIM_SetCompare2(TIM3,0);    //防止上电就乱动
	TIM_SetCompare3(TIM3,0);    //防止上电就乱动
	TIM_SetCompare4(TIM3,0);    //防止上电就乱动
}



















