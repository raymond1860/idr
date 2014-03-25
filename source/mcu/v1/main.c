#include <stdio.h>
#include <string.h>
#include "STC15F2K08S2.h"
#include "spi.h"
#include "secure.h"
#include "common.h"
#include "packet.h"
#include "thm3060.h"


//#define UART_DEBUGGING

#define MAX_BUFSIZE 128

unsigned char xdata comm_buffer[64];	


void SAM_packet_handler(uint8* buf,int size);
void PRIVATE_packet_handler(uint8* buf,int size);
void Delay1ms()		//@27MHz
{
	unsigned char i, j;	
	i = 27;
	j = 64;
	do
	{
		while (--j);
	} while (--i);
}
void DelayMs(int ms){
	while(ms-->0) Delay1ms();
}
/*
 * idr work flow
 *			  HOST(PC-like uart terminal)
 *             /|\              
 *              |  	
 * THM3060<--->MCU<----->SAM
 *
 * THM3060 is RF non contact ISO/IEC14443 A/B ,ISO/IEC15693 card reader
 * SAM is ID2 card security module
 *
 * Interface between THM3060 and MCU is SPI
 * Interface between SAM and MCU are I2C and UART
 * Interface between HOST and MCU is UART
 *
 * For MCU STC15F2K08S2 based application:
 *     SPI and I2C is gpio simulated.
 *     UART1 is for HOST&MCU
 *     UART2 is for SAM&MCU
 * 
*/


//main function  

void main(void)
{
	unsigned char idata buf[MAX_BUFSIZE];
	int len; 	
	unsigned char prot;
	uint8 i;

	#ifdef HEART_BEAT
	i=3;
	while(i-->0){
	  HEART_BEAT=0;
	  DelayMs(100);
	  HEART_BEAT=1;
	  DelayMs(100);
	}
	#endif
	init_i2c();
	#ifndef UART_DEBUGGING
	uart1_init();
	#endif
	uart2_init();
    reset_prf();				//SPI 模式芯片复位
    write_reg(TMRL,0x08);    	//设置最大响应时间,单位302us

	while(1)
 	{   
	  #ifndef UART_DEBUGGING
	  //step 1:read uart1 
	  len = uart1_read(buf,MAX_BUFSIZE);


	  //step 2:check it's valid command
	  if(len<=0){
	  	 #ifdef HEART_BEAT
		 HEART_BEAT=0;
		 #endif
	  	 DelayMs(100);
 	  	 #ifdef HEART_BEAT
		 HEART_BEAT=1;
		 DelayMs(100);
		 #endif
		 continue;
	  }else {
  	  	 #ifdef HEART_BEAT
		 HEART_BEAT=0;
		 #endif
	  }
	  #else
	  //just write test command to sam

	  //only for test
	  buf[0]=0xAA; buf[1]=0xAA;  buf[2]=0xAA;  buf[3]=0x96;  buf[4]=0x69;

	  //SAM packet	  

	  //reset sam or rf?
	  //buf[5]=0x00;buf[6]=0x03; buf[7]=0x10; buf[8]=0xff; buf[9]=0xec; len = 10;
	  //get sam id
	  buf[5]=0x00;buf[6]=0x03; buf[7]=0x12; buf[8]=0xff; buf[9]=0xee; len = 10;
	  //find card
	  //buf[5]=0x00;buf[6]=0x03; buf[7]=0x20; buf[8]=0x01; buf[9]=0x22; len = 10;
	  //select card
	  //buf[5]=0x00;buf[6]=0x03; buf[7]=0x20; buf[8]=0x02; buf[9]=0x21; len = 10;
	  //read card
	  //buf[5]=0x00;buf[6]=0x03; buf[7]=0x30; buf[8]=0x01; buf[9]=0x32; len = 10;

	  //vendor private packet
	  //buf[0]=0x2;buf[1]=0x00;buf[2]=0x04;buf[3]=0x32;buf[4]=0x41;buf[5]=0x03;buf[6]=0xe8;buf[7]=0x98;buf[8]=0x03;len=9;

	  #endif

	  prot = packet_protocol(buf,len);
	  //step 3:command dispatch
	  //secure card command 
	  //read prf command
	  //mcu command
	  if(PPROT_SAM==prot){
	  	//just write to secure
		SAM_packet_handler(buf,len);
	  }else if(PPROT_PRIV==prot){
	  	PRIVATE_packet_handler(buf,len);
	  }else {
	  	//just drop ???		
	  }

  	}  	 
}

/*
 * We must parse SAM packet to decide whether we should 
 read SAM or not
*/

void SAM_packet_handler(uint8* buf,int size){
	unsigned char len;
	unsigned char i;
	uart2_write(buf,size);
	/*
	//reuse buf;
	len= read_sec(buf); 	 			//读加密模块
	
	if (len!=0)
	{
		if (buf[0]==0x05)
		{
			close_prf();				 //关闭射频
			for (i=0;i<255;i++);			    
			open_prf();					 //打开射频
			for (i=0;i<255;i++);
        }

	  	EA =0;
		write_prf( buf,len);			 //发送数据
		EA =1;
		len = read_prf(buf);			 //接收身份证反馈的信息
            buf[len]=0x00; 				 //在数据后补一位0，作为CRC校验信息
	  		len =len+1;
			    
	        write_sec(buf,len);			 //写加密模块
	}
	*/
	//read from uart2 (for SAM return)
	//and write it to uart1
	do {
		DelayMs(100);//fine tune?
    	len = uart2_read(buf,MAX_BUFSIZE);
		#ifndef UART_DEBUGGING
		if(len>0)
			uart1_write(buf,len);
		#endif
		#ifdef HEART_BEAT
		HEART_BEAT=0;
		DelayMs(100);
		HEART_BEAT=1;
		#endif
	}while(len>0);

}

void PRIVATE_packet_handler(uint8* buf,int size){
	uint8* cmd_buf = buf+3;	//just skip prefix
	uint8 ret;
	if(size){}
	switch(cmd_buf[0]){
		case CMD_CLASS_COMM:{
			//FIXME
		}break;
		case CMD_CLASS_READER: {
			//FIXME
		}break;
		case CMD_CLASS_CARD: {
			switch(cmd_buf[1]){
				case 0x41: {
					/* card cmd: activate non contact card
					 * FMT: |CLASS[1]|CMD[1]|DELAY[2](0->no wait,0xffff->always wait)|
					 * ANSWER:|STATUS_CODE[2]|TYPE[1](0x0A->Type A,0x0B Type B)|UID[4]|
					*/
					int plen;
					uint16 delaytime_ms = (cmd_buf[2]<<8)+cmd_buf[3]; 
					uint16 *sw;
					//step down delay time to meet host requirement
					if(delaytime_ms!=0xffff) delaytime_ms--;
					//generall uid_len is 4
					close_prf();DelayMs(5);
					open_prf();	DelayMs(5);
					do {
						ret = THM_Anticollision(&comm_buffer[3]);  
					}while((0!=ret)&&(1!=ret)&&(delaytime_ms>0));

					//prepare answer payload
					sw = (unsigned short*)&comm_buffer[0];
					*sw = (!ret)?STATUS_CODE_CARD_NOT_ANSWERED:STATUS_CODE_SUCCESS;
					comm_buffer[2] = (2==ret)?0x00:0x0A;
					//setup answer packet now
					plen = setup_vendor_packet(buf,MAX_BUFSIZE,comm_buffer,7);

					//send answer packet to uart1
					uart1_write(buf,plen); 
				}break;
			}	
		}break;
		case CMD_CLASS_EXT: {
			//FIXME
		}break;

	}
}

