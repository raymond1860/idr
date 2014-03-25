#ifndef COMMON_H
#define COMMON_H 1


#define HEART_BEAT P03

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;


extern unsigned char xdata comm_buffer[64];
#define FOSC 27000000L

/*UART*/
#define BAUDRATE	115200
#define NONE_PARITY	0
#define ODD_PARITY 1
#define EVEN_PARITY 2
#define MARK_PARITY 3
#define SPACE_PARITY 4

#define PARITY NONE_PARITY


//UART1 is for HOST&MCU
void uart1_init(void);
void uart1_write(const char* buf,int size);	
int uart1_read(char* buf,int max_size);


//UART2 is for SAM&MCU
void uart2_init(void);
void uart2_write(const char* buf,int size);
int uart2_read(char* buf,int max_size);


void Delay1ms();
void DelayMs(int ms);



#endif