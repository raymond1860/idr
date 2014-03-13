#include <stdio.h>
#include <string.h>
#include "STC15F2K08S2.h"
#include "spi.h"
#include "secure.h"
#include "common.h"
#include "packet.h"
#include "thm3060.h"

#define HEART_BEAT P20

#define MAX_BUFSIZE 120

unsigned char xdata comm_buffer[64],comm_buffer2[64];	


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
	GenPacket p;
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
	uart1_init();
	uart2_init();
	AUXR = 0x00;	
    reset_prf();				//SPI 模式芯片复位
    write_reg(TMRL,0x08);    	//设置最大响应时间,单位302us

	while(1)
 	{   	
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
	  }
	  p.pbuf = buf;
	  p.plen = len;
	  prot = packet_protocol(&p);
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


void SAM_packet_handler(uint8* buf,int size){
	unsigned char len;
	unsigned char i;
	uart2_write(buf,size);
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

	//read from uart2 (for SAM return)
	//and write it to uart1
	do {
		DelayMs(10);//fine tune?
    	len = uart2_read(buf,MAX_BUFSIZE);
		if(len>0)
			uart1_write(buf,len);
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
					uint8 anti_flag;
					uint16 delaytime_ms = (cmd_buf[2]<<8)+cmd_buf[3]; 
					uint16 uid_len=0;
					uint16 *sw;
					//step down delay time to meet host requirement
					if(delaytime_ms!=0xffff) delaytime_ms--;
					//generall uid_len is 4
					close_prf();DelayMs(5);
					open_prf();	DelayMs(5);
					do {
						ret = THM_Anticollision2(&anti_flag,&uid_len,comm_buffer);  
					}while(!ret&&(delaytime_ms>0));

					//prepare answer payload
					sw = (unsigned short*)&comm_buffer2[0];
					*sw = (!ret)?STATUS_CODE_CARD_NOT_ANSWERED:STATUS_CODE_SUCCESS;
					comm_buffer2[2] = (ret)?0x0A:0x00;
					if(uid_len>0)
						memcpy(&comm_buffer2[3],comm_buffer,uid_len);

					//setup answer packet now
					plen = setup_vendor_packet(buf,MAX_BUFSIZE,comm_buffer2,uid_len+3);

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

