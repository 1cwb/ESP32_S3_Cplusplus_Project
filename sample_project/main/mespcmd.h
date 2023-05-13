#pragma once
#include <iostream>
#include <cstring>
using namespace std;

enum EspCmdId
{
    E_ESP_CMD_ID_INVALID,
    E_ESP_CMD_ID_MOTOR_CTRL,
    E_ESP_CMD_ID_MOTOR_STATUS,
    E_ESP_CMD_ID_SEND_STR,
    E_ESP_CMD_ID_MAX
};
enum EMotorCmd
{
    E_MOTOR_CMD_COAST,
    E_MOTOR_CMD_FORWARD,
    E_MOTOR_CMD_REVERSE,
    E_MOTOR_CMD_BRAKE,
    E_MOTOR_CMD_SET_SPEED
};
struct stMotorCmd
{
    EMotorCmd ecmd;
    uint32_t uspeed;
    void setCmd(EMotorCmd cmd, uint32_t speed)
    {
        ecmd = cmd;
        uspeed = speed;
    }
};
struct stMotorStatus
{
    EMotorCmd status;
    uint32_t speed;
    void setStatus(EMotorCmd sta, uint32_t sp)
    {
        status = sta;
        speed = sp;
    }
};
struct stBaseCmd
{
    EspCmdId eid;
    uint8_t dataLen;
    union mm
    {
        stMotorCmd motorCmd;
        stMotorStatus motorStatus;
        uint8_t buf[128];
    }udata;
    void setData(EspCmdId id, void* data, uint8_t dataLen)
    {
        if(data && dataLen > 0)
        {
            this->dataLen = dataLen;
            memcpy(&udata, data, dataLen);
        }
        eid = id;
    }
};
