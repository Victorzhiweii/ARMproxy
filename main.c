#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>  /* netdb is necessary for struct hostent */
#include <pthread.h> 
#include <signal.h>
// #include "tcpserver.h"
#include "sock.h"
#include "uart.h" 
#include "cJSON.h"
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include "common.h"
#include "profile.h"
#include "command.h"

static const char* APPVERSION = "20190222.002";



static const char PLC_COMMAND_LIST[][20] = {
	"READ01",
	"TEST01",
	"RETEST01",
	"TESTOK",
	"TESTNG",
	"ERROR01",  // rfid 初始化失败
	"ERROR02",  // macmini01 联机失败
	"ERROR03",  // macmini02 联机失败
	"ERROR04",  // macmini03 联机失败
	"ERROR05",  // rfid 写命令失败
	"ERROR06",  // rfid 读取失败
	"ERROR07",  // rfid 读取校验失败
	"ERROR08",  // rfid 读取超时
	"ERROR09",  // rfid 读取数据无效
	"ERROR10",  // socket 发送数据失败
	"ERROR11",  // socket mini response busy
};

static DeviceContainerInfo* gDeviceContainerInfo;
static int notify_data_recv(char* name, char* buffer, int buffer_length)
{
	printf("notify_data_recv(1): %s , len: %d, content: \n", name, buffer_length);
#if 1
	dumpbufferinfo(buffer, buffer_length);
#endif

	CmdRouterInfo* router = gDeviceContainerInfo->Router;

	while(router != NULL)
	{
		// 判断是否是来主动发起方的命令消息
		if(buffer_length >= strlen(router->Cmd) && strcmp(name, router->DevNameFrom) == 0)
		{
			if(memcmp(buffer, router->Cmd, buffer_length) == 0)
			{
				// 指定的命令
				CmdDeviceLinkInfo* item_to = router->DevTo;
				while(item_to != NULL)
				{
					do_command_action(router->Cmd, 
						get_device_info(gDeviceContainerInfo, router->DevNameFrom), 
						get_device_info(gDeviceContainerInfo, item_to->Name), 
						get_device_info(gDeviceContainerInfo, router->DevNameReply));

					item_to = item_to->Next;
				}
				return 0;
			}
		}

		// 来自回复设备的消息


		printf("%s(%s, %s) \n", router->Cmd, router->DevNameFrom, router->DevNameReply);

		router = router->Next;
	}

	CommunicationBaseInfo* device = get_device_info(gDeviceContainerInfo, name);
	if(device != NULL)
	{
		printf("notify_data_recv(20), ready send data to name:%s\n", name);
		QueueManager* last_send_queue = device->LastSendQueue;
		if(last_send_queue != NULL)
		{
			CommunicationBaseInfo* reply_device = last_send_queue->ReplyDevice;
			if(reply_device != NULL)
			{
				try_send_to_data(reply_device, buffer, buffer_length);
			}
			else
			{
				printf("notify_data_recv(22), can't found the reply device for name:%s\n", name);
			}
		}
		else
		{
			printf("notify_data_recv(23), can't found last send device for name:%s\n", name);
		}
		
	}
	else
	{
		printf("notify_data_recv(24), can't found the device by name:%s\n", name);
	}
	

    return 0;
}

static int quit_flag = 0;
static void signal_handler(int sig)
{
	printf("signal_handler(1) get exit signal:%d\n", sig);
	CommunicationBaseInfo* info = gDeviceContainerInfo->Devices;
	while(info != NULL)
	{
		info->ExitFlag = 1;
		info = info->Next;
	}

    quit_flag = 1;
}


int main(int argc, const char *argv[])
{
	char config_file[256];	
	DeviceContainerInfo device_info;
	memset(&device_info, 0, sizeof(DeviceContainerInfo));
	gDeviceContainerInfo = &device_info;

	/* 
		参数个数小于1则返回，按如下方式执行:
		./uart_test /dev/ttyAT1
	*/
		
	printf("app version:%s\n", APPVERSION);

	if(argc > 1 && (memcmp(argv[1], "-v", strlen("-v")) == 0
					|| memcmp(argv[1], "-h", strlen("-h")) == 0))
	{
		printf("Useage: -v show app version\n");
		printf(" 		-c config file path\n");
		printf(" 		-h show help\n");
		exit(0);
		return 0;
	}

	strcpy(config_file, "./appconfig.json");
	if (argc > 2 && memcmp(argv[1], "-c", strlen("-c")) == 0) 
	{
		printf("check config file: %s\n", argv[2]);
		strcpy(config_file, argv[2]);
    } 

	if(parse_profile_file(&device_info, config_file) != 0)
	{
		printf("main(2): parse config file:%s error\n", config_file);
		exit(1);
		return 1;
	}


	if(device_info.Devices != NULL)
    {
        printf(">>>>>>>>>>>>list devices>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        CommunicationBaseInfo* info = device_info.Devices;
        while(info != NULL)
        {
            if(info->Mode == SerialPort)
            {
                printf("Start Thread for SerialPort(%s), %s, %d\n", info->Name, ((SerialPortInfo*)(info->RealPtr))->Dev, ((SerialPortInfo*)(info->RealPtr))->Speed);
				
				info->Callback = notify_data_recv;

				pthread_create(&(info->handler),NULL,(void*)SerialPortThread, info->RealPtr);
            }
            else if(info->Mode == SockClient)
            {
                printf("Start Thread for SockClient(%s), %s : %d\n", info->Name, ((SocketInfo*)(info->RealPtr))->Ip, ((SocketInfo*)(info->RealPtr))->Port);
				
				info->Callback = notify_data_recv;

				pthread_create(&(info->handler),NULL,(void*)tcpSocketClientThread, info->RealPtr);
            }
            else if(info->Mode == SockServer)
            {
                printf("SockServer(%s), %s : %d\n", info->Name, ((SocketInfo*)(info->RealPtr))->Ip, ((SocketInfo*)(info->RealPtr))->Port);
            }
            
            info = info->Next;
        }
        printf("<<<<<<<<<<list devices end<<<<<<<<<<<<<<<<<<<<\n");
    }
	
	printf("main(10) waiting for exit signal\n");

	signal(SIGINT,signal_handler);
    while(!quit_flag)
    {
        sleep(2);
    }

	CommunicationBaseInfo* info = device_info.Devices;
	while(info != NULL)
	{
		printf("main(11) thread name:%s handle:%ld\n", info->Name, info->handler);
		if(info->handler != 0)
		{
			pthread_join(info->handler, NULL);
		}

		info = info->Next;
	}

	free_profile_info(gDeviceContainerInfo);

	printf("main(20) app exit\n");
}

