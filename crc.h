#ifndef __CRC_H
#define __CRC_H

const unsigned short wCRCTalbeAbs[] = {
0x0000, 0xCC01, 0xD801, 0x1400,
0xF001, 0x3C00, 0x2800, 0xE401,
0xA001, 0x6C00, 0x7800, 0xB401,
0x5000, 0x9C01, 0x8801, 0x4400,
};


int crc_func(char* send_rfid, int len);
unsigned short CRC16 (unsigned char* pchMsg, unsigned int wDataLen);

#endif

