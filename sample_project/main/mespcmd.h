#pragma once
#include <iostream>
#include <cstring>
using namespace std;

enum EspCmdId
{
    E_ESP_CMD_ID_INVALID,
    E_ESP_CMD_ID_MOTOR_CTRL,

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
    stMotorCmd()
    {

    }
    stMotorCmd(EMotorCmd cmd, uint32_t speed)
    {
        ecmd = cmd;
        uspeed = speed;
    }
};
struct stBaseCmd
{
    EspCmdId eid;
    uint8_t dataLen;
    union mm
    {
        stMotorCmd motorCmd;
        mm(){}
    }udata;
    stBaseCmd()
    {

    }
    stBaseCmd(EspCmdId id, void* data, uint8_t dataLen)
    {
        if(data && dataLen > 0)
        {
            memcpy(&udata, data, dataLen);
        }
        eid = id;
    }
};
