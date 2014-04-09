/*
 *All Rights Reserved 2014, XingHuo Info Inc,.
*/
#include "spi.h"
#include "secure.h"
#include "common.h"
#include "packet.h"
#include "thm3060.h"



#define MAX_BUFSIZE 120

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
void Delay1us()		//@27.000MHz
{
	unsigned char i;

	_nop_();
	i = 4;
	while (--i);
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


#ifndef ENABLE_BYPASS_MODE
//main function  
void main(void)
{
	unsigned char idata buf[MAX_BUFSIZE];
	int len; 	
	unsigned char prot;
	uint8 i=0;

	init_i2c();
	#ifndef IAP_ENABLED
	uart1_init();
	#endif
	uart2_init();
    reset_prf();				//SPI 模式芯片复位
    write_reg(TMRL,0x08);    	//设置最大响应时间,单位302us

	while(++i<=3){
	  DbgLeds(0xff);
	  DelayMs(100);
	  DbgLeds(0x00);
	  DelayMs(100);
	}

	while(1)
 	{   
 	
	  #ifndef IAP_ENABLED
	  //step 1:read uart1 
	  len = uart1_read(buf,MAX_BUFSIZE);
 	  //buf[0]=0xAA; buf[1]=0xAA;  buf[2]=0xAA;  buf[3]=0x96;  buf[4]=0x69;
	  //buf[5]=0x00;buf[6]=0x03; buf[7]=0x12; buf[8]=0xff; buf[9]=0xee; len = 10;


	  //step 2:check it's valid command
	  if(len<=0){
	  	 DelayMs(100);
		 continue;
	  }
	  DbgLeds(0x00);
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
	  //step 3:command dispatcher
	  if(PPROT_SAM==prot){
		SAM_packet_handler(buf,len);
	  }else if(PPROT_PRIV==prot){
	  	PRIVATE_packet_handler(buf,len);
	  }else {
	  	//just drop ???		
	  }
  	}  	 
}

//See SAM protocol
#define SAM_COMMAND_CLASS_SAM 	0x01
#define SAM_COMMAND_CLASS_ANTI	0x02
#define SAM_COMMAND_CLASS_CARD 	0x04
#define SAM_COMMAND_CLASS_COMM 	0x08

static unsigned char SAM_command_filter(uint8* cmd){
	switch(cmd[7]){
		case 0x10: case 0x11: case 0x12: {
			return SAM_COMMAND_CLASS_SAM;
		}
		case 0x20: {
			return SAM_COMMAND_CLASS_ANTI;
		}
		case 0x30: return SAM_COMMAND_CLASS_CARD;
		//FIXME: when uart baudrate change
		case 0x60: case 0x61: {
			return SAM_COMMAND_CLASS_COMM;
		}
	}
	return 0;
}

void SAM_packet_handler(uint8* buf,int size){
	unsigned char len,totallen;
	unsigned char i;
	unsigned char cmd_filter =SAM_command_filter(buf);

	uart2_write(buf,size);
	//reuse buf;	  
	if(cmd_filter&(SAM_COMMAND_CLASS_ANTI|SAM_COMMAND_CLASS_CARD)){
read_sec_again:
			len= read_sec(buf); 	 			//读加密模块 	
			if (len){
				if (buf[0]==0x05)
				{
					
					close_prf();				 //关闭射频
					Delay1ms();//for (i=0;i<255;i++);			    
					open_prf();					 //打开射频
					Delay1ms();//for (i=0;i<255;i++);
		        }

			  	EA =0;
				write_prf( buf,len);			 //发送数据
				EA =1;
				len = read_prf(buf);			 //接收身份证反馈的信息
		        buf[len++]=0x00; 				 //在数据后补一位0，作为CRC校验信息
					    
			    write_sec(buf,len);			 //写加密模块

				if(cmd_filter&SAM_COMMAND_CLASS_CARD) goto read_sec_again;
					

			}else	goto read_sec_completed;
			
			
	}

read_sec_completed:	
	//read from uart2 (for SAM return)
	//and write it to uart1
	totallen=i=0;
	do {
		DelayMs(10);//fine tune?
    	len = uart2_read(buf,MAX_BUFSIZE);
		#ifndef IAP_ENABLED
		if(len>0){			
			uart1_write(buf,len);
			totallen+=len;
		}
		#endif
		i++;		
	}while(!totallen||i<10||len);


}

void PRIVATE_packet_handler(uint8* buf,int size){
	uint8* cmd_buf = buf+3;	//just skip prefix
	uint16 plen;
	uint16 delaytime_ms;
	uint16 *sw;
	uint8 ret;
	if(size<5) return;
	switch(cmd_buf[0]){
		case CMD_CLASS_COMM:{
			//FIXME
		}break;
		case CMD_CLASS_READER: {
			switch(cmd_buf[1]){
				case READER_SUB_CMD_INFOMATION:{
					sw = (unsigned short*)&comm_buffer[0];
					*sw = STATUS_CODE_SUCCESS;
					comm_buffer[2] = RF_ADAPTER_ID_THM3060;
					plen = setup_vendor_packet(buf,MAX_BUFSIZE,comm_buffer,3);
					//send answer packet to uart1
					uart1_write(buf,plen); 
				}break;
				case READER_SUB_CMD_READ_REG:{
					comm_buffer[2]=read_reg(cmd_buf[2]);
					sw = (unsigned short*)&comm_buffer[0];
					*sw = STATUS_CODE_SUCCESS;					
					plen = setup_vendor_packet(buf,MAX_BUFSIZE,comm_buffer,3);
					//send answer packet to uart1
					uart1_write(buf,plen); 					
				}break;
				case READER_SUB_CMD_WRITE_REG:{
					write_reg(cmd_buf[2],cmd_buf[3]);					
					sw = (unsigned short*)&comm_buffer[0];
					*sw = STATUS_CODE_SUCCESS;					
					plen = setup_vendor_packet(buf,MAX_BUFSIZE,comm_buffer,2);
					//send answer packet to uart1
					uart1_write(buf,plen);
				}break;
			}
		}break;
		case CMD_CLASS_CARD: {
			switch(cmd_buf[1]){
				case CARD_SUB_CMD_ACTIVATE_NON_CONTACT_MEMORY_CARD: {
					/* card cmd: activate non contact card
					 * FMT: |CLASS[1]|CMD[1]|DELAY[2](0->no wait,0xffff->always wait)|
					 * ANSWER:|STATUS_CODE[2]|TYPE[1](0x0A->Type A,0x0B Type B)|UID[4]|
					*/					
					delaytime_ms = (cmd_buf[2]<<8)+cmd_buf[3]; 
					
					//step down delay time to meet host requirement
					if(delaytime_ms!=0xffff) delaytime_ms--;
					//generall uid_len is 4
					do {
						ret = THM_Anticollision(&comm_buffer[3]);  
						//assume this operation is about 5ms
						if(delaytime_ms>10){
							delaytime_ms-=10;
							DelayMs(5);
						}
						else {
							delaytime_ms = 0;
							DelayMs(5);
						}
					}while((0!=ret)&&(1!=ret)&&(delaytime_ms>0));

					//prepare answer payload
					sw = (unsigned short*)&comm_buffer[0];
					*sw = (2==ret)?STATUS_CODE_CARD_NOT_ANSWERED:STATUS_CODE_SUCCESS;
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
		case CMD_CLASS_MCU:{
			 switch(cmd_buf[1]){
			 	case MCU_SUB_CMD_RESET:{
					uint8 xdata resettype;
					uint8 xdata delay_s;
					//first param is reset type,
					resettype = cmd_buf[2];
					//second param is delay time to execute reset
					delay_s = cmd_buf[3];
					sw = (unsigned short*)&comm_buffer[0];
					*sw = STATUS_CODE_SUCCESS;
					//setup answer packet now
					plen = setup_vendor_packet(buf,MAX_BUFSIZE,comm_buffer,2);
					//send answer packet
					uart1_write(buf,plen);
					//delay specified time to execute reset 
					while(delay_s--)
						DelayMs(1000);
					if(MCU_RESET_TYPE_ISP==resettype)
						IAP_CONTR = 0x60; //软件复位,系统重新从ISP代码区开始运行程序
					else
						IAP_CONTR = 0x20;//软件复位,系统重新从用户代码区开始运行程序

					//infinite loop to waiting for reset 
					while(1){
						DbgLeds(0x00);
						DelayMs(100);
						DbgLeds(0xff);
						DelayMs(100);
					} 
						
				}break;	
				case MCU_SUB_CMD_FIRMWARE_VERSION:{
					sw = (unsigned short*)&comm_buffer[0];
					*sw = STATUS_CODE_SUCCESS;
					comm_buffer[2] = FIRMWARE_VERSION;
					//setup answer packet now
					plen = setup_vendor_packet(buf,MAX_BUFSIZE,comm_buffer,3);
					//send answer packet
					uart1_write(buf,plen);
				}break;
			 }
		}break;

	}
}

#else
#define __enable_T1()		TR1 =  1
#define __disable_T1()  	TR1 =  0

#define __enable_UART_interrupt()		ES = 1; __enable_T1()
#define __disable_UART_interrup()	    ES = 0; __disable_T1()

//initial seril interrupt 
void initial_serial()
{
    SCON  = 0x50;                   	/* SCON: mode 1, 8-bit UART, enable rcvr    */
    PCON  = 0X80;                       /* SMOD =0, K=1;SMOD =1, K=2;               */
    PS=1;                               /* COMM HIGH PRIORITY                       */
    TMOD  = 0x21;                   	/* TMOD: timer 1, mode 2, 8-bit reload      */
    TH1   =0x8b;//ff;// 0xff;;                    	/* TH1:  reload value for 115200 baud @22.1184MHz */
    TL1   =0x8b;//ff;//0xff;                    	/* Tl1:  reload value for 115200 baud @22.1184MHz */ 
    __enable_T1();                  	/* TR1:  timer 1 run */
	TI = 1;

}


void main()
{
	unsigned char idata buf[MAX_BUFSIZE];
	unsigned char len; 	
	unsigned char i;
	init_i2c();
	AUXR = 0x00;	
	initial_serial();	  
    reset_prf();				//SPI 模式芯片复位
    write_reg(TMRL,0x08);    	//设置最大响应时间,单位302us
	//P14=0;

	while(1)
 	{   	
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

  	}  	 
}

#endif


#ifdef ENABLE_LED_DEBUG
void DbgLeds(uint8 led){
	#ifdef LED_DBG_BIT1
	LED_DBG_BIT1=led&0x1?0:1;
	#endif
	#ifdef LED_DBG_BIT2
	LED_DBG_BIT2=led&0x2?0:1;
	#endif
	#ifdef LED_DBG_BIT3
	LED_DBG_BIT3=led&0x4?0:1;
	#endif
	#ifdef LED_DBG_BIT4
	LED_DBG_BIT4=led&0x8?0:1;
	#endif
	#ifdef LED_DBG_BIT5
	LED_DBG_BIT5=led&0x10?0:1;
	#endif
	#ifdef LED_DBG_BIT6
	LED_DBG_BIT6=led&0x20?0:1;
	#endif
	#ifdef LED_DBG_BIT7
	LED_DBG_BIT7=led&0x40?0:1;
	#endif
	#ifdef LED_DBG_BIT8
	LED_DBG_BIT8=led&0x80?0:1;
	#endif				 
}
#endif

