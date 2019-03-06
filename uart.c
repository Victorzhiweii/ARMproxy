#include     <stdio.h> 
#include     <stdlib.h>
#include     <string.h>
#include     <unistd.h>
#include     <sys/types.h>
#include     <sys/stat.h>  
#include     <fcntl.h>  
#include     <termios.h>
#include     <errno.h> 
#include     <pthread.h> 
#include     <sys/ioctl.h> 
#include     "uart.h"
#include 	"common.h" 
#include 	 "profile.h"
#include 	 "cJSON.h"

//                    info->Callback(info->Name, "ERR:SENDFAIL#", strlen("ERR:SENDFAIL#"));
//						strcpy(revbuf, "ERR:HEADER#");
//						strcpy(revbuf, "ERR:CRC#");
//						strcpy(revbuf, "ERR:RESP#");
void bellthread(void)
{
	am335x_gpio_arg arg;
	int fd = -1;

	fd = open("/dev/am335x_gpio", O_RDWR);

	arg.pin = GPIO_TO_PIN(1, 18);
	
	arg.data = 1;
	ioctl(fd, IOCTL_GPIO_SETOUTPUT, &arg);
	usleep(500*1000);
	arg.data = 0;
	ioctl(fd, IOCTL_GPIO_SETOUTPUT, &arg);
	close(fd);	
	//pthread_exit(0);
}

void set_speed(int fd, int speed)
{
	int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800,B2400, B1200};
	int name_arr[] = {115200, 57600, 38400,  19200,  9600,  4800,  2400, 1200};
	
	int   status;
	int i;
    struct termios   Opt;
    tcgetattr(fd, &Opt);
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
		if (speed == name_arr[i])
		{
			tcflush(fd, TCIOFLUSH);
			cfsetispeed(&Opt, speed_arr[i]);
			cfsetospeed(&Opt, speed_arr[i]);
			status = tcsetattr(fd, TCSANOW, &Opt);
			if (status != 0)
				perror("tcsetattr fd1");

			tcflush(fd,TCIOFLUSH);

			break;
     	}
    }
}

int set_Parity(int fd,int databits,int stopbits,int parity)
{
    struct termios options;
    if  ( tcgetattr( fd,&options)  !=  0)
    {
		perror("SetupSerial 1");
		return(FALSE);
    }
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
		case 7:
			options.c_cflag |= CS7;
			break;
		case 8:
			options.c_cflag |= CS8;
			break;
		default:
			fprintf(stderr,"Unsupported data size\n");
			return (FALSE);
    }
    switch (parity)
    {
		case 'n':
		case 'N':
			options.c_cflag &= ~PARENB;   
			options.c_iflag &= ~INPCK;   
			break;
		case 'o':
		case 'O':
			options.c_cflag |= (PARODD | PARENB); 
			options.c_iflag |= INPCK;           
			break;
		case 'e':
		case 'E':
			options.c_cflag |= PARENB;     
			options.c_cflag &= ~PARODD;
			options.c_iflag |= INPCK;     
			break;
		case 'S':
		case 's':  
			options.c_cflag &= ~PARENB;
			options.c_cflag &= ~CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported parity\n");
			return (FALSE);
	}
	switch (stopbits)
	{
		case 1:
			options.c_cflag &= ~CSTOPB;
			break;
		case 2:
			options.c_cflag |= CSTOPB;
			break;
		default:
			fprintf(stderr,"Unsupported stop bits\n");
			return (FALSE);
    }


	options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL|IXON);
	options.c_oflag &= ~OPOST;
	options.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

    /* Set input parity option */
    
    if (parity != 'n')
    {
		options.c_iflag |= INPCK;
	}
	
    options.c_cc[VTIME] = 3; // 0.3 seconds
    options.c_cc[VMIN] = 0;
    
	
	tcflush(fd,TCIFLUSH); /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)
    {
		perror("SetupSerial 3");
		return (FALSE);
    }
    return (TRUE);
}

int uart_init(char *p, int speed)
{		
	int fd;
	fd = open(p, O_RDWR|O_NOCTTY);
	set_speed(fd,speed); //设置串口波特率	
	set_Parity(fd,8,1,'N'); //设置8位数据位，1位停止位，无校验等其他设置。
	
	return fd;	
}   	 


static int init_rfid(int fd)
{
	int i, crc;
	char revbuf[LENGTH];
	int len = write(fd, send_rfid_cmd_manual_mode, sizeof(send_rfid_cmd_manual_mode));			//模式设为交互模式
	printf("init_rfid(1), set mode result:%d \n", len);

	usleep(1000 * 300);	
	len = read(fd, revbuf, sizeof(revbuf));
	
	printf("init_rfid(4), read change mode cmd response data size:%d \n", len);
	
	if(len > 0) 		//排除未收到数据的可能
 	{
		crc = crc_func(revbuf, len);
		if(crc != 0)											//crc校验，确保数据完整								
		{
			printf ("init_rfid(7) RFID response crc failed numbytes = %d\n", len); 
			for(i = 0; i < len; i++)
			{
				printf("%02x ", revbuf[i]);
			}
			printf ("\n");
			return crc;
		}
 	}
	else
	{
		return 1;
	}

 	len = write(fd, send_rfid_cmd_store, sizeof(send_rfid_cmd_store));			//保存模式设置
	printf("init_rfid(6), write data size:%d \n", len);
	usleep(1000 * 200);	
	len = read(fd, revbuf, sizeof(revbuf));
	
	printf("init_rfid(7), read store cmd response data size:%d \n", len);
	if(len > 0) 		//排除未收到数据的可能
	{
		crc = crc_func(revbuf, len);
		if(crc != 0)											//crc校验，确保数据完整								
		{
			printf ("init_rfid(7) RFID response failed numbytes = %d\n", len); 
			for(i = 0; i < len; i++)
			{
				printf("%02x ", revbuf[i]);
			}
			printf ("\n");

			return crc;
		}
	}
	else
	{
		return 1;
	}
	
	return 0;
}

// void dumpbufferinfo(char* buffer, int len)
// {
// 	int i;
// 	for(i = 0; i < len; i++)
// 	{
// 		if(buffer[i] >= 32 && buffer[i] < 127)
// 		{
// 			printf("%c ", buffer[i]);
// 		}
// 		else if(buffer[i] == '\0')
// 		{
// 			break;
// 		}
// 		else
// 		{
// 			printf("0x%02X ", buffer[i]);
// 		}
// 	}

// 	printf ("\n");
// }

#if 1
static int prepare_filter_recv_data_rfid(char* name, char* revbuf, int* len, int* read_offset)
{
	if(revbuf[0] != 0xFF) 
	{
		printf("rfidThread(10-%s), read invalid header :%d , content: \n", name, *len);
		#if 1 
		dumpbufferinfo(revbuf, *len);
		#endif
	}
	else
	{
		if(*read_offset == *len)
		{

		}
		else if(*len < 3 || ((unsigned char)revbuf[1]) > ((unsigned char)(*len - 3)))
		{ // 数据没有读取完成
			
			printf("rfidThread(12-%s), not read completed : %d < totlen:%d, read_offset:%d  \n", 
					name, *len, revbuf[1] + 3, *read_offset);
			
			*read_offset = *len;

			usleep(1000 * 100);
			return -1;
		}
	}
	
	*read_offset = 0;

	// rfid包头不对
	if(revbuf[0] != 0xFF) 
	{
		printf("rfidThread(13-%s), read invalid header data :%d, content:  \n", name, *len);
		#if 1
			dumpbufferinfo(revbuf, *len);
		#endif
		// 包头不正确
		strcpy(revbuf, "ERR:HEADER#");
		*len = strlen(revbuf);
	}
	else
	{
		int crc = crc_func(revbuf, *len);
		if(crc != 0)
		{
			printf("rfidThread(14-%s), rfid data crc :%d error, read data numbytes:%d, content: \n", name, crc, *len);
			#if 1
				dumpbufferinfo(revbuf, *len);
			#endif

			// 数据校验失败
			strcpy(revbuf, "ERR:CRC#");
			*len = strlen(revbuf);
		}
		else if(revbuf[5] != 0)
		{
			printf("prepare_filter_recv_data_rfid(15-%s), read data faild numbytes :%d, status:0x%x, content:\n", name, *len, revbuf[5]);
		#if 1
			dumpbufferinfo(revbuf, *len);
		#endif
			// 返回错误状态码
			strcpy(revbuf, "ERR:RESP#");
			*len = strlen(revbuf);
		}
	}

	if(*len >= 10 && memcpy(revbuf, "ERR:", strlen("ERR:")) != 0)
	{
		// #if 1
		// dumpbufferinfo(revbuf,  len);
		// #endif
		int buff_len = *len;
		char* ptr = revbuf + 6;
		buff_len -= 8;
		// 清除尾部的空格
		while(buff_len > 0)
		{
			char d = ptr[buff_len-1];
			if(d == '\0' || d == '\r' || d == '\n' || d == ' ')
			{
				buff_len--;
				printf("prepare_filter_recv_data_rfid(17-%s), remove invalid char pos : %d , value: 0x%02X \n", 
					name, buff_len, d);
			}
			else
			{
				//printf("rfidThread(12-%s), len : %d , value: 0x%02X \n", info->name, len, d);
				break;
			}
		}
		// 判断是否有非法字符
		int i = 0;
		for(i=0; i < buff_len; i++) 
		{
			unsigned char d = ptr[i];
			if(d >= 32 && d <= 126) 
			{
				continue;
			}
			else
			{
				printf("prepare_filter_recv_data_rfid(18-%s), index:%d found invalid char : 0x%02X \n", name, buff_len, d);
				buff_len = i;
				break;
			}
		}

		*len = buff_len;

		// 头部偏移量
		return 6;
	}
#else
static int prepare_filter_recv_data_rfid(char* name, char* revbuf, int &len, int &read_offset)
{
	if(revbuf[0] != 0xFF) 
	{
		printf("rfidThread(10-%s), read invalid header :%d , content: \n", name, len);
		#if 1 
		dumpbufferinfo(revbuf, len);
		#endif
	}
	else
	{
		if(read_offset == len)
		{

		}
		else if(len < 3 || ((unsigned char)revbuf[1]) > ((unsigned char)(len - 3)))
		{ // 数据没有读取完成
			
			printf("rfidThread(12-%s), not read completed : %d < totlen:%d, read_offset:%d  \n", 
					name, len, revbuf[1] + 3, read_offset);
			
			read_offset = len;

			usleep(1000 * 100);
			return -1;
		}
	}
	
	read_offset = 0;

	// rfid包头不对
	if(revbuf[0] != 0xFF) 
	{
		printf("rfidThread(13-%s), read invalid header data :%d  \n", name, len);
		#if 1
			printf ("rfidThread(13-%s), read data numbytes = %d, content: \n", name, len); 
			dumpbufferinfo(revbuf, len);
		#endif
		// 包头不正确
		strcpy(revbuf, "ERR:HEADER#");
		len = strlen(revbuf);
	}
	else
	{
		crc = crc_func(revbuf, len);
		if(crc != 0)
		{
			printf("rfidThread(14-%s), rfid data crc :%d error, read data numbytes:%d, content: \n", name, crc, len);
			#if 1
				dumpbufferinfo(revbuf, len);
			#endif

			// 数据校验失败
			strcpy(revbuf, "ERR:CRC#");
			len = strlen(revbuf);
		}
		else if(revbuf[5] != 0)
		{
			printf("prepare_filter_recv_data_rfid(15-%s), read data faild numbytes :%d, status:0x%x, content:\n", name, len, revbuf[5]);
		#if 1
			dumpbufferinfo(revbuf, len);
		#endif
			// 返回错误状态码
			strcpy(revbuf, "ERR:RESP#");
			len = strlen(revbuf);
		}
	}

	if(len >= 10 && memcpy(revbuf, "ERR:", strlen("ERR:")) != 0)
	{
		// #if 1
		// dumpbufferinfo(revbuf,  len);
		// #endif
		char* ptr = revbuf + 6;
		len -= 8;
		// 清除尾部的空格
		while(len > 0)
		{
			char d = ptr[len-1];
			if(d == '\0' || d == '\r' || d == '\n' || d == ' ')
			{
				len--;
				printf("prepare_filter_recv_data_rfid(17-%s), remove invalid char pos : %d , value: 0x%02X \n", 
					name, len, d);
			}
			else
			{
				//printf("rfidThread(12-%s), len : %d , value: 0x%02X \n", name, len, d);
				break;
			}
		}
		// 判断是否有非法字符
		int i = 0;
		for(i=0; i < len; i++) 
		{
			unsigned char d = ptr[i];
			if(d >= 32 && d <= 126) 
			{
				continue;
			}
			else
			{
				printf("prepare_filter_recv_data_rfid(18-%s), index:%d found invalid char : 0x%02X \n", name, len, d);
				len = i;
				break;
			}
		}

		// 头部偏移量
		return 6;
	}
#endif
	return 0;
}


void SerialPortThread(void * arg)
{
	SerialPortInfo* sp_info = (SerialPortInfo*)arg;
	CommunicationBaseInfo* info = &(sp_info->Base);
	
	int len, read_offset = 0;
	char revbuf[300];
  
	
	printf("SPThread(0000)\n");
	info->ExitFlag = 0;
	info->Connected = 0;
	// memset(info->buf, 0, sizeof(info->buf));
    // info->len = 0;

	printf("SPThread(0-%s), entry uart thread, %s, %d\n", info->Name, sp_info->Dev, sp_info->Speed);

	info->fd = uart_init(sp_info->Dev, sp_info->Speed);
	if(info->fd == -1){
		printf("SPThread(1-%s), init uart:%d for %s\n", info->Name, errno, sp_info->Dev);
        return;
	}

	printf("SPThread(2-%s), init uart:%d success\n", info->Name, info->fd);

	//fcntl(info->fd, F_SETFL, FNDELAY); //非阻塞
		
	if(strcmp(info->Name, "SGRFID") == 0)
	{
		len = init_rfid(info->fd);
		if(len != 0)
		{
			printf("SPThread(3-%s), init rfid failed : %d\n", info->Name, len);
			return;
		}
	}
	
	info->Connected = 1;
	//fcntl(info->fd, F_SETFL, 0); //阻塞

	while(info->ExitFlag == 0)
  	{
  		//printf("rfidThread(10-%s), read data size :%d \n", info->name, len);

		// struct  timeval start;
        // struct  timeval end;
        // unsigned  long diff;
        // gettimeofday(&start,NULL);
		
		len = read_offset + read(info->fd, revbuf + read_offset, sizeof(revbuf) - read_offset);

		// gettimeofday(&end,NULL);
		
		// diff = 1000 * (end.tv_sec-start.tv_sec)+ (end.tv_usec-start.tv_usec) / 1000;
		
        // printf("thedifference is %ld\n", diff);
		
		//printf("rfidThread(12-%s), read data size :%d \n", info->name, len);
		
		if(len > 0) //接收数据
		{
			int header_offset = 0;
			if(strcmp(info->Name, "SGRFID") == 0)
			{
				header_offset = prepare_filter_recv_data_rfid(info->Name, revbuf, &len, &read_offset);
			}
			
			if(header_offset < 0) // 数据还没有收完全，
			{
				continue;
			}

			if(len > 0 && info->ProcessRecv != NULL)
            {
                info->ProcessRecv(info, revbuf + header_offset, len);
            }
			else if(len == 0)
			{
				printf("SPThread(13-%s), no valid data recv\n", info->Name);
			}
            else 
            {
                printf("SPThread(13-%s), process data handler is null.\n", info->Name);
            }
		}

		read_offset = 0;

		
		// 检查是否有需要发送的内容，如果有就发送出去
        if(info->ProcessSend != NULL)
        {
            if(info->ProcessSend(info) > 0 )
            {
                usleep(1000 * 100);
            }
        }
        else
        {
            printf("tcpThread(23-%s), no send process handler \n", info->Name);
        }
		
		usleep(1000 * 20);
   } 

	close(info->fd);
	info->fd = -1;
	info->handler = 0;
}
