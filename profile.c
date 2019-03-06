
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h> 
#include <errno.h>
#include "cJSON.h"
#include "common.h"
#include "profile.h"


static int default_notify_data_recv(char* name, char* buffer, int buffer_length)
{
    printf("default_notify_data_recv(1): %s , len: %d, cmd: \n", name, buffer_length);
#if 1
	dumpbufferinfo(buffer, buffer_length);
#endif
    return 0;
}

static int check_and_send_data(void* arg)
{
    CommunicationBaseInfo* info = (CommunicationBaseInfo*)arg;
    
    QueueManager* queuemgr = &(info->SendQueue);
    if(queuemgr->PushIndex != queuemgr->PopIndex)
    {
        #if 1
        printf("process_data_send(20-%s), push: %d , pop: %d \n", 
                info->Name, queuemgr->PushIndex, queuemgr->PopIndex);
        
        pthread_spin_lock(&(queuemgr->LockObject));

        QueueBuffer* ptr = queuemgr->Buffer + queuemgr->PopIndex;

        queuemgr->PopIndex++;
        if(queuemgr->PopIndex >= MAX_SEND_BUFFER_COUNT)
        {
            queuemgr->PopIndex = 0;
        }
        pthread_spin_unlock(&(queuemgr->LockObject));

        if(ptr->Len > 0)
        {
            int len = write(info->fd, ptr->Buf, ptr->Len);

            // 缓存最后一次发送数据
            //memcpy(&(info->LastSendBuffer), ptr, sizeof(QueueBuffer));

            info->LastSendQueue = queuemgr;

            if(len != ptr->Len)
            {
                printf("process_data_send(21-%s), send to socket err: %d != %d, err: %d, push: %d , pop: %d \n", 
                        info->Name, len, ptr->Len, errno, queuemgr->PushIndex, queuemgr->PopIndex);

                if(info->Callback != NULL)
                {
                    info->Callback(info->Name, "ERR:SENDFAIL#", strlen("ERR:SENDFAIL#"));
                }

                return 0;
            }
            else
            {
                printf("process_data_send(22-%s), send to socket success len : %d, push: %d , pop: %d \n", 
                    info->Name, len, queuemgr->PushIndex, queuemgr->PopIndex);

                // usleep(1000 * 100);

                return len;
            }
        }
        else
        {
            printf("process_data_send(23-%s), no any data to send, push: %d , pop: %d \n", 
                    info->Name, queuemgr->PushIndex, queuemgr->PopIndex);
        }
        
        #endif
    }

    return 0;
}


static int process_try_send(void* arg, char* buffer, int buffer_length)
{
    CommunicationBaseInfo* info = (CommunicationBaseInfo*)arg;

    QueueManager* queuemgr = &(info->SendQueue);


    pthread_spin_lock(&(queuemgr->LockObject));
    int index = queuemgr->PushIndex;
    if(buffer_length > 0 && buffer != NULL)
    {
        memcpy(queuemgr->Buffer[index].Buf, buffer, buffer_length);
        queuemgr->Buffer[index].Buf[buffer_length] = '\0';
        queuemgr->Buffer[index].Len = buffer_length;

        #if 1
        printf("process_try_send(3): %s , len: %d, send data:", info->Name, buffer_length);
        dumpbufferinfo(buffer, buffer_length);			
        #endif

        index++;
        if(index >= MAX_SEND_BUFFER_COUNT)
        {
            index = 0;
        }
        queuemgr->PushIndex = index;
    }
    else
    {
        memset(&(queuemgr->Buffer[index]), 0, sizeof(QueueBuffer));

        printf("process_try_send(8): %s , %d, no any data to send:", info->Name, buffer_length);
        dumpbufferinfo(buffer, buffer_length);

        buffer_length = 0;
    }
    
    pthread_spin_unlock(&(queuemgr->LockObject));

    return buffer_length;
}

static int process_data_recv(void* arg, char* buffer, int buffer_length)
{
    CommunicationBaseInfo* info = (CommunicationBaseInfo*)arg;
    
    if(info->Callback != NULL && buffer_length > 0)
    {
        info->Callback(info->Name, buffer, buffer_length);

        // 缓存最后一次接收到的数据
        memcpy(info->LastRecvBuffer.Buf, buffer, buffer_length);
        info->LastRecvBuffer.Buf[buffer_length] = '\0';
        info->LastRecvBuffer.Len = buffer_length;

        return buffer_length;
    }
    else if(buffer_length <= 0)
    {
        printf("process_data_recv(10-%s), no recv any valid data\n", info->Name);
    }
    else 
    {
        printf("process_data_recv(11-%s), no callback handler.\n", info->Name);
    }

    return 0;
}


static void fill_socket_profile(SocketInfo* info, cJSON *data)
{
	cJSON *node;
	char* value;
	char *key;
	int i, element_count;

	element_count = cJSON_GetArraySize(data);
	//printf("fill_socket_profile(1) child_count %d \n", child_count);
	for(i = 0; i < element_count; i++)
	{
		node = cJSON_GetArrayItem(data, i);	
		key = node->string;

		//printf("fill_socket_profile(2) index: %d, key:%s \n", z, key);
		
		if(strcmp(key,"ip") == 0)
		{
			value = node->valuestring;
			//printf("fill_socket_profile(4) index: %d, value:%s \n", z, addr);
			strcpy(info->Ip, value);
		}
		else if(strcmp(key,"port") == 0)
		{
			info->Port = node->valueint;	

			//printf("fill_socket_profile(4) index: %d, value:%d \n", z, info->port);
		}
	}
}

static void fill_uart_profile(SerialPortInfo* info, cJSON *data)
{
	cJSON *node;
	char* value;
	char *key;
	int i, element_count;

	element_count = cJSON_GetArraySize(data);
	//printf("fill_uart_profile(1) child_count %d \n", child_count);
	for(i = 0; i < element_count; i++)
	{
		node = cJSON_GetArrayItem(data, i);	
		key = node->string;
		//printf("fill_uart_profile(2) index: %d, key:%s \n", z, key);
		if(strcmp(key,"dev") == 0)
		{
			value = node->valuestring;
			strcpy(info->Dev, value);
		}
		else if(strcmp(key,"speed") == 0)
		{
			info->Speed = node->valueint;
		}
	}
}


static CommunicationBaseInfo* parse_devices_profile_node(char* name, cJSON *data)
{
    cJSON *node;
	char* value;
	char *key;
	int z, element_count;
    
    element_count = cJSON_GetArraySize(data);

    printf("parse_devices_profile_node(1), %s, %d \n", name, element_count);

	node = cJSON_GetObjectItem(data, "mode");
    value = node->valuestring;
    printf("parse_devices_profile_node(2), %s, %s \n", name, value);

    if(strcmp(value, "SerialPort") == 0)
    {
        SerialPortInfo* ptr = (SerialPortInfo*)malloc(sizeof(SerialPortInfo));
        memset(ptr, 0, sizeof(SerialPortInfo));
        strcpy(ptr->Base.Name, name);
        ptr->Base.RealPtr = ptr;
        ptr->Base.Mode = SerialPort;

        fill_uart_profile(ptr, data);

        printf("parse_devices_profile_node(3), %s SerialPort, dev:%s, speed:%d \n", 
                    name, ptr->Dev, ptr->Speed);

        pthread_spin_init(&(ptr->Base.SendQueue.LockObject), PTHREAD_PROCESS_PRIVATE);
        ptr->Base.ProcessRecv = process_data_recv;
        ptr->Base.ProcessSend = check_and_send_data;
        ptr->Base.TrySend = process_try_send;
        ptr->Base.Callback = default_notify_data_recv;

        return &(ptr->Base);
    }
    else if(strcmp(value, "SockClient") == 0
            || strcmp(value, "SockServer") == 0)
    {
        SocketInfo* ptr = (SocketInfo*)malloc(sizeof(SocketInfo));
        memset(ptr, 0, sizeof(SocketInfo));
        strcpy(ptr->Base.Name, name);
        ptr->Base.RealPtr = ptr;
        
        if(strcmp(value, "SockClient") == 0)
        {
            ptr->Base.Mode = SockClient;
        }
        else
        {
            ptr->Base.Mode = SockServer;
        }
        
        fill_socket_profile(ptr, data);

        printf("parse_devices_profile_node(3), %s %s, ip:%s, port:%d \n", 
                    name, value, ptr->Ip, ptr->Port);

        pthread_spin_init(&(ptr->Base.SendQueue.LockObject), PTHREAD_PROCESS_PRIVATE);

        ptr->Base.ProcessRecv = process_data_recv;
        ptr->Base.ProcessSend = check_and_send_data;
        ptr->Base.TrySend = process_try_send;
        ptr->Base.Callback = default_notify_data_recv;

        return &(ptr->Base);
    }

    return (CommunicationBaseInfo*)NULL;
}


static int parse_devices_profile(DeviceContainerInfo* device_info, cJSON *data)
{
	// cJSON *format;
	// cJSON *child_obj;
	char* addr;
	char *key;
	int i, element_count;
	cJSON* devices = cJSON_GetObjectItem(data, "DEVICES");
	//printf("fill_uart_profile(0) name : %s \n", info->name);
	
	element_count = cJSON_GetArraySize(devices);

#if ENABLE_DEBUG
	printf("parse_devices_profile(1) element_count : %d \n", element_count);
#endif

    cJSON* dev_node = cJSON_GetArrayItem(devices, 0);
    //key = dev_node->string;
	//printf("parse_devices_profile(2) index: %d, key:%s \n", i, key);

    CommunicationBaseInfo* lastdevices = NULL;

    while(dev_node != NULL)
    {
        key = dev_node->string;
		printf("parse_devices_profile(2) key:%s \n", key);

        if(lastdevices == NULL)
        {
            device_info->Devices = lastdevices = parse_devices_profile_node(key, cJSON_GetObjectItem(devices, key));
        }
        else
        {
            lastdevices->Next = parse_devices_profile_node(key, cJSON_GetObjectItem(devices, key));
            lastdevices = lastdevices->Next;
        }

        if(lastdevices == NULL)
        {
            printf("parse_devices_profile(5) parse node:%s failed\n", key);
            return 1;
        }

        dev_node = dev_node->next;
    }

    return 0;
}


static CmdRouterInfo* parse_route_profile_node(char* cmd, cJSON *data)
{
    cJSON *node;
	char* value;
	char *key;
	int i, element_count;
    CmdDeviceLinkInfo* dev_item = NULL;

    CmdRouterInfo info;
    memset(&info, 0, sizeof(CmdRouterInfo));
    strcpy(info.Cmd, cmd);

    printf("parse_route_profile_node(1) cmd:%s \n", cmd);

	node = cJSON_GetObjectItem(data, "from");
    value = node->valuestring;

    printf("parse_route_profile_node(2) from:%s \n", value);

    strcpy(info.DevNameFrom, value);

    node = cJSON_GetObjectItem(data, "to");
    element_count = cJSON_GetArraySize(node);
    
    printf("parse_route_profile_node(3) to count:%d \n", element_count);

    dev_item = NULL;
    for(i=0; i<element_count; i++)
    {
        cJSON* child_node = cJSON_GetArrayItem(node, i);

        printf("parse_route_profile_node(4) %s -> %s \n", child_node->string, child_node->valuestring);

        if(dev_item == NULL)
        {
            info.DevTo = dev_item = (CmdDeviceLinkInfo*)malloc(sizeof(CmdDeviceLinkInfo));
            memset(dev_item, 0, sizeof(CmdDeviceLinkInfo));
        }
        else
        {
            dev_item->Next = (CmdDeviceLinkInfo*)malloc(sizeof(CmdDeviceLinkInfo));
            memset(dev_item->Next, 0, sizeof(CmdDeviceLinkInfo));
            dev_item = dev_item->Next;
        }

        strcpy(dev_item->Name, child_node->valuestring);
    }
    
    
    node = cJSON_GetObjectItem(data, "reply");
    strcpy(info.DevNameReply, node->valuestring);
    // element_count = cJSON_GetArraySize(node);

    // printf("parse_route_profile_node(7) reply count:%d \n", element_count);

    // dev_item = NULL;
    // for(i=0; i<element_count; i++)
    // {
    //     cJSON* child_node = cJSON_GetArrayItem(node, i);
    //     if(dev_item == NULL)
    //     {
    //         info.DevReply = dev_item = (CmdDeviceLinkInfo*)malloc(sizeof(CmdDeviceLinkInfo));
    //         memset(dev_item, 0, sizeof(CmdDeviceLinkInfo));
    //     }
    //     else
    //     {
    //         dev_item->Next = (CmdDeviceLinkInfo*)malloc(sizeof(CmdDeviceLinkInfo));
    //         memset(dev_item->Next, 0, sizeof(CmdDeviceLinkInfo));
    //         dev_item = dev_item->Next;
    //     }

    //     strcpy(dev_item->Name, child_node->valuestring);
    // }

    CmdRouterInfo* ptr = (CmdRouterInfo*)malloc(sizeof(CmdRouterInfo));
    memcpy(ptr, &info, sizeof(CmdRouterInfo));

    return ptr;
}

static int parse_route_profile(DeviceContainerInfo* device_info, cJSON *data)
{
	// cJSON *format;
	// cJSON *child_obj;
	char* addr;
	char *key;
	int i, child_count;

	cJSON* routelist = cJSON_GetObjectItem(data, "CMDROUTE");
	//printf("fill_uart_profile(0) name : %s \n", info->name);
	
	child_count = cJSON_GetArraySize(routelist);

#if ENABLE_DEBUG
	printf("parse_route_profile(1) child_count : %d \n", child_count);
#endif

    cJSON* dev_node = cJSON_GetArrayItem(routelist, 0);	
    //key = dev_node->string;
	//printf("parse_route_profile(2) index: %d, key:%s \n", i, key);

    CmdRouterInfo* last_node = NULL;
    while(dev_node != NULL)
    {
        key = dev_node->string;
		printf("parse_route_profile(2) key:%s \n", key);
        if(last_node == NULL)
        {
            device_info->Router = last_node = parse_route_profile_node(key, cJSON_GetObjectItem(routelist, key));
        }
        else
        {
            last_node->Next = parse_route_profile_node(key, cJSON_GetObjectItem(routelist, key));
            last_node = last_node->Next;
        }

        if(last_node == NULL)
        {
            printf("parse_route_profile(5) parse node:%s failed\n", key);
            return 1;
        }

        dev_node = dev_node->next;
    }

    return 0;
}

int parse_profile_file(DeviceContainerInfo* device_info, const char* configfile)
{
	FILE *fp;
	long len;
	char *content;

	fp=fopen(configfile,"rb");
	if(fp == NULL){
		printf("parse_profile_file(2): open %s file error\n", configfile);
		return 1;
	}

	fseek(fp,0,SEEK_END);
	len=ftell(fp);
	fseek(fp,0,SEEK_SET);
	content=(char*)malloc(len+1);
    if(content == NULL)
    {
        printf("parse_profile_file(3): malloc size: %ld ,error:%d\n", len, errno);
        fclose(fp);
        return 2;
    }

	fread(content,1,len,fp);
	fclose(fp);


	cJSON* json=cJSON_Parse(content);

	len = cJSON_GetArraySize(json);

    if(parse_devices_profile(device_info, json) != 0)
    {
        printf("parse_profile_file(6): parse devices failed\n");
        return 1;
    }

    if(parse_route_profile(device_info, json) != 0)
    {
        printf("parse_profile_file(7): parse route failed\n");
        return 1;
    }

    free(content);

    dump_profile_info(device_info);

    return 0;
}

void free_profile_info(DeviceContainerInfo* device_info)
{
    printf(">>>>>>>>>>>>free memory>>>>>>>>>>>>>>>>>>>>>>>>>\n");

    if(device_info->Devices != NULL)
    {
        CommunicationBaseInfo* info = device_info->Devices;
        while(info != NULL)
        {
            CommunicationBaseInfo* del_node = info->RealPtr;
            info = info->Next;

            if(del_node->RealPtr != NULL)
            {
                pthread_spin_destroy(&(del_node->SendQueue.LockObject));
                printf("free device node:%s\n", del_node->Name);
                free(del_node->RealPtr);
            }
        }
    }

    if(device_info->Router != NULL)
    {
        CmdRouterInfo* router = device_info->Router;
        while(router != NULL)
        {
            CmdDeviceLinkInfo* item = router->DevTo;
            while(item != NULL)
            {
                CmdDeviceLinkInfo* del_node = item;
                item = item->Next;
                if(del_node != NULL)
                {
                    printf("free route link node to:%s\n", del_node->Name);
                    free(del_node);
                }
            }

            // item = router->DevReply;
            // while(item != NULL)
            // {
            //     CmdDeviceLinkInfo* del_node = item;
            //     item = item->Next;
            //     if(del_node != NULL)
            //     {
            //         printf("free route link node reply:%s\n", del_node->Name);
            //         free(del_node);
            //     }
            // }

            CmdRouterInfo* del_node = router;
            router = router->Next;

            if(del_node != NULL)
            {
                printf("free route node:%s\n", del_node->Cmd);
                free(del_node);
            }
        }
    }

    printf("<<<<<<<<<<free memory end<<<<<<<<<<<<<<<<<<<<\n");
}

void dump_profile_info(DeviceContainerInfo* device_info)
{
    if(device_info->Devices != NULL)
    {
        printf(">>>>>>>>>>>>dump devices>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        CommunicationBaseInfo* info = device_info->Devices;
        while(info != NULL)
        {
            if(info->Mode == SerialPort)
            {
                printf("SerialPort(%s), %s, %d\n", info->Name, ((SerialPortInfo*)(info->RealPtr))->Dev, ((SerialPortInfo*)(info->RealPtr))->Speed);
            }
            else if(info->Mode == SockClient)
            {
                printf("SockClient(%s), %s : %d\n", info->Name, ((SocketInfo*)(info->RealPtr))->Ip, ((SocketInfo*)(info->RealPtr))->Port);
            }
            else if(info->Mode == SockServer)
            {
                printf("SockServer(%s), %s : %d\n", info->Name, ((SocketInfo*)(info->RealPtr))->Ip, ((SocketInfo*)(info->RealPtr))->Port);
            }
            
            info = info->Next;
        }
        printf("<<<<<<<<<<dump devices end<<<<<<<<<<<<<<<<<<<<\n");
    }

    if(device_info->Router != NULL)
    {
        printf(">>>>>>>>>>>>dump route>>>>>>>>>>>>>>>>>>>>>>>>>\n");

        CmdRouterInfo* router = device_info->Router;
        while(router != NULL)
        {
            char dev_to[256];
            memset(dev_to, 0, sizeof(dev_to));

            char dev_reply[256];
            memset(dev_reply, 0, sizeof(dev_reply));

            CmdDeviceLinkInfo* item = router->DevTo;
            strcpy(dev_to, "to:");
            while(item != NULL)
            {
                strcat(dev_to, item->Name);
                strcat(dev_to, ",");
                item = item->Next;
            }

            printf("%s(%s -> %s <- %s) \n", router->Cmd, router->DevNameFrom, dev_to, router->DevNameReply);

            router = router->Next;
        }

        printf("<<<<<<<<<<dump route end<<<<<<<<<<<<<<<<<<<<\n");
    }
}

void dumpbufferinfo(const char* buffer, int len)
{
	int i;
    if(buffer != NULL)
    {
        for(i = 0; i < len; i++)
        {
            if(buffer[i] >= 32 && buffer[i] < 127)
            {
                printf("%c ", buffer[i]);
            }
            else if(buffer[i] == '\0')
            {
                break;
            }
            else
            {
                printf("0x%02X ", buffer[i]);
            }
        }

        printf ("\n");
    }	
}

CommunicationBaseInfo* get_device_info(DeviceContainerInfo* device_info, const char* name)
{
    CommunicationBaseInfo* info = device_info->Devices;
    while(info != NULL)
    {
        if(strcmp(name, info->Name) == 0)
            return info;
        
        info = info->Next;
    }

    return NULL;
}
