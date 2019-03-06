#include <stdlib.h>
#include <stdio.h>
#include "crc.h"

unsigned short CRC16 (unsigned char* pchMsg, unsigned int wDataLen)
{
	unsigned short wCRC = 0xFFFF;
	unsigned int i = 0;
	unsigned char chChar = 0;
	for (i = 0; i < wDataLen; i++)
	{
		chChar = pchMsg[i];
		wCRC = wCRCTalbeAbs[(chChar ^ wCRC) & 15] ^ (wCRC >> 4);
		wCRC = wCRCTalbeAbs[((chChar >> 4) ^ wCRC) & 15] ^ (wCRC >> 4);
	}
	return wCRC;
}

int crc_func(char* send_rfid, int len)
{
	unsigned short i, an;
	int low,high;
	
	unsigned char data[100];
	// 如果长度不够，就直接返回失败
	if(len < 3)
	{
		return 1;
	}

	for(i = 0; i < len - 2; i++)
	{
		data[i] = send_rfid[i];
	}
	an = CRC16(data, len - 2);
	
	low = (short)(an & 0xff); 
	high = (short)(an>>8);
	//printf("gao wei shi : %02x    ,di wei shi : %02x\n",high, low);
	//printf("crc1 : %02x    ,crc2 : %02x\n",send_rfid[len - 2], send_rfid[len - 1]);
	
	if((high == send_rfid[len - 2]) && (low == send_rfid[len - 1]))
	{	
		return 0;
	}else
	{
		return 1;
	}
}



