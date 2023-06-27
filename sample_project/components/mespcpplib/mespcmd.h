#pragma once
#include <iostream>
#include <cstring>
using namespace std;

enum EspCmdId
{
    E_ESP_CMD_ID_INVALID,
    E_ESP_CMD_ID_CONNECT,
    E_ESP_CMD_ID_IS_DEV_ONLINE,
    E_ESP_CMD_ID_DEV_ONLINE,
    E_ESP_CMD_ID_SEND_STR,
    E_ESP_CMD_ID_CHNNEL_DATA,
    E_ESP_CMD_ID_MAX
};

#pragma pack (1)
#define CH_NUM_MAX (18)
struct stEspChannel
{
    uint16_t ch0 : 11;
    uint16_t ch1 : 11;
    uint16_t ch2 : 11;
    uint16_t ch3 : 11;
    uint16_t ch4 : 11;
    uint16_t ch5 : 11;
    uint16_t ch6 : 11;
    uint16_t ch7 : 11;
    uint16_t ch8 : 11;
    uint16_t ch9 : 11;
    uint16_t ch10 : 11;
    uint16_t ch11 : 11;
    uint16_t ch12 : 11;
    uint16_t ch13 : 11;
    uint16_t ch14 : 11;
    uint16_t ch15 : 11;
    uint16_t ch16 : 11;
    uint16_t ch17 : 11;
    void packedChData(const uint16_t* data, int len)
    {
        if(data == nullptr || len < 18)
        {
            return;
        }
            ch0 = data[0];
            ch1 = data[1];
            ch2 = data[2];
            ch3 = data[3];
            ch4 = data[4];
            ch5 = data[5];
            ch6 = data[6];
            ch7 = data[7];
            ch8 = data[8];
            ch9 = data[9];
            ch10 = data[10];
            ch11 = data[11];
            ch12 = data[12];
            ch13 = data[13];
            ch14 = data[14];
            ch15 = data[15];
            ch16 = data[16];
            ch17 = data[17];
    }
};
#pragma pack ()

struct stBaseCmd
{
    EspCmdId eid;
    uint8_t dataLen;
    union mm
    {
        stEspChannel chdata;
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
