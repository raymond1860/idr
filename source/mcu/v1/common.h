#ifndef COMMON_H
#define COMMON_H 1
typedef unsigned char uint8;
typedef unsigned int  uint32;

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
void uart1_write(char c);
//return read char count
char uart1_read();


//UART2 is for SAM&MCU
void uart2_init(void);
void uart2_write(char c);
//return read char count
char uart2_read();


#endif