#include "math.h"
#include <stdbool.h>
#include "imu.h"
#include "fly_control.h"

int16_t  Gax, Gay, Gaz;

const float RtA = 57.2957795f;
const float Gyro_G = 0.03051756f * 2; // 陀螺仪初始化量程+-2000度每秒于1 / (65536 / 4000) = 0.03051756*2
const float Gyro_Gr = 0.0005326f * 2; // 面计算度每秒,转换弧度每秒则 2*0.03051756    * 0.0174533f = 0.0005326*2

static float NormAccz;

//=========================姿态解算==============================
typedef volatile struct
{
	float q0;
	float q1;
	float q2;
	float q3;
} Quaternion;

// Fast inverse square-root
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root
float invSqrt(float number)	/*快速开平方求倒*/
{
	long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(long *)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float *)&i;
    y = y * (threehalfs - (x2 * y * y)); // 1st iteration （第一次牛顿迭代）
    return y;
}
/*
 * 函数名：mahony_update
 * 描述  ：姿态解算
 * 输入  ：陀螺仪三轴数据（单位：弧度/秒），加速度三轴数据（单位：g）
	时间间隔dt
 * 返回  ：
 */
void imuUpdate(const _st_Mpu *pMpu, _st_AngE *state , float dt)	/*数据融合 互补滤波*/
{
	volatile struct V
    {
        float x;
        float y;
        float z;
    } Gravity, Acc, Gyro, AccGravity;

    static struct V GyroIntegError = {0};
    static float KpDef = 6.8f;
    static float KiDef = 0.0008f;
    static Quaternion NumQ = {1, 0, 0, 0};
    float q0_t, q1_t, q2_t, q3_t;
    // float NormAcc;
    float NormQuat;
    float HalfTime = dt * 0.5f;

    // 提取等效旋转矩阵中的重力分量
    Gravity.x = 2 * (NumQ.q1 * NumQ.q3 - NumQ.q0 * NumQ.q2);
    Gravity.y = 2 * (NumQ.q0 * NumQ.q1 + NumQ.q2 * NumQ.q3);
    Gravity.z = 1 - 2 * (NumQ.q1 * NumQ.q1 + NumQ.q2 * NumQ.q2);
    // 加速度归一化
    NormQuat = invSqrt(squa(MPU6050.accX) + squa(MPU6050.accY) + squa(MPU6050.accZ));

    Acc.x = pMpu->accX * NormQuat;
    Acc.y = pMpu->accY * NormQuat;
    Acc.z = pMpu->accZ * NormQuat;
    // 向量差乘得出的值
    AccGravity.x = (Acc.y * Gravity.z - Acc.z * Gravity.y);
    AccGravity.y = (Acc.z * Gravity.x - Acc.x * Gravity.z);
    AccGravity.z = (Acc.x * Gravity.y - Acc.y * Gravity.x);
    // 再做加速度积分补偿角速度的补偿值
    GyroIntegError.x += AccGravity.x * KiDef;
    GyroIntegError.y += AccGravity.y * KiDef;
    GyroIntegError.z += AccGravity.z * KiDef;
    // 角速度融合加速度积分补偿值
    Gyro.x = pMpu->gyroX * Gyro_Gr + KpDef * AccGravity.x + GyroIntegError.x; // 弧度制
    Gyro.y = pMpu->gyroY * Gyro_Gr + KpDef * AccGravity.y + GyroIntegError.y;
    Gyro.z = pMpu->gyroZ * Gyro_Gr + KpDef * AccGravity.z + GyroIntegError.z;

    // 一阶龙格库塔法, 更新四元数
    q0_t = (-NumQ.q1 * Gyro.x - NumQ.q2 * Gyro.y - NumQ.q3 * Gyro.z) * HalfTime;
    q1_t = (NumQ.q0 * Gyro.x - NumQ.q3 * Gyro.y + NumQ.q2 * Gyro.z) * HalfTime;
    q2_t = (NumQ.q3 * Gyro.x + NumQ.q0 * Gyro.y - NumQ.q1 * Gyro.z) * HalfTime;
    q3_t = (-NumQ.q2 * Gyro.x + NumQ.q1 * Gyro.y + NumQ.q0 * Gyro.z) * HalfTime;

    NumQ.q0 += q0_t;
    NumQ.q1 += q1_t;
    NumQ.q2 += q2_t;
    NumQ.q3 += q3_t;

    // 四元数归一化
    NormQuat = invSqrt(squa(NumQ.q0) + squa(NumQ.q1) + squa(NumQ.q2) + squa(NumQ.q3));
    NumQ.q0 *= NormQuat;
    NumQ.q1 *= NormQuat;
    NumQ.q2 *= NormQuat;
    NumQ.q3 *= NormQuat;

    {
        /*机体坐标系下的Z方向向量*/
        float vecxZ = 2 * NumQ.q0 * NumQ.q2 - 2 * NumQ.q1 * NumQ.q3;     /*矩阵(3,1)项*/
        float vecyZ = 2 * NumQ.q2 * NumQ.q3 + 2 * NumQ.q0 * NumQ.q1;     /*矩阵(3,2)项*/
        float veczZ = 1 - 2 * NumQ.q1 * NumQ.q1 - 2 * NumQ.q2 * NumQ.q2; /*矩阵(3,3)项*/
#ifdef YAW_GYRO
        *(float *)state = atan2f(2 * NumQ.q1 * NumQ.q2 + 2 * NumQ.q0 * NumQ.q3, 1 - 2 * NumQ.q2 * NumQ.q2 - 2 * NumQ.q3 * NumQ.q3) * RtA; // yaw
#else
        float yaw_G = pMpu->gyroZ * Gyro_G;    // 将Z轴角速度陀螺仪值 转换为Z角度/秒      Gyro_G陀螺仪初始化量程+-2000度每秒于1 / (65536 / 4000) = 0.03051756*2
        if ((yaw_G > 1.0f) || (yaw_G < -1.0f)) // 数据太小可以认为是干扰，不是偏航动作
        {
            state->yaw += yaw_G * dt; // 角速度积分成偏航角
        }
#endif
        state->pitch = asin(vecxZ) * RtA; // 俯仰角

        state->roll = atan2f(vecyZ, veczZ) * RtA; // 横滚角

        NormAccz = pMpu->accX * vecxZ + pMpu->accY * vecyZ + pMpu->accZ * veczZ; /*Z轴垂直方向上的加速度，此值涵盖了倾斜时在Z轴角速度的向量和，不是单纯重力感应得出的值*/
    }
}




void updateYaw(_st_AngE *state) {
    if (state == NULL) return; // 检查输入指针是否为空

    int16_t magX = Gax, magY = Gay, magZ = Gaz; // 假设Gax, Gay, Gaz已通过updateHMC5883更新

    // 翻转磁力计轴 (如果需要)
    magX *= -1; // 翻转 X 轴
    magZ *= -1; // 翻转 Z 轴

    // 应用磁力计增益补偿和偏移量补偿
    static const float magOffset[] = {0.0f, 0.0f, 0.0f}; // 需要根据实际校准数据填充
    static const float magGain[] = {1.0f, 1.0f, 1.0f};   // 同上
    magX = (magX - magOffset[0]) * magGain[0];
    magY = (magY - magOffset[1]) * magGain[1];
    magZ = (magZ - magOffset[2]) * magGain[2];

    // 获取当前的横滚角和俯仰角（弧度）
    double rollAngle = state->roll * DEG2RAD;
    double pitchAngle = state->pitch * DEG2RAD;

    // 计算磁场矢量在水平面的分量 Bfx 和垂直于水平面的分量 Bfy
    double Bfy = magZ * sin(rollAngle) - magY * cos(rollAngle);
    double Bfx = magX * cos(pitchAngle) + magY * sin(pitchAngle) * sin(rollAngle) + magZ * sin(pitchAngle) * cos(rollAngle);

    // 使用 atan2 函数来计算偏航角
    state->yaw = atan2(-Bfy, Bfx) * RAD2DEG;

    // 根据需要翻转偏航角方向
    state->yaw *= -1;

    // 可选：限制偏航角在 [-180, 180] 度之间
    if (state->yaw > 180.0f) state->yaw -= 360.0f;
    else if (state->yaw < -180.0f) state->yaw += 360.0f;
}

float GetAccz(void)
{
		return NormAccz;
}






