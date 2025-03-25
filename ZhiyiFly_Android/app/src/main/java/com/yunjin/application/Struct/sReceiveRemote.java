package com.yunjin.application.Struct;

public class sReceiveRemote {
    // 定义与 STM32 结构体完全一致的字段
    public short THR;  // 油门值 (int16_t)
    public short YAW;  // 偏航值 (int16_t)
    public short ROL;  // 横滚值 (int16_t)
    public short PIT;  // 俯仰值 (int16_t)

    // 构造函数
    public sReceiveRemote() {
        THR = 0;
        YAW = 0;
        ROL = 0;
        PIT = 0;
    }
}