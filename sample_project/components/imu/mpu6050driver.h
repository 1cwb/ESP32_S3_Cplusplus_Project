#pragma once
#include "mi2c.h"
#include "esp_timer.h"
//****************************************
// 定义MPU6050内部地址
//****************************************
#define MPU_SELF_TESTX_REG		0X0D	//自检寄存器X
#define MPU_SELF_TESTY_REG		0X0E	//自检寄存器Y
#define MPU_SELF_TESTZ_REG		0X0F	//自检寄存器Z
#define MPU_SELF_TESTA_REG		0X10	//自检寄存器A
#define MPU_SAMPLE_RATE_REG		0X19	//采样频率分频器
#define MPU_CFG_REG				0X1A	//配置寄存器
#define MPU_GYRO_CFG_REG	    0X1B	//陀螺仪配置寄存器
#define MPU_ACCEL_CFG_REG		0X1C	//加速度计配置寄存器
#define MPU_MOTION_DET_REG		0X1F	//运动检测阀值设置寄存器
#define MPU_FIFO_EN_REG			0X23	//FIFO使能寄存器
#define MPU_I2CMST_CTRL_REG		0X24	//IIC主机控制寄存器
#define MPU_I2CSLV0_ADDR_REG	0X25	//IIC从机0器件地址寄存器
#define MPU_I2CSLV0_REG			0X26	//IIC从机0数据地址寄存器
#define MPU_I2CSLV0_CTRL_REG	0X27	//IIC从机0控制寄存器
#define MPU_I2CSLV1_ADDR_REG	0X28	//IIC从机1器件地址寄存器
#define MPU_I2CSLV1_REG			0X29	//IIC从机1数据地址寄存器
#define MPU_I2CSLV1_CTRL_REG	0X2A	//IIC从机1控制寄存器
#define MPU_I2CSLV2_ADDR_REG	0X2B	//IIC从机2器件地址寄存器
#define MPU_I2CSLV2_REG			0X2C	//IIC从机2数据地址寄存器
#define MPU_I2CSLV2_CTRL_REG	0X2D	//IIC从机2控制寄存器
#define MPU_I2CSLV3_ADDR_REG	0X2E	//IIC从机3器件地址寄存器
#define MPU_I2CSLV3_REG			0X2F	//IIC从机3数据地址寄存器
#define MPU_I2CSLV3_CTRL_REG	0X30	//IIC从机3控制寄存器
#define MPU_I2CSLV4_ADDR_REG	0X31	//IIC从机4器件地址寄存器
#define MPU_I2CSLV4_REG			0X32	//IIC从机4数据地址寄存器
#define MPU_I2CSLV4_DO_REG		0X33	//IIC从机4写数据寄存器
#define MPU_I2CSLV4_CTRL_REG	0X34	//IIC从机4控制寄存器
#define MPU_I2CSLV4_DI_REG		0X35	//IIC从机4读数据寄存器

#define MPU_I2CMST_STA_REG		0X36	//IIC主机状态寄存器
#define MPU_INTBP_CFG_REG		0X37	//中断/旁路设置寄存器
#define MPU_INT_EN_REG			0X38	//中断使能寄存器
#define MPU_INT_STA_REG			0X3A	//中断状态寄存器

#define MPU_ACCEL_XOUTH_REG		0X3B	//加速度值,X轴高8位寄存器
#define MPU_ACCEL_XOUTL_REG		0X3C	//加速度值,X轴低8位寄存器
#define MPU_ACCEL_YOUTH_REG		0X3D	//加速度值,Y轴高8位寄存器
#define MPU_ACCEL_YOUTL_REG		0X3E	//加速度值,Y轴低8位寄存器
#define MPU_ACCEL_ZOUTH_REG		0X3F	//加速度值,Z轴高8位寄存器
#define MPU_ACCEL_ZOUTL_REG		0X40	//加速度值,Z轴低8位寄存器

#define MPU_TEMP_OUTH_REG		0X41	//温度值高八位寄存器
#define MPU_TEMP_OUTL_REG		0X42	//温度值低8位寄存器

#define MPU_GYRO_XOUTH_REG		0X43	//陀螺仪值,X轴高8位寄存器
#define MPU_GYRO_XOUTL_REG		0X44	//陀螺仪值,X轴低8位寄存器
#define MPU_GYRO_YOUTH_REG		0X45	//陀螺仪值,Y轴高8位寄存器
#define MPU_GYRO_YOUTL_REG		0X46	//陀螺仪值,Y轴低8位寄存器
#define MPU_GYRO_ZOUTH_REG		0X47	//陀螺仪值,Z轴高8位寄存器
#define MPU_GYRO_ZOUTL_REG		0X48	//陀螺仪值,Z轴低8位寄存器

#define MPU_I2CSLV0_DO_REG		0X63	//IIC从机0数据寄存器
#define MPU_I2CSLV1_DO_REG		0X64	//IIC从机1数据寄存器
#define MPU_I2CSLV2_DO_REG		0X65	//IIC从机2数据寄存器
#define MPU_I2CSLV3_DO_REG		0X66	//IIC从机3数据寄存器

#define MPU_I2CMST_DELAY_REG	0X67	//IIC主机延时管理寄存器
#define MPU_SIGPATH_RST_REG		0X68	//信号通道复位寄存器
#define MPU_MDETECT_CTRL_REG	0X69	//运动检测控制寄存器
#define MPU_USER_CTRL_REG		0X6A	//用户控制寄存器
#define MPU_PWR_MGMT1_REG		0X6B	//电源管理寄存器1
#define MPU_PWR_MGMT2_REG		0X6C	//电源管理寄存器2 
#define MPU_FIFO_CNTH_REG		0X72	//FIFO计数寄存器高八位
#define MPU_FIFO_CNTL_REG		0X73	//FIFO计数寄存器低八位
#define MPU_FIFO_RW_REG			0X74	//FIFO读写寄存器
#define MPU_DEVICE_ID_REG		0X75	//器件ID寄存器
 
//如果AD0脚(9脚)接地,IIC地址为0X68(不包含最低位).
//如果接V3.3,则IIC地址为0X69(不包含最低位).
#define MPU_ADDR				0X68

class Mpu6050
{
public:
    static Mpu6050* getInstance()
    {
        static Mpu6050 mpu6050_;
        return &mpu6050_;
    }
    uint8_t whoAmI()
    {
        uint8_t who = 0;
        readRegister(MPU_DEVICE_ID_REG, &who, 1);
        return who;
    }
    int16_t getTemperature(void)
    {
        uint8_t buf[2]; 
        int16_t raw;
        float temp;
        
        readRegister(MPU_TEMP_OUTH_REG,buf,2); 
        raw=((uint16_t)buf[0]<<8)|buf[1];
        temp=36.53+((double)raw)/340;
        return temp*100;
    }
    bool getGyroScope(int16_t *gx,int16_t *gy,int16_t *gz)
    {
        bool res = false;
        uint8_t buf[6];
        
        res=readRegister(MPU_GYRO_XOUTH_REG,buf,6);
        if(res)
        {
            *gx=((uint16_t)buf[0]<<8)|buf[1];
            *gy=((uint16_t)buf[2]<<8)|buf[3];
            *gz=((uint16_t)buf[4]<<8)|buf[5];
        } 	
        return res;
    }
    bool getAccelErometer(int16_t *ax, int16_t *ay,int16_t *az)
    {
        bool res = false;
        uint8_t buf[6];  
        res=readRegister(MPU_ACCEL_XOUTH_REG,buf,6);
        if(res)
        {
            *ax=((uint16_t)buf[0]<<8)|buf[1];
            *ay=((uint16_t)buf[2]<<8)|buf[3]; 
            *az=((uint16_t)buf[4]<<8)|buf[5];
        } 	
        return res;
    }
    bool init(int32_t sda, int32_t scl, uint8_t addr = MPU_ADDR, i2c_port_t i2cNum = I2C_NUM_0, uint32_t clockSpeed = 200000)
    {
        i2cmaster_.init(sda, scl, i2cNum, clockSpeed);
        addr_ = addr;
        if(!writeRegister(MPU_PWR_MGMT1_REG,0X80))	//reset
        {
            printf("reset mpu fail\n");
        }
        vTaskDelay(100/portTICK_PERIOD_MS);
        writeRegister(MPU_PWR_MGMT1_REG,0X00);  //wake up
        MPU_Set_Gyro_Fsr(3);

        MPU_Set_Accel_Fsr(0);                   //加速度传感器,±2g
        MPU_Set_Rate(50);                       //设置采样率50Hz
        writeRegister(MPU_INT_EN_REG,0X00);     //关闭所有中断
        writeRegister(MPU_USER_CTRL_REG,0X00);	//I2C主模式关闭
        writeRegister(MPU_FIFO_EN_REG,0X00);	//关闭FIFO
        writeRegister(MPU_INTBP_CFG_REG,0X80);	//INT引脚低电平有效

        if(whoAmI() == MPU_ADDR)				//器件ID正确,即res = MPU_ADDR = 0x68
        {
            writeRegister(MPU_PWR_MGMT1_REG,0X01);		//设置CLKSEL,PLL X轴为参考
            writeRegister(MPU_PWR_MGMT2_REG,0X00);		//加速度与陀螺仪都工作
            MPU_Set_Rate(50);							//设置采样率为50Hz
        }
        printf("i am %X\n",whoAmI());
        return true;
    }
    uint8_t MPU_Read_Len(uint8_t addr,uint8_t reg,uint8_t len,uint8_t *buf)
    {
        //return i2cmaster_.writeReadDevice(addr,&reg,1,buf,len,1000/portTICK_PERIOD_MS) == true ? 0 : -1;
        if (len == 0)
        {
            return ESP_OK;
        }
        i2c_cmd_handle_t cmd = i2cmaster_.cmdLinkCreate();
        i2cmaster_.cmdStart(cmd);
        i2cmaster_.masterWriteByte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2cmaster_.masterWriteByte(cmd, reg, true);
        i2cmaster_.cmdStart(cmd);
        i2cmaster_.masterWriteByte(cmd, (addr << 1) | I2C_MASTER_READ, true);
        if (len > 1)
        {
            i2cmaster_.masterRead(cmd, buf, len - 1, I2C_MASTER_ACK);
        }
        i2cmaster_.masterReadByte(cmd, buf + len - 1, I2C_MASTER_NACK);
        i2cmaster_.masterStop(cmd);
        bool bret = i2cmaster_.masterCmdBegin(cmd, 1000 / portTICK_PERIOD_MS);
        i2cmaster_.cmdLinkDelete(cmd);
        return bret ? 0 : -1;
    }
    uint8_t MPU_Write_Len(uint8_t addr,uint8_t reg,uint8_t len,const uint8_t *buf)
    {
        //uint8_t writeBuff[len+1] = {reg};
        //memcpy(writeBuff+1, buf, len);
        //return i2cmaster_.writeToDevice(addr, writeBuff, sizeof(writeBuff), 1000/portTICK_PERIOD_MS) == true ? 0 : -1;
        i2c_cmd_handle_t cmd = i2cmaster_.cmdLinkCreate();
        i2cmaster_.cmdStart(cmd);
        i2cmaster_.masterWriteByte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
        i2cmaster_.masterWriteByte(cmd, reg, true);
        i2cmaster_.masterWrite(cmd, buf, len, true);
        i2cmaster_.masterStop(cmd);
        bool bret = i2cmaster_.masterCmdBegin(cmd, 1000 / portTICK_PERIOD_MS);
        i2cmaster_.cmdLinkDelete(cmd);
        // printf("esp32_i2c_write\n");
        return bret ? 0 : -1;

    }
private:
    Mpu6050()
    {
    }
    ~Mpu6050()
    {
    }
    Mpu6050(const Mpu6050&) = delete;
    Mpu6050(Mpu6050&&) = delete;
    Mpu6050& operator=(const Mpu6050&) =delete;
    Mpu6050& operator=(Mpu6050&&) =delete;
    bool readRegister(uint8_t regAddr, uint8_t *data, size_t len)
    {
        //return i2cmaster_.writeReadDevice(addr_,&regAddr,1,data,len,1000/portTICK_PERIOD_MS);
        return MPU_Read_Len(addr_, regAddr, len, data) == 0 ? true : false;
    }
    bool writeRegister(uint8_t regAddr, uint8_t data)
    {
        //const uint8_t writeBuf[2] = {regAddr, data};
        //return i2cmaster_.writeToDevice(addr_, writeBuf, sizeof(writeBuf), 1000/portTICK_PERIOD_MS);
        return  MPU_Write_Len(addr_,regAddr,1,&data) == 0 ? true : false;
    }
    /**********************************************
    函数名称：MPU_Set_Gyro_Fsr
    函数功能：设置MPU6050陀螺仪传感器满量程范围
    函数参数：fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
    函数返回值：0,设置成功  其他,设置失败
    **********************************************/
    bool MPU_Set_Gyro_Fsr(uint8_t fsr)
    {
        return writeRegister(MPU_GYRO_CFG_REG,fsr<<3); //设置陀螺仪满量程范围
    }
    /**********************************************
    函数名称：MPU_Set_Accel_Fsr
    函数功能：设置MPU6050加速度传感器满量程范围
    函数参数：fsr:0,±2g;1,±4g;2,±8g;3,±16g
    函数返回值：0,设置成功  其他,设置失败
    **********************************************/
    bool MPU_Set_Accel_Fsr(uint8_t fsr)
    {
        return writeRegister(MPU_ACCEL_CFG_REG,fsr<<3); //设置加速度传感器满量程范围  
    }
    /**********************************************
    函数名称：MPU_Set_LPF
    函数功能：设置MPU6050的数字低通滤波器
    函数参数：lpf:数字低通滤波频率(Hz)
    函数返回值：0,设置成功  其他,设置失败
    **********************************************/
    bool MPU_Set_LPF(uint16_t lpf)
    {
        uint8_t data=0;
        
        if(lpf>=188)data=1;
        else if(lpf>=98)data=2;
        else if(lpf>=42)data=3;
        else if(lpf>=20)data=4;
        else if(lpf>=10)data=5;
        else data=6; 
        return writeRegister(MPU_CFG_REG,data);//设置数字低通滤波器  
    }

    /**********************************************
    函数名称：MPU_Set_Rate
    函数功能：设置MPU6050的采样率(假定Fs=1KHz)
    函数参数：rate:4~1000(Hz)  初始化中rate取50
    函数返回值：0,设置成功  其他,设置失败
    **********************************************/
    bool MPU_Set_Rate(uint16_t rate)
    {
        uint8_t data;
        if(rate>1000)rate=1000;
        if(rate<4)rate=4;
        data=1000/rate-1;
        data=writeRegister(MPU_SAMPLE_RATE_REG,data);	//设置数字低通滤波器
        return MPU_Set_LPF(rate/2);					    //自动设置LPF为采样率的一半
    }

private:
    MI2CMaster i2cmaster_;//(1, 2);
    uint8_t addr_;
};
