#include "config.h"
#include "imu.h"
#include "fly_control.h"
#include "PID_CTL.h"

//********************************
// pid ���Ƶ�����***************
//********************************
extern sReceiveRemote receiveRemote; // ������յ���ң������������
uint8_t isUnlock = 0; // �Ƿ�����ı�־λ��0-δ������1-����

/* ������̬PID���⻷�������ǣ��ڻ���Y����ٶ� */
sPID pitch_pid;
sPID gyroY_pid;
/* �����̬PID���⻷������ǣ��ڻ���X����ٶ� */
sPID roll_pid;
sPID gyroX_pid;
/* ƫ����̬PID���⻷��ƫ���ǣ��ڻ���Z����ٶ� */
sPID yaw_pid;
sPID gyroZ_pid;


/* ����PID:�⻷-���߶ȣ��ڻ�-��z���ٶ� */
sPID height_pid; // �߶�pid
sPID zSpeed_pid; // z���ٶ�pid

/* pid�ṹ���ַ�����飬����ͳһ���� */
sPID *pids[8] = {&pitch_pid, &gyroY_pid, &roll_pid, &gyroX_pid, &yaw_pid, &gyroZ_pid, &height_pid, &zSpeed_pid};

extern float Gyro_G ;

/* ��Ӧ4�������ֵ */
int16_t motor1, motor2, motor3, motor4;
/**
 * @description: ��̬����PID����
 * @param {float} dt    ���ڣ���λ��
 * @return {*}
 */
void App_Flight_PosturePID(float dt)
{
    static uint8_t status = UNLOCK_0;
    switch (status)
    {
    case UNLOCK_0:
        /* ����״̬ */
        if (isUnlock == 1)
        {
            status = UNLOCK_PROCESS;
            /* ����PID���������½�����Ӱ���µ�PID���� */
            Com_PID_Reset(pids, 6);
        }
        break;
    case UNLOCK_PROCESS:
        /* ����״̬ */

        /* ŷ���ǲ���ֵ */
        pitch_pid.measure = Angle.pitch;
        roll_pid.measure = Angle.roll;
        yaw_pid.measure = Angle.yaw;

        /* ���ٶȲ���ֵ */
        gyroY_pid.measure = MPU6050.gyroY * Gyro_G;
        gyroX_pid.measure = MPU6050.gyroX * Gyro_G;
        gyroZ_pid.measure = MPU6050.gyroZ * Gyro_G;

        /* ������̬����PID���� */
        Com_PID_Cascade(&pitch_pid, &gyroY_pid, dt);
        /* �����̬����PID���� */
        Com_PID_Cascade(&roll_pid, &gyroX_pid, dt);
        /* ƫ����̬����PID���� */
        Com_PID_Cascade(&yaw_pid, &gyroZ_pid, dt);

        /* ����ָ��������˳�PID���� */
        if (isUnlock == 0)
        {
            status = UNLOCK_0;
        }

        break;
    default:
        break;
    }
}

/**
 * @description: ���Ƶ������PID���������ֵ�����ڵ��
 * @return {*}
 */
void App_Flight_Motor(void)
{
	static uint8_t status = UNLOCK_0;
    int16_t thr_tmp = 0;
    switch (status)
    {
    case UNLOCK_0:
        /* ����״̬ */
        if (isUnlock == 1)
        {
            status = UNLOCK_PROCESS;
            /* �����λ */
            motor1 = motor2 = motor3 = motor4 = 0;
        }
        break;
    case UNLOCK_PROCESS:
			/*��ʽ���ƽ׶�*/
		
		
			/*�ȴ�������ֵ*/
			/* 1.1����������ֵ��ȡֵ��ƥ�� ccr��ȡֵ��Χ0-1000 */
        thr_tmp = receiveRemote.THR - 1000;
        /* >>>>>>>>>>>>>>>>>>����PID�������������ֵ<<<<<<<<<<<<<<<<<<< */
        thr_tmp += LIMIT(zSpeed_pid.out,-200,200);
			/* 1.2���޷�����ֵ900����������10%��PIDʹ�� */
        thr_tmp = LIMIT(thr_tmp, 0, 900);
      /* 1.3�����޷��������ֵ�������ڵ���� */
        motor1 = motor2 = motor3 = motor4 = thr_tmp;
			/*����PID���*/
		 /* 2��PID������� */
        /*
            motor1--����
            motor2--����
            motor3--����
            motor4--����

            ����PID��� ==��  1��2һ�飬3��4һ�飬��������෴
            ���PID��� ==��  1��4һ�飬2��3һ�飬��������෴
            ƫ��PID��� ==��  1��3һ�飬2��4һ�飬��������෴
         */
				motor1 += +gyroY_pid.out + gyroX_pid.out + gyroZ_pid.out;
        motor2 += +gyroY_pid.out - gyroX_pid.out - gyroZ_pid.out;
        motor3 += -gyroY_pid.out - gyroX_pid.out + gyroZ_pid.out;
        motor4 += -gyroY_pid.out + gyroX_pid.out - gyroZ_pid.out;

			/* ����ָ����������˳����� */
        if (isUnlock == 0)
        {
            status = UNLOCK_0;
        }

        /* Ϊ�˵��԰�ȫ���������ʱ������PID */
        // if (thr_tmp <= 10)
        /* ���붨�ߺ󣬰�ȫ��ʩҪ��ԭʼֵ�ж� */
        if (receiveRemote.THR <= 1010)
        {
            motor1 = motor2 = motor3 = motor4 = 0;
        }

        break;
    default:
        break;
    }
			/*�����PWM */
 /* ͳһ������TIM��CCR�� */
    /*
        ��ǰ��TIM3��CH1  ==> motor1
        ��ǰ��TIM2-CH2  ==> motor2
        �Һ�TIM1-CH3  ==> motor3
        ���TIM4-CH4  ==> motor4
    */
    TIM_SetCompare3(TIM3, LIMIT(motor1, 0, 1000));
		TIM_SetCompare3(TIM3, LIMIT(motor2, 0, 1000));
		TIM_SetCompare3(TIM3, LIMIT(motor3, 0, 1000));
		TIM_SetCompare3(TIM3, LIMIT(motor4, 0, 1000));

}

/**
 * @description: λ�ƿ��ƣ�ǰ�������ҡ�ˮƽ��ת
 * @return {*}
 */
void App_Flight_Move(void)
{
    /* ҡ��ֵ�ͽǶ�ֵ�Ļ������ ��
        ���磺 ��ҡ������--����
            ��������2000,2000-1500=500�Ĳ�ֵ ��Ӧ 20��Ƕ�
                20/500 = 0.04
            ��������1000,1000-1500=-500�Ĳ�ֵ ��Ӧ -20��Ƕ�
                -20/-500 = 0.04
    */
    float move_ratio = 0.04;
    /* ��ҡ��ֵת���ɽǶȵ�ȡֵ ===�� ������Ƕ�ֵ ��ֵ�� PID�⻷���Ƕȣ�������ֵ */
    /* ����ǰ��ɣ�������̬  */
    pitch_pid.expect = move_ratio * (receiveRemote.PIT - 1500);
    /* �������ҷɣ������̬ */
    roll_pid.expect = move_ratio * (receiveRemote.ROL - 1500);
    /* ����ˮƽ��ת��ƫ����̬(ƫ����ֵ���=ʵ��ֵ/2,�ֶ�����һ��) */
    yaw_pid.expect = 2 * move_ratio * (receiveRemote.YAW - 1500);
}


/**
 * @description: PID������ֵ
 * @return {*}
 */
void App_Flight_PID_Init(void)
{
    /* ========������̬============ */
    /* �ڻ���Y����ٶ� */
    gyroY_pid.kp = +3.00f; // +3.00
    gyroY_pid.ki = 0.00f;
    gyroY_pid.kd = +0.085f; // +0.085
    /* �⻷�������� */
    pitch_pid.kp = -7.00f; // -7.00
    pitch_pid.ki = 0.00f;
    pitch_pid.kd = 0.00f;

    /* ========�����̬============ */
    /* �ڻ���X����ٶ� */
    gyroX_pid.kp = -3.00f; // -3.00
    gyroX_pid.ki = 0.00f;
    gyroX_pid.kd = -0.085f; //-0.085
    /* �⻷������� */
    roll_pid.kp = -7.00f; //-7.0
    roll_pid.ki = 0.00f;
    roll_pid.kd = 0.00f;

    /* ========ƫ����̬============ */
    /* �ڻ���Z����ٶ� */
    gyroZ_pid.kp = +2.00f; // +2.00
    gyroZ_pid.ki = 0.00f;
    gyroZ_pid.kd = 0.00f;
    /* �⻷��ƫ���� */
    yaw_pid.kp = -2.00f; // -2.00
    yaw_pid.ki = 0.00f;
    yaw_pid.kd = 0.00f;


    /* ========���ߴ���============ */
    /* �ڻ���Z���ٶ� */
    zSpeed_pid.kp = -1.20f;
    zSpeed_pid.ki = 0.00f;
    zSpeed_pid.kd = -0.085f;
    /* �⻷���߶� */
    height_pid.kp = -1.20f;
    height_pid.ki = 0.00f;
    height_pid.kd = -0.085f;
}









