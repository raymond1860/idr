#ifndef COMMON_H
#define COMMON_H 1

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int  uint32;


extern unsigned char xdata comm_buffer[64],comm_buffer2[64];	

#define FOSC 13560000L

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
void uart1_writechar(char c);
void uart1_write(const char* buf,int size);

//return read char count
int uart1_read(char* buf,int max_size);
//read single char ,block operation
//char uart1_readchar();


//UART2 is for SAM&MCU
void uart2_init(void);
void uart2_writechar(char c);
void uart2_write(const char* buf,int size);

//char uart2_readchar(void);
//return read char count
int uart2_read(char* buf,int max_size);


#endif