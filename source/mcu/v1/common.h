#ifndef COMMON_H
#define COMMON_H 1
#include <stdio.h>
#include <string.h>
#include <intrins.h>
#include "STC15F2K08S2.h"

/*
 * LED debug mode support valid led stage indicator
*/
#define ENABLE_LED_DEBUG

/*
* IAP enabled mode will disable uart1 
*/
//#define IAP_ENABLED

/*
 * Bypass mode is working as no uart connection between mcu and host
*/
//#define ENABLE_BYPASS_MODE


//version is bcd encoding,e.g 0x10 ->v1.0
#define FIRMWARE_VERSION 0x10


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
void Delay1us();

#ifdef ENABLE_LED_DEBUG
//maximum 8bit led debug output
//and assume low active
#define LED_DBG_BIT1 P03
#define LED_DBG_BIT2 P00
#define LED_DBG_BIT3 P23
#define LED_DBG_BIT4 P37
void DbgLeds(uint8 led);
#else
#define DbgLeds(x)
#endif



#endif
