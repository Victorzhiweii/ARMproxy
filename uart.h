#ifndef __UART_H
#define __UART_H

#define FALSE 1
#define TRUE 0

#define GPIO_IOC_MAGIC   'G'
#define IOCTL_GPIO_SETOUTPUT              _IOW(GPIO_IOC_MAGIC, 0, int)   
   

#define GPIO_TO_PIN(bank, gpio) (32 * (bank) + (gpio))
typedef struct {
        int pin;
        int data;
}am335x_gpio_arg;

static const char send_rfid_cmd_auto_mode[] = {0xFF, 0x0D, 0xA0, 0x00, 0x00, 0x03, 0x10, 0x00, 0x01, 0xF4, 0x00, 0x00, 0x00, 0x32, 0x5f, 0xe0}; //模式设为自动寻卡模式 


static const char send_rfid_cmd_manual_mode[] = {0xFF, 0x0b, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x00, 0x35,0xEA }; //模式设为交互模式 
static const char send_rfid_cmd_read_60[] = {0xFF, 0x07, 0x11,0x00, 0x00,0x00, 0x00,0x3C,0x9F,0x7A }; //读60个标签数据 
static const char send_rfid_cmd_read_40[] = {0xFF, 0x07, 0x11,0x00, 0x00,0x00, 0x00,0x28,0x90,0x7A }; //读40个标签数据 

static const char send_rfid_cmd_store[] = {0xFF, 0x04, 0x16, 0x00, 0x00, 0xE0, 0xD0}; //保存模式设置


void set_speed(int fd, int speed);
int set_Parity(int fd,int databits,int stopbits,int parity);
int uart_init(char *p, int speed);		//初始化uart并打开接受线程，在主函数的最后要加上close（fd）
void uart1thread(void);
void bellthread(void);
void uart2thread(void * arg);


void SerialPortThread(void * arg);


#endif
