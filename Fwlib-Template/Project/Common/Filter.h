#ifndef __FILTER_H
#define __FILTER_H

#include "stm32f10x.h" 
#include "config.h"
//�������˲������ṹ��
struct KalmanFilter{
	float LastP;		//��һ��Э����
	float NewP;		//���µ�Э����
	float Out;			//���������
	float Kg;				//����������
	float Q;				//����������Э����
	float R;				//�۲�������Э����
};

//һά�������˲�
extern void kalmanfiter(struct KalmanFilter *EKF,float input);	
extern struct KalmanFilter EKF[3];

void kalmanfiter(struct KalmanFilter *EKF,float input);

#endif
