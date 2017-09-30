#include "inofmanaging.h"

/**
 * @brief init_update_cmd  初始化升级命令
 * @param addr
 * @param cmd
 */
void init_update_cmd(int addr,updateCmd *cmd)
{
    cmd->header = 0x7B;
    cmd->stx = 0xA5;
    cmd->addr = addr;
    cmd->len = 0x10;

    memset(cmd->reserved,0,sizeof(cmd->reserved));

    cmd->XOR = 0;
    cmd->XOR ^=cmd->header;
    cmd->XOR ^=cmd->stx;
    cmd->XOR ^=cmd->addr;
    cmd->XOR ^=cmd->len;

    //    cmd->XOR ^=cmd->reserved;
}

/*
 * addr 执行板地址 array 数据打包后
 * 该函数用于将需要发送的远程升级命令打包
 */
void send_to_packet(uchar addr,QByteArray &array)
{
    updateCmd cmd;

    init_update_cmd(addr,&cmd);

    array.append(cmd.header);
    array.append(cmd.stx);
    array.append(cmd.addr);
    array.append(cmd.len);

    for(int i = 0 ; i < sizeof(cmd.reserved);i++)
        array.append(cmd.reserved[i]);

    array.append(cmd.XOR);
}

void init_text_cmd(uchar addr ,char *data,textSendCmd *cmd)
{
    cmd->header = 0x7B;
    cmd->addr = addr;
    cmd->len = TEXT_MAX_LEN;
    memcpy(cmd->data,data,TEXT_MAX_LEN);
}

/**
 * @brief text_send_packet 每1024字节为一包，发送升级文件
 * @param data 一包（升级文件）
 * @param array 调用串口发送时的数据包
 */
void text_send_packet(uchar addr ,char *data,QByteArray &array)
{
    textSendCmd textCmd;

    init_text_cmd(addr,data ,&textCmd);

    array.append(textCmd.header);
    array.append(textCmd.addr);
    array.append((textCmd.len >> 8));
    array.append((textCmd.len & 0xFF));

    for(int i = 0 ; i < TEXT_MAX_LEN ; i++)
        array.append(textCmd.data[i]);

    textCmd.XOR = CRC16_2(array.data(), array.size());
    array.append(textCmd.XOR);
}




