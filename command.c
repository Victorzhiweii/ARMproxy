#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h> 
#include <errno.h>
#include "cJSON.h"
#include "uart.h"
#include "common.h"
#include "profile.h"
#include "command.h"


int try_send_to_data(CommunicationBaseInfo* to, const char* buffer, int buffer_length)
{
    return try_send_data(to, NULL, buffer, buffer_length);
}


int try_send_data(CommunicationBaseInfo* to, CommunicationBaseInfo* reply, const char* buffer, int buffer_length)
{
    //CommunicationBaseInfo* info = (CommunicationBaseInfo*)arg;

    QueueManager* queuemgr = &(to->SendQueue);

    pthread_spin_lock(&(queuemgr->LockObject));
    int index = queuemgr->PushIndex;
    if(buffer_length > 0 && buffer != NULL)
    {
        memcpy(queuemgr->Buffer[index].Buf, buffer, buffer_length);
        queuemgr->Buffer[index].Buf[buffer_length] = '\0';
        queuemgr->Buffer[index].Len = buffer_length;

        #if 1
        printf("try_send_data(3): %s , len: %d, send data:", to->Name, buffer_length);
        dumpbufferinfo(buffer, buffer_length);			
        #endif

        queuemgr->ReplyDevice = reply;

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

        printf("try_send_data(8): %s , %d, no any data to send:", to->Name, buffer_length);
        dumpbufferinfo(buffer, buffer_length);

        buffer_length = 0;
    }
    
    pthread_spin_unlock(&(queuemgr->LockObject));

    return buffer_length;
}





void do_command_action(const char* cmd, CommunicationBaseInfo* from, CommunicationBaseInfo* to, CommunicationBaseInfo* reply)
{
    // 如果是思谷的rfid，就行转码
    if(to != NULL && strncmp(to->Name, "SGRFID", strlen("SGRFID")) == 0)
    {
        if(strncmp(cmd, "READ", strlen("READ")) == 0)
        {
            try_send_data(to, reply, send_rfid_cmd_read_40, sizeof(send_rfid_cmd_read_40));
            return;
        }
    }

    try_send_data(to, reply, cmd, strlen(cmd));
}







// int notify_data_recv(char* name, char* buffer, int buffer_length)
// {
// 	printf("notify_data_recv(1): %s , len: %d, cmd: \n", name, buffer_length);
// #if 1
// 	dumpbufferinfo(buffer, buffer_length);
// #endif



//     return 0;
// }
