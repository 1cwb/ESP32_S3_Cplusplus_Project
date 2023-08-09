// #ifdef _MPU_DMP_DRIVER_H
// #define _MPU_DMP_DRIVER_H
#pragma once
#include "mpu6050driver.h"
// #define MPU6500
#define MPU_I2C_SCL 2
#define MPU_I2C_SDA 1

class MPU6050Dmp
{
public:
    MPU6050Dmp()
    {
    }
    ~MPU6050Dmp()
    {
    }
    MPU6050Dmp(const MPU6050Dmp&) = delete;
    MPU6050Dmp(MPU6050Dmp&&) = delete;
    MPU6050Dmp& operator=(const MPU6050Dmp&) =delete;
    MPU6050Dmp& operator=(MPU6050Dmp&&) =delete;
    bool init(int32_t sda = MPU_I2C_SDA, int32_t scl = MPU_I2C_SCL, uint8_t addr = MPU_ADDR, i2c_port_t i2cNum = I2C_NUM_0, uint32_t clockSpeed = 200000)
    {
        if(!Mpu6050::getInstance()->init(sda, scl, addr, i2cNum, clockSpeed))
        {
            printf("Error : i2c init fail\n");
            return false;
        }
        if(mpu_dmp_init() != 0)
        {
            return false;
        }
        return true;
    }
    bool getData(float* pitch, float* roll, float* yaw)
    {
        gyro_data_ready_cb();
        return dmp_get_data(pitch, roll, yaw) == 0;
    }
    void run_self_test(void);
private:
    uint8_t dmp_get_data(float* pitch, float* roll, float* yaw);
    void gyro_data_ready_cb(void);
    uint8_t mpu_dmp_init(void);
    void system_reset(void);
};
// #endif // DEBUG