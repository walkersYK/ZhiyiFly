#ifndef __FILTER_H
#define __FILTER_H

#include "stm32f10x.h" 
#include "config.h"
//卡尔曼滤波参数结构体
struct KalmanFilter{
	float LastP;		//上一次协方差
	float NewP;		//最新的协方差
	float Out;			//卡尔曼输出
	float Kg;				//卡尔曼增益
	float Q;				//过程噪声的协方差
	float R;				//观测噪声的协方差
};

//一维卡尔曼滤波
extern void kalmanfiter(struct KalmanFilter *EKF,float input);	
extern struct KalmanFilter EKF[3];

void kalmanfiter(struct KalmanFilter *EKF,float input);

#endif
