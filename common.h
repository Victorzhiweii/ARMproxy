#ifndef __COMMON_H__
#define __COMMON_H__

#include <pthread.h> 

#define BACKLOG 	10 
#define LENGTH 		256              		// Buffer length 

#define MAX_SEND_BUFFER_COUNT  10
#define MAX_SEND_BUFFER_CONTENT_SIZE  100
#define MAX_DEVICE_NAME_SIZE  16
#define MAX_COMMAND_SIZE  32

#define ENABLE_DEBUG	1

#define MAX(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

typedef int (*fun_data_recv_ptr)(char*, char*, int); // 声明一个指向同样参数、返回值的函数指针类型

typedef int (*fun_process_data_ptr)(void*, char*, int); // 声明一个指向同样参数、返回值的函数指针类型

typedef int (*fun_check_and_send_ptr)(void*); // 声明一个指向同样参数、返回值的函数指针类型

typedef struct {
	char Buf[128];
	int Len;
}QueueBuffer;

typedef struct {
	QueueBuffer Buffer[MAX_SEND_BUFFER_COUNT];
	int PushIndex;
	int PopIndex;
	pthread_spinlock_t LockObject;
	void* ReplyDevice;
}QueueManager;

enum CommunicationMode
{
      SerialPort=1, SockClient, SockServer
};

typedef struct Communication_Base {
	char Name[MAX_DEVICE_NAME_SIZE];
	int fd;

	enum CommunicationMode Mode;

	QueueManager SendQueue;

	QueueManager* LastSendQueue;
	QueueBuffer LastRecvBuffer;

	
	pthread_t handler;

	int Connected;

	int ExitFlag;

	void* RealPtr;

	// 
	fun_data_recv_ptr Callback;

	// 处理接收到的数据
	fun_process_data_ptr ProcessRecv;
	
	// 判断本地缓存是否有数据需要发送，有就发送
	fun_check_and_send_ptr ProcessSend;

	// 手动发送数据到本地缓存
	fun_process_data_ptr TrySend;

	struct Communication_Base* Next;
	
} CommunicationBaseInfo;

typedef struct {
    char Dev[48];
    int Speed;

	CommunicationBaseInfo Base;
}SerialPortInfo;

typedef struct {
    char Ip[48];
    int Port;
	CommunicationBaseInfo Base;
}SocketInfo;

typedef struct CmdDeviceLink{
	char Name[MAX_DEVICE_NAME_SIZE];

	struct CmdDeviceLink* Next;

}CmdDeviceLinkInfo;

typedef struct CmdRouter{
	char Cmd[MAX_COMMAND_SIZE];

	char DevNameFrom[MAX_DEVICE_NAME_SIZE];
	CmdDeviceLinkInfo* DevTo;
	char DevNameReply[MAX_DEVICE_NAME_SIZE];

	struct CmdRouter* Next;

}CmdRouterInfo;

typedef struct {
	CommunicationBaseInfo* Devices;
	CmdRouterInfo* Router;
}DeviceContainerInfo;



#endif

