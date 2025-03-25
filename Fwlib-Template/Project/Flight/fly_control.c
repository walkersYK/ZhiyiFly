#include "config.h"
#include "imu.h"
#include "fly_control.h"
#include "PID_CTL.h"

//********************************
// pid 控制电机输出***************
//********************************
extern sReceiveRemote receiveRemote; // 保存接收到的遥控器控制数据
uint8_t isUnlock = 0; // 是否解锁的标志位：0-未解锁，1-解锁

/* 俯仰姿态PID：外环：俯仰角；内环：Y轴角速度 */
sPID pitch_pid;
sPID gyroY_pid;
/* 横滚姿态PID：外环：横滚角；内环：X轴角速度 */
sPID roll_pid;
sPID gyroX_pid;
/* 偏航姿态PID：外环：偏航角；内环：Z轴角速度 */
sPID yaw_pid;
sPID gyroZ_pid;


/* 定高PID:外环-》高度；内环-》z轴速度 */
sPID height_pid; // 高度pid
sPID zSpeed_pid; // z轴速度pid

/* pid结构体地址的数组，方便统一重置 */
sPID *pids[8] = {&pitch_pid, &gyroY_pid, &roll_pid, &gyroX_pid, &yaw_pid, &gyroZ_pid, &height_pid, &zSpeed_pid};

extern float Gyro_G ;

/* 对应4个电机的值 */
int16_t motor1, motor2, motor3, motor4;
/**
 * @description: 姿态控制PID处理
 * @param {float} dt    周期，单位秒
 * @return {*}
 */
void App_Flight_PosturePID(float dt)
{
    static uint8_t status = UNLOCK_0;
    switch (status)
    {
    case UNLOCK_0:
        /* 锁定状态 */
        if (isUnlock == 1)
        {
            status = UNLOCK_PROCESS;
            /* 重置PID，避免重新解锁后，影响新的PID计算 */
            Com_PID_Reset(pids, 6);
        }
        break;
    case UNLOCK_PROCESS:
        /* 锁定状态 */

        /* 欧拉角测量值 */
        pitch_pid.measure = Angle.pitch;
        roll_pid.measure = Angle.roll;
        yaw_pid.measure = Angle.yaw;

        /* 角速度测量值 */
        gyroY_pid.measure = MPU6050.gyroY * Gyro_G;
        gyroX_pid.measure = MPU6050.gyroX * Gyro_G;
        gyroZ_pid.measure = MPU6050.gyroZ * Gyro_G;

        /* 俯仰姿态串级PID处理 */
        Com_PID_Cascade(&pitch_pid, &gyroY_pid, dt);
        /* 横滚姿态串级PID处理 */
        Com_PID_Cascade(&roll_pid, &gyroX_pid, dt);
        /* 偏航姿态串级PID处理 */
        Com_PID_Cascade(&yaw_pid, &gyroZ_pid, dt);

        /* 如果恢复锁定，退出PID处理 */
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
 * @description: 控制电机：将PID结果和油门值作用于电机
 * @return {*}
 */
void App_Flight_Motor(void)
{
	static uint8_t status = UNLOCK_0;
    int16_t thr_tmp = 0;
    switch (status)
    {
    case UNLOCK_0:
        /* 锁定状态 */
        if (isUnlock == 1)
        {
            status = UNLOCK_PROCESS;
            /* 电机复位 */
            motor1 = motor2 = motor3 = motor4 = 0;
        }
        break;
    case UNLOCK_PROCESS:
			/*正式控制阶段*/
		
		
			/*先处理油门值*/
			/* 1.1、处理油门值的取值，匹配 ccr的取值范围0-1000 */
        thr_tmp = receiveRemote.THR - 1000;
        /* >>>>>>>>>>>>>>>>>>定高PID结果作用于油门值<<<<<<<<<<<<<<<<<<< */
        thr_tmp += LIMIT(zSpeed_pid.out,-200,200);
			/* 1.2、限幅油门值900，至少留出10%给PID使用 */
        thr_tmp = LIMIT(thr_tmp, 0, 900);
      /* 1.3、将限幅后的油门值，作用于电机上 */
        motor1 = motor2 = motor3 = motor4 = thr_tmp;
			/*处理PID结果*/
		 /* 2、PID结果处理 */
        /*
            motor1--左上
            motor2--右上
            motor3--右下
            motor4--左下

            俯仰PID结果 ==》  1、2一组，3、4一组，两组符号相反
            横滚PID结果 ==》  1、4一组，2、3一组，两组符号相反
            偏航PID结果 ==》  1、3一组，2、4一组，两组符号相反
         */
				motor1 += +gyroY_pid.out + gyroX_pid.out + gyroZ_pid.out;
        motor2 += +gyroY_pid.out - gyroX_pid.out - gyroZ_pid.out;
        motor3 += -gyroY_pid.out - gyroX_pid.out + gyroZ_pid.out;
        motor4 += -gyroY_pid.out + gyroX_pid.out - gyroZ_pid.out;

			/* 如果恢复成锁定，退出控制 */
        if (isUnlock == 0)
        {
            status = UNLOCK_0;
        }

        /* 为了调试安全，油门最低时，禁用PID */
        // if (thr_tmp <= 10)
        /* 加入定高后，安全措施要用原始值判断 */
        if (receiveRemote.THR <= 1010)
        {
            motor1 = motor2 = motor3 = motor4 = 0;
        }

        break;
    default:
        break;
    }
			/*输出到PWM */
 /* 统一作用于TIM的CCR中 */
    /*
        左前：TIM3—CH1  ==> motor1
        右前：TIM2-CH2  ==> motor2
        右后：TIM1-CH3  ==> motor3
        左后：TIM4-CH4  ==> motor4
    */
    TIM_SetCompare3(TIM3, LIMIT(motor1, 0, 1000));
		TIM_SetCompare3(TIM3, LIMIT(motor2, 0, 1000));
		TIM_SetCompare3(TIM3, LIMIT(motor3, 0, 1000));
		TIM_SetCompare3(TIM3, LIMIT(motor4, 0, 1000));

}

/**
 * @description: 位移控制：前、后、左、右、水平旋转
 * @return {*}
 */
void App_Flight_Move(void)
{
    /* 摇杆值和角度值的换算比例 ：
        比如： 右摇杆上下--俯仰
            打到最上面2000,2000-1500=500的差值 对应 20°角度
                20/500 = 0.04
            打到最下面1000,1000-1500=-500的差值 对应 -20°角度
                -20/-500 = 0.04
    */
    float move_ratio = 0.04;
    /* 将摇杆值转换成角度的取值 ===》 将这个角度值 赋值给 PID外环（角度）的期望值 */
    /* 控制前后飞：俯仰姿态  */
    pitch_pid.expect = move_ratio * (receiveRemote.PIT - 1500);
    /* 控制左右飞：横滚姿态 */
    roll_pid.expect = move_ratio * (receiveRemote.ROL - 1500);
    /* 控制水平旋转：偏航姿态(偏航的值大概=实际值/2,手动补偿一下) */
    yaw_pid.expect = 2 * move_ratio * (receiveRemote.YAW - 1500);
}


/**
 * @description: PID参数赋值
 * @return {*}
 */
void App_Flight_PID_Init(void)
{
    /* ========俯仰姿态============ */
    /* 内环：Y轴角速度 */
    gyroY_pid.kp = +3.00f; // +3.00
    gyroY_pid.ki = 0.00f;
    gyroY_pid.kd = +0.085f; // +0.085
    /* 外环：俯仰角 */
    pitch_pid.kp = -7.00f; // -7.00
    pitch_pid.ki = 0.00f;
    pitch_pid.kd = 0.00f;

    /* ========横滚姿态============ */
    /* 内环：X轴角速度 */
    gyroX_pid.kp = -3.00f; // -3.00
    gyroX_pid.ki = 0.00f;
    gyroX_pid.kd = -0.085f; //-0.085
    /* 外环：横滚角 */
    roll_pid.kp = -7.00f; //-7.0
    roll_pid.ki = 0.00f;
    roll_pid.kd = 0.00f;

    /* ========偏航姿态============ */
    /* 内环：Z轴角速度 */
    gyroZ_pid.kp = +2.00f; // +2.00
    gyroZ_pid.ki = 0.00f;
    gyroZ_pid.kd = 0.00f;
    /* 外环：偏航角 */
    yaw_pid.kp = -2.00f; // -2.00
    yaw_pid.ki = 0.00f;
    yaw_pid.kd = 0.00f;


    /* ========定高处理============ */
    /* 内环：Z轴速度 */
    zSpeed_pid.kp = -1.20f;
    zSpeed_pid.ki = 0.00f;
    zSpeed_pid.kd = -0.085f;
    /* 外环：高度 */
    height_pid.kp = -1.20f;
    height_pid.ki = 0.00f;
    height_pid.kd = -0.085f;
}









