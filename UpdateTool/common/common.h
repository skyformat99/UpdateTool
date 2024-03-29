﻿#ifndef COMMON_H
#define COMMON_H

#include <qglobal.h>
#include <QDebug>
#include <QString>

#define TEXT_MAX_LEN 1024

typedef struct
{
    uchar header;
    uchar stx;
    uchar addr;
    uchar len;
    uchar reserved[11];
    uchar XOR;  //异或校验
}updateCmd;

typedef struct
{
    uchar header;
    uchar addr;
    quint16 len;//数据包长度，默认为1024
    char data[1024];
    uchar XOR;  //crc校验
}textSendCmd;

extern int CRC16_2(char* pchMsg, char wDataLen);//CRC校验


#endif
