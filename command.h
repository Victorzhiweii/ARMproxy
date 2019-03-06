#ifndef __COMMAND_H__
#define __COMMAND_H__



// int notify_data_recv(char* name, char* buffer, int buffer_length);

void do_command_action(const char* cmd, CommunicationBaseInfo* from, CommunicationBaseInfo* to, CommunicationBaseInfo* reply);
int try_send_data(CommunicationBaseInfo* to, CommunicationBaseInfo* reply, const char* buffer, int buffer_length);
int try_send_to_data(CommunicationBaseInfo* to, const char* buffer, int buffer_length);

#endif
