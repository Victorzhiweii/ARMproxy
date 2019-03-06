#include <stdlib.h>
#include <stdio.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/wait.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <pthread.h> 
#include <arpa/inet.h> 
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include "tcpserver.h"
#include "uart.h" 
#include <dirent.h>
#include <sys/file.h>
#include <errno.h>
#include "cJSON.h"
#include <sys/time.h>
#include <time.h>

int flag_test;
char test_buf[LENGTH];//载具码
char test_buf1[LENGTH];//粹盘码



char send_rfid1[] = {0xFF,	0x0b,  0x1C, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x35,0xEA }; //模式设为交互模式 
char send_rfid2[] = {0xFF,	0x07,  0x11,0x00, 0x00,0x00, 0x00,0x3C,0x9F,0x7A }; //读60个标签数据 
char send_rfid3[] = {0xFF, 0x04, 0x16, 0x00, 0x00, 0xE0, 0xD0}; //保存模式设置





void tcp_MAChread1(void *arg)
{
	parameter_mode parameter_mode1;
	parameter_mode1 = *(parameter_mode *)arg;
	int sum;
	char revbuf[LENGTH];
	int i = 0;
	while(1)
	{
		sum=recv(parameter_mode1.socked_MAC1,revbuf,LENGTH,0);
		if(sum==-1) 
		{ 
			printf("ERROR:Fail to get string\n"); 
			continue;
		}
		if(sum>0)
		{
			printf ("OK: Receviced numbytes = %d\n", sum); 		
			for(i = 0; i < sum; i++)
			{
				printf ("OK: Receviced numbytes = %d\n", revbuf[i]); 	
			}
			if(revbuf[sum - 3] == 'S')
			{
				send(parameter_mode1.socked_test1,"01:PASS",strlen("01:PASS"),0);
			}
			if(revbuf[sum - 3] == 'L')
			{
				send(parameter_mode1.socked_test1,"01:FAIL",strlen("01:FAIL"),0);
			}
		}
	}
}

void tcp_MAChread2(void *arg)
{
	parameter_mode parameter_mode1;
	parameter_mode1 = *(parameter_mode *)arg;
	int sum;
	char revbuf[LENGTH];
	while(1)
	{
		sum=recv(parameter_mode1.socked_MAC2,revbuf,LENGTH,0);
		if(sum==-1) 
		{ 
			printf("ERROR:Fail to get string\n"); 
			continue;
		}
		if(sum>0)
		{
			printf ("OK: Receviced numbytes = %d\n", sum);		
			printf ("OK: Receviced string is: %s\n", revbuf);
			if(revbuf[sum - 6] == 'P')
			{
				send(parameter_mode1.socked_test1,"02:PASS",strlen("02:PASS"),0);
			}
			if(revbuf[sum - 6] == 'F')
			{
				send(parameter_mode1.socked_test1,"02:FAIL",strlen("02:FAIL"),0);
			}
		}

	}
}

void tcp_MAChread3(void *arg)
{
	parameter_mode parameter_mode1;
	parameter_mode1 = *(parameter_mode *)arg;

	int sum;
	char revbuf[LENGTH];
	while(1)
	{
		sum=recv(parameter_mode1.socked_MAC3,revbuf,LENGTH,0);
		if(sum==-1) 
		{ 
			printf("ERROR:Fail to get string\n"); 
			continue;
		}
		if(sum>0)
		{
			printf ("OK: Receviced numbytes = %d\n", sum);		
			printf ("OK: Receviced string is: %s\n", revbuf);
			if(revbuf[sum - 6] == 'P')
			{
				send(parameter_mode1.socked_test1,"03:PASS",strlen("03:PASS"),0);
			}
			if(revbuf[sum - 6] == 'F')
			{
				send(parameter_mode1.socked_test1,"03:FAIL",strlen("03:FAIL"),0);
			}
		}

	}

	
}


void tcp_testhread1(void *arg)
{
	parameter_mode parameter_mode1;
	parameter_mode1 = *(parameter_mode *)arg;
	int socked;
	uartmode *uart = (uartmode *)malloc(sizeof(uartmode)*10);

	FILE *f;
	long len;
	char *content;
	cJSON *json;
	cJSON *pSub;
	cJSON *format;
	cJSON *parameter;
	char *key;
	char *key1;
	int parameter_size;
	int format_size;

	int sum = 0;
	int i = 0, z = 0;
	char revbuf[LENGTH]; // Receive buffer 
	
	char buf_MAC1[LENGTH];

	uart[3].name = "/dev/ttyO3";
	uart[4].name = "/dev/ttyO4";
	uart[5].name = "/dev/user_uart"; 

	f=fopen("./test.json","rb");
	fseek(f,0,SEEK_END);
	len=ftell(f);
	fseek(f,0,SEEK_SET);
	content=(char*)malloc(len+1);
	fread(content,1,len,f);
	fclose(f);

	
	json=cJSON_Parse(content);
	
	parameter_size = cJSON_GetArraySize(json);
	
	socked = parameter_mode1.socked_test1;

	
	for (i = 0; i < parameter_size; i++)
	{

		pSub = cJSON_GetArrayItem(json,i);	
		key = pSub->string;
		format = NULL;

		if(strcmp(key,"/dev/ttyO3") == 0)
		{	

			format = cJSON_GetObjectItem(json,"/dev/ttyO3");
			format_size = cJSON_GetArraySize(format);
			for(z = 0; z < format_size; z++)
			{
				parameter = cJSON_GetArrayItem(format,z);	
				key1 = parameter->string;
				if(strcmp(key1,"speed") == 0)
				{
					uart[3].speed = cJSON_GetObjectItem(format,"speed")->valueint;				
				}				
			}
		}

		if(strcmp(key,"/dev/ttyO4") == 0)
		{	

			format = cJSON_GetObjectItem(json,"/dev/ttyO4");
			format_size = cJSON_GetArraySize(format);
			for(z = 0; z < format_size; z++)
			{
				parameter = cJSON_GetArrayItem(format,z);	
				key1 = parameter->string;
				if(strcmp(key1,"speed") == 0)
				{

					uart[4].speed = cJSON_GetObjectItem(format,"speed")->valueint;
				
				}				
			}
		}

		if(strcmp(key,"/dev/user_uart") == 0)
		{	

			format = cJSON_GetObjectItem(json,"/dev/user_uart");
			format_size = cJSON_GetArraySize(format);
			for(z = 0; z < format_size; z++)
			{
				parameter = cJSON_GetArrayItem(format,z);	
				key1 = parameter->string;
				if(strcmp(key1,"speed") == 0)
				{
					uart[5].speed = cJSON_GetObjectItem(format,"speed")->valueint;
				}				
			}
		}
	}

	cJSON_Delete(json);

	uart[3].fd = uart_init(uart[3].name, uart[3].speed);
	uart[4].fd = uart_init(uart[4].name, uart[4].speed);
	if(parameter_mode1.mode == 1)
	{
		uart[5].fd = uart_init(uart[5].name, uart[5].speed);
	}

	while(1)
	{
		memset(revbuf, 0, 200);
		
		sum=recv(socked ,revbuf,LENGTH,0);
		if(sum<= 0) 
		{  
			if(errno == EINTR)
			{
				printf("ERROR:recv is errno\n"); 
				continue;
			}else
			{
				printf("ERROR:Fail to get string\n"); 
				break;
			}
		}
		else
		{
			printf ("OK: TCP numbytes = %d\n", sum);		
			printf ("\nOK: TCP string %s\n", revbuf);

			buf_MAC1[0] = 'C';
			buf_MAC1[1] = 'A';
			buf_MAC1[2] = 'S';
			buf_MAC1[3] = 'N';
			buf_MAC1[4] = ':';
			for(i = 0; i < 10; i++)
			{
				buf_MAC1[5 + i] = test_buf[i];
			}
			buf_MAC1[15] = ';';
			buf_MAC1[16] = 'T';
			buf_MAC1[17] = 'R';
			buf_MAC1[18] = 'S';
			buf_MAC1[19] = 'N';
			buf_MAC1[20] = ':';
			for(i = 0; i < 11; i++)
			{
				buf_MAC1[21 + i] = test_buf[i + 11];
			}
			buf_MAC1[32] = '\r';
			buf_MAC1[33] = '\n';

			if(strcmp(revbuf, "test01") == 0)							//把缓存放入测试机1中
			{
				
					/*write(uart[3].fd, "\n", 1);
					usleep(500*1000);
					
					test_buf[10] = 13;
					write(uart[3].fd, test_buf, 11);
					usleep(500*1000);
					
					test_buf1[11] = 13;
					write(uart[3].fd, test_buf1, 12);*/

					
					/*for(i = 0; i < 11; i++)
					{
						printf("%02x ", test_buf1[i]);
					}
					printf ("\nOK: RFID mode string \n");*/
					
					send(parameter_mode1.socked_MAC1,buf_MAC1,34,0);								
			}

			if(strcmp(revbuf, "test02") == 0)							//把缓存放入测试机2中
			{
					/*write(uart[4].fd, "\n", 1);
					usleep(500*1000);
					
					test_buf[10] = 13;
					write(uart[4].fd, test_buf,  11);
					usleep(500*1000);
					
					test_buf1[11] = 13;
					write(uart[4].fd, test_buf1,  12);*/
					
					send(parameter_mode1.socked_MAC2,buf_MAC1,34,0);
							
			}

			if(strcmp(revbuf, "test03") == 0)							//把缓存放入测试机3中
			{
					/*printf("\n fd is %d\n", uart[5].fd);
					
					write(uart[5].fd, "\n", 1);
					usleep(500*1000);
					
					printf("\n name is %s\n", uart[5].name);
					
					test_buf[10] = 13;
					write(uart[5].fd, test_buf, 11);
					usleep(500*1000);

					printf("\n speed is %d\n", uart[5].speed);
					
					test_buf1[11] = 13;
					write(uart[5].fd, test_buf1,  12);*/
					
					send(parameter_mode1.socked_MAC3,buf_MAC1,34,0);
			}
		}
	}

	printf("pthread_exit is open\n");
	pthread_exit(0); 
}

void tcp1thread(void *arg)
{
	parameter_mode parameter_mode1;
	parameter_mode1 = *(parameter_mode *)arg;
	int socked;
	
	int sum = 0;
	char revbuf[LENGTH]; // Receive buffer 
	char product[LENGTH];
	char sdbuf[LENGTH];
	char sdbuf1[LENGTH];
	char testbuf[LENGTH];

	FILE *f;
	long len;
	char *content;
	cJSON *json;
	cJSON *pSub;
	cJSON *format;
	cJSON *parameter;
	char *key;
	char *key1;
	int parameter_size;
	int format_size;
	
	int i = 0;
	int z = 0;
	int crc = 0;
	int flag1 = 0;

	uartmode *uart = (uartmode *)malloc(sizeof(uartmode)*10);
	
	uart[1].name = "/dev/ttyO1";

	f=fopen("./test.json","rb");
	fseek(f,0,SEEK_END);
	len=ftell(f);
	fseek(f,0,SEEK_SET);
	content=(char*)malloc(len+1);
	fread(content,1,len,f);
	fclose(f);
	pthread_t tcp_test1;
	pthread_t tcp_MAC1;
	pthread_t tcp_MAC2;
	pthread_t tcp_MAC3;
	
	 json=cJSON_Parse(content);

	parameter_size = cJSON_GetArraySize(json);
	

	socked = parameter_mode1.socked;
	pthread_create(&tcp_test1,NULL,(void*)tcp_testhread1,&parameter_mode1);
	pthread_create(&tcp_MAC1,NULL,(void*)tcp_MAChread1,&parameter_mode1);
	pthread_create(&tcp_MAC2,NULL,(void*)tcp_MAChread2,&parameter_mode1);
	pthread_create(&tcp_MAC3,NULL,(void*)tcp_MAChread3,&parameter_mode1);
	
	for (i = 0; i < parameter_size; i++)
	{

		pSub = cJSON_GetArrayItem(json,i);	
		key = pSub->string;
		format = NULL;
		if(strcmp(key,"/dev/ttyO1") == 0)
		{	

			format = cJSON_GetObjectItem(json,"/dev/ttyO1");
			format_size = cJSON_GetArraySize(format);
			for(z = 0; z < format_size; z++)
			{
				parameter = cJSON_GetArrayItem(format,z);	
				key1 = parameter->string;
				if(strcmp(key1,"speed") == 0)
				{

					uart[1].speed = cJSON_GetObjectItem(format,"speed")->valueint;
				
				}				
			}
		}
	}

	cJSON_Delete(json);

	uart[1].fd = uart_init(uart[1].name, uart[1].speed);
			
	write(uart[1].fd, send_rfid1, sizeof(send_rfid1));			//模式设置为交互模式
	usleep(1000*200);
	if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
	{
		printf ("OK: RFID mode numbytes = %d\n", uart[1].len); 
		for(i = 0; i < uart[1].len; i++)
		{
			printf("%02x ", uart[1].buf[i]);
		}
		printf ("\nOK: RFID mode string \n");
		crc = crc_func(uart[1].buf, uart[1].len);
		if(crc != 0)											//crc校验，确保数据完整								
		{
			write(uart[1].fd, send_rfid1, sizeof(send_rfid1));			//模式设置为交互模式
			usleep(1000*200);
			if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
			{
				crc = crc_func(uart[1].buf, uart[1].len);
				if(crc != 0)
				{
					send(socked,"error1",strlen("error1"),0);
				}
			}
		}
	}
	else
	{
		write(uart[1].fd, send_rfid1, sizeof(send_rfid1));	
		usleep(1000*200);
		if((uart[1].len = read(uart[1].fd,uart[1].buf,200))<=0) 		//排除未收到数据的可能		
		{
			send(socked,"error1",strlen("error1"),0);
		}
	}

	write(uart[1].fd, send_rfid3, sizeof(send_rfid3));			//保存模式设置
	usleep(1000*200);
	if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0)		//排除未收到数据的可能
	{
		printf ("OK: RFID mode numbytes = %d\n", uart[1].len); 
		for(i = 0; i < uart[1].len; i++)
		{
			printf("%02x ", uart[1].buf[i]);
		}
		printf ("\nOK: RFID mode string \n");
		crc = crc_func(uart[1].buf, uart[1].len);
		if(crc != 0)											//crc校验，确保数据完整								
		{
			write(uart[1].fd, send_rfid3, sizeof(send_rfid3));			//模式设置为交互模式
			usleep(1000*200);
			if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
			{
				crc = crc_func(uart[1].buf, uart[1].len);
				if(crc != 0)
				{
					send(socked,"error1",strlen("error1"),0);
				}
			}
		}
	}
	else
	{
		write(uart[1].fd, send_rfid3, sizeof(send_rfid3));	
		usleep(1000*200);
		if((uart[1].len = read(uart[1].fd,uart[1].buf,200))<=0) 		//排除未收到数据的可能
		{
			send(socked,"error1",strlen("error1"),0);
		}
	}

	
	memset(sdbuf, 0, 200);
	
	
	while(1)
	{
		memset(revbuf, 0, 200);
		memset(uart[1].buf, 0, 200);
		//memset(test_buf, 0, 200);
		//memset(test_buf1, 0, 200);

		
		sum=recv(socked ,revbuf,LENGTH,0);
		if(sum<= 0) 
		{  
			if(errno == EINTR)
			{
				continue;
			}else
			{
				printf("ERROR:Fail to get string\n"); 
				break;
			}
		}		
		else
		{
			printf ("OK: TCP numbytes = %d\n", sum); 		
			printf ("\nOK: TCP string %s\n", revbuf);
			
			if(strcmp(revbuf, "read01") == 0)
			{				
				write(uart[1].fd, send_rfid2, sizeof(send_rfid2));			//读数据
				usleep(1000*200);
				uart[1].len = read(uart[1].fd,uart[1].buf,200);
				
				
				if(uart[1].len>0) 		//排除未收到数据的可能
				{				
					flag1 = 0;
					
					printf ("OK: RFID numbytes = %d\n", uart[1].len); 		
					for(i = 0; i < uart[1].len; i++)
					{
						printf("%02x ", uart[1].buf[i]);
					}
					printf ("\nOK: RFID mode string \n");
					crc = crc_func(uart[1].buf, uart[1].len);
					if(crc != 0)											//crc校验，确保数据完整	
					{
						write(uart[1].fd, send_rfid2, sizeof(send_rfid2));			//读数据
						usleep(1000*200);
						uart[1].len = read(uart[1].fd,uart[1].buf,200);
						crc = crc_func(uart[1].buf, uart[1].len);
						if(crc != 0)											//crc校验，确保数据完整	
						{
							send(socked,"error1",strlen("error1"),0);
						}
					}
					else
					{
						
						if(uart[1].buf[1] != 0x41)							//排除未读到码的可能性
						{
							send(socked,"error2",strlen("error2"),0);
						}
						else
						{
						
							for(i = 0; i < 22; i++)
							{
								product[i] = uart[1].buf[i + 6];
								test_buf[i] = uart[1].buf[i + 6];
							}

							send(socked,product,22,0);
							
						}
					}
				}
				else
				{
					write(uart[1].fd, send_rfid2, sizeof(send_rfid2));			//读数据
					usleep(1000*200);
					uart[1].len = read(uart[1].fd,uart[1].buf,200);
					if(uart[1].len<=0) 
					{
						send(socked,"error1",strlen("error1"),0);
					}					
				}
			}

			

			
			
		}
	}  
	
	printf("pthread_exit is open\n");
	pthread_exit(0); 	
}


void tcp2thread(void *arg)
{
	int socked;
	socked = *(int *)arg;
	int sum = 0;
	char revbuf[LENGTH]; // Receive buffer 
	char sdbuf2[LENGTH];
	char sdbuf1[LENGTH];

	int i = 0;
	int z = 0;
	int crc = 0;
	int flag1 = 0;
	int flag2 = 0;
	
	FILE *f;
	long len;
	char *content;
	cJSON *json;
	cJSON *pSub;
	cJSON *format;
	cJSON *parameter;
	char *key;
	char *key1;
	int parameter_size;
	int format_size;
	
	uartmode *uart = (uartmode *)malloc(sizeof(uartmode)*5);
	uart[1].name = "/dev/ttyO1";
	uart[2].name = "/dev/ttyO2";

	
	f=fopen("./test.json","rb");
	fseek(f,0,SEEK_END);
	len=ftell(f);
	fseek(f,0,SEEK_SET);
	content=(char*)malloc(len+1);
	fread(content,1,len,f);
	fclose(f);

	 json=cJSON_Parse(content);

	parameter_size = cJSON_GetArraySize(json);

	for (i = 0; i < parameter_size; i++)
	{

		pSub = cJSON_GetArrayItem(json,i);	
		key = pSub->string;
		format = NULL;
		if(strcmp(key,"/dev/ttyO1") == 0)
		{	

			format = cJSON_GetObjectItem(json,"/dev/ttyO1");
			format_size = cJSON_GetArraySize(format);
			for(z = 0; z < format_size; z++)
			{
				parameter = cJSON_GetArrayItem(format,z);	
				key1 = parameter->string;
				if(strcmp(key1,"speed") == 0)
				{

					uart[1].speed = cJSON_GetObjectItem(format,"speed")->valueint;
				
				}				
			}
		}

	}
	cJSON_Delete(json);

	uart[1].fd = uart_init(uart[1].name, uart[1].speed);	
		

	write(uart[1].fd, send_rfid1, sizeof(send_rfid1));			//RFID1模式设置为交互模式	
	usleep(1000*200);
	if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
	{
		printf ("OK: RFID mode numbytes = %d\n", uart[1].len); 
		for(i = 0; i < uart[1].len; i++)
		{
			printf("%02x ", uart[1].buf[i]);
		}
		printf ("\nOK: RFID mode string \n");
		crc = crc_func(uart[1].buf, uart[1].len);
		if(crc != 0)											//crc校验，确保数据完整								
		{
			write(uart[1].fd, send_rfid1, sizeof(send_rfid1));			//模式设置为交互模式
			usleep(1000*200);
			if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
			{
				crc = crc_func(uart[1].buf, uart[1].len);
				if(crc != 0)
				{
					send(socked,"error1",strlen("error1"),0);
				}
			}
		}
	}
	else
	{
		write(uart[1].fd, send_rfid1, sizeof(send_rfid1));
		usleep(1000*200);
		if((uart[1].len = read(uart[1].fd,uart[1].buf,200))<=0) 		//排除未收到数据的可能
		{
			send(socked,"error1",strlen("error1"),0);
		}
	}

	write(uart[1].fd, send_rfid3, sizeof(send_rfid3));			//保存模式设置
	usleep(1000*200);
	if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0)		//排除未收到数据的可能
	{
		printf ("OK: RFID mode numbytes = %d\n", uart[1].len); 
		for(i = 0; i < uart[1].len; i++)
		{
			printf("%02x ", uart[1].buf[i]);
		}
		printf ("\nOK: RFID mode string \n");
		crc = crc_func(uart[1].buf, uart[1].len);
		if(crc != 0)											//crc校验，确保数据完整								
		{
			write(uart[1].fd, send_rfid3, sizeof(send_rfid3));			//保存模式设置
			usleep(1000*200);
			if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
			{
				crc = crc_func(uart[1].buf, uart[1].len);
				if(crc != 0)
				{
					send(socked,"error1",strlen("error1"),0);
				}
			}
		}
	}
	else
	{
		write(uart[1].fd, send_rfid3, sizeof(send_rfid3));	
		usleep(1000*200);
		if((uart[1].len = read(uart[1].fd,uart[1].buf,200))<=0) 		//排除未收到数据的可能
		{
			send(socked,"error1",strlen("error1"),0);
		}
	}

	memset(sdbuf1, 0, 200);


	while(1)
	{
		memset(revbuf, 0, 200);
		memset(uart[1].buf, 0, 200);
		flag1 = 0;
		sum=recv(socked ,revbuf,LENGTH,0);
		if(sum<= 0) 
		{  
			if(errno == EINTR)
			{
				continue;
			}else
			{
				printf("ERROR:Fail to get string\n"); 
				break;
			}
		}
		else
		{
			printf ("OK: TCP numbytes = %d\n", sum); 		
			printf ("\nOK: TCP string %s\n", revbuf);

			if(strcmp(revbuf, "read01") == 0)
			{
				write(uart[1].fd, send_rfid2, sizeof(send_rfid2));			//读数据
				usleep(1000*200);
				if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
				{
					printf ("OK: RFID numbytes = %d\n", uart[1].len); 		
					for(i = 0; i < uart[1].len; i++)
					{
						printf("%02x ", uart[1].buf[i]);
					}
					printf ("\nOK: RFID mode string \n");
					crc = crc_func(uart[1].buf, uart[1].len);
					if(crc != 0)											//crc校验，确保数据完整	
					{
						write(uart[1].fd, send_rfid2, sizeof(send_rfid2));
						usleep(1000*200);
						if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
						{
							crc = crc_func(uart[1].buf, uart[1].len);
							if(crc != 0)
							{
								send(socked,"error1",strlen("error1"),0);
							}
						}
					}
					else
					{
						if(uart[1].buf[1] !=  0x41)							//排除未读到码的可能性
						{
							send(socked,"error2",strlen("error2"),0);
						}else
						{
							for(i = 0; i < 60; i++)
							{
								if(uart[1].buf[i + 6] != 0)
								{
									flag1++;
									sdbuf1[flag1 - 1] = uart[1].buf[i + 6];
								}				
							}
							
							send(socked,sdbuf1,flag1,0);
						}
					}
				}
				else
				{
					write(uart[1].fd, send_rfid2, sizeof(send_rfid2));
					usleep(1000*200);
					if((uart[1].len = read(uart[1].fd,uart[1].buf,200))<=0) 		//排除未收到数据的可能
					{
						send(socked,"error1",strlen("error1"),0);
					}
				}
			}

		}
	}
	printf("pthread_exit is open\n");
	pthread_exit(0); 
}


void tcp3thread(void *arg)
{
	int socked;
	socked = *(int *)arg;
	int sum = 0;
	char revbuf[LENGTH]; // Receive buffer 

	char sdbuf1[LENGTH];

	int i = 0;
	int z = 0;
	int crc = 0;
	int flag1 = 0;
	int flag2 = 0;

	FILE *f;
	long len;
	char *content;
	cJSON *json;
	cJSON *pSub;
	cJSON *format;
	cJSON *parameter;
	char *key;
	char *key1;
	int parameter_size;
	int format_size;

	uartmode *uart = (uartmode *)malloc(sizeof(uartmode)*5);
	uart[1].name = "/dev/ttyO2";

	f=fopen("./test.json","rb");
	fseek(f,0,SEEK_END);
	len=ftell(f);
	fseek(f,0,SEEK_SET);
	content=(char*)malloc(len+1);
	fread(content,1,len,f);
	fclose(f);

	
	json=cJSON_Parse(content);
	
	parameter_size = cJSON_GetArraySize(json);
	
	for (i = 0; i < parameter_size; i++)
	{

		pSub = cJSON_GetArrayItem(json,i);	
		key = pSub->string;
		format = NULL;

		if(strcmp(key,"/dev/ttyO2") == 0)
		{	

			format = cJSON_GetObjectItem(json,"/dev/ttyO2");
			format_size = cJSON_GetArraySize(format);
			for(z = 0; z < format_size; z++)
			{
				parameter = cJSON_GetArrayItem(format,z);	
				key1 = parameter->string;
				if(strcmp(key1,"speed") == 0)
				{

					uart[1].speed = cJSON_GetObjectItem(format,"speed")->valueint;
				
				}				
			}
		}
	}
	cJSON_Delete(json);
	uart[1].fd = uart_init(uart[1].name, uart[1].speed);

	write(uart[1].fd, send_rfid1, sizeof(send_rfid1));			//RFID2模式设置为交互模式
	usleep(1000*200);


	
	if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
	{


		
		printf("OK: RFID mode numbytes = %d\n", uart[1].len); 
		for(i = 0; i < uart[1].len; i++)
		{
			printf("%02x ", uart[1].buf[i]);
		}
		printf ("\nOK: RFID mode string \n");
		crc = crc_func(uart[1].buf, uart[1].len);
		if(crc != 0)											//crc校验，确保数据完整								
		{
			write(uart[1].fd, send_rfid1, sizeof(send_rfid1));
			usleep(1000*200);
			if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 
			{
				crc = crc_func(uart[1].buf, uart[1].len);
				if(crc != 0)
				{
					send(socked,"error1",strlen("error1"),0);
				}
			}
		}
	}else
	{

		
		write(uart[1].fd, send_rfid1, sizeof(send_rfid1));
		usleep(1000*200);
		if((uart[1].len = read(uart[1].fd,uart[1].buf,200))<=0)
		{
	
			send(socked,"error1",strlen("error1"),0);
		}		
	}

	write(uart[1].fd, send_rfid3, sizeof(send_rfid3));			//保存模式设置
	usleep(1000*200);
	if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0)		//排除未收到数据的可能
	{
		printf ("OK: RFID mode numbytes = %d\n", uart[1].len); 
		for(i = 0; i < uart[1].len; i++)
		{
			printf("%02x ", uart[1].buf[i]);
		}
		printf ("\nOK: RFID mode string \n");
		crc = crc_func(uart[1].buf, uart[1].len);
		if(crc != 0)											//crc校验，确保数据完整								
		{
			write(uart[1].fd, send_rfid3, sizeof(send_rfid3));
			usleep(1000*200);
			if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 
			{
				crc = crc_func(uart[1].buf, uart[1].len);
				if(crc != 0)
				{
					send(socked,"error1",strlen("error1"),0);
				}
			}
		}
	}
	else
	{
		write(uart[1].fd, send_rfid3, sizeof(send_rfid3));
		usleep(1000*200);
		if((uart[1].len = read(uart[1].fd,uart[1].buf,200))<=0)
		{
			send(socked,"error1",strlen("error1"),0);
		}	
	}

	memset(sdbuf1, 0, 200);

	while(1)
	{
		memset(revbuf, 0, 200);
		memset(uart[1].buf, 0, 200);
		flag1 = 0;
		sum=recv(socked ,revbuf,LENGTH,0);
		if(sum<= 0) 
		{  
			if(errno == EINTR)
			{
				continue;
			}else
			{
				printf("ERROR:Fail to get string\n"); 
				break;
			}
		}
		else
		{
			printf ("OK: TCP numbytes = %d\n", sum); 		
			printf ("\nOK: TCP string %s\n", revbuf);

			if(strcmp(revbuf, "read01") == 0)
			{
				write(uart[1].fd, send_rfid2, sizeof(send_rfid2));			//读数据
				usleep(1000*200);
				if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 		//排除未收到数据的可能
				{
					printf ("OK: RFID numbytes = %d\n", uart[1].len); 		
					for(i = 0; i < uart[1].len; i++)
					{
						printf("%02x ", uart[1].buf[i]);
					}
					printf ("\nOK: RFID mode string \n");
					crc = crc_func(uart[1].buf, uart[1].len);
					if(crc != 0)											//crc校验，确保数据完整	
					{
						write(uart[1].fd, send_rfid2, sizeof(send_rfid2));	
						usleep(1000*200);
						if((uart[1].len = read(uart[1].fd,uart[1].buf,200))>0) 
						{
							crc = crc_func(uart[1].buf, uart[1].len);
							if(crc != 0)
							{
								send(socked,"error1",strlen("error1"),0);
							}
						}
					}else
					{
						if(uart[1].buf[1] !=  0x41)							//排除未读到码的可能性
						{
							send(socked,"error2",strlen("error2"),0);
						}else
						{
							for(i = 0; i < 60; i++)
							{
								if(uart[1].buf[i + 6] != 0)
								{
									flag1++;
									sdbuf1[flag1 - 1] = uart[1].buf[i + 6];
								}				
							}
							
							send(socked,sdbuf1,flag1,0);
						}
					}
				}
				else
				{
					write(uart[1].fd, send_rfid2, sizeof(send_rfid2));
					usleep(1000*200);
					if((uart[1].len = read(uart[1].fd,uart[1].buf,200))<=0)
					{
						send(socked,"error1",strlen("error1"),0);
					}
				}
			}
		}
	}
	printf("pthread_exit is open\n");
	pthread_exit(0); 
}



