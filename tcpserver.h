#ifndef __TCPSERVER_H
#define __TCPSERVER_H



#define BACKLOG 	10 
#define LENGTH 512              		// Buffer length 

void tcp1thread(void * arg);
void tcp2thread(void * arg);
void tcp3thread(void * arg);


typedef struct {
	int fd;
	int speed;
	char buf[256];
	char* name;
	int len;	
}uartmode;

typedef struct {
	int socked;
	int mode;
	int socked_test1;
	int socked_MAC1;
	int socked_MAC2;
	int socked_MAC3;
	char buf[256];
	char buf1[256];
	int flag;
}parameter_mode;





#endif

