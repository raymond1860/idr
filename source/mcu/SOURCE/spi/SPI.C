/////////////////////////////////////////////////////////////////////////////////
//
//           
//        T H M 3 0 6 0      S P I   I N T E R F A C E   P R O G R A M  
//
// Project:           THM3060 DEMO
//
//
// resource usage:
//
// history:
//                    Created by DingYM 2009.08.11
// 
// note:
//          
// 			
//   (C)TONGFANG  Electronics  2009.08   All rights are reserved. 
//
/////////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include <intrins.h>
#include "..\at89x52.h"
#include "spi.h"


// 此处用户可以根据自己应用进行修改
#define SS_N       P1_0
#define SPI_CLK    P1_1	   
#define MOSI	   P1_2
#define MISO       P1_3
#define POR        P1_5	   



// THM3060 寄存器地址
#define DATA       0
#define PSEL	   1
#define FCONB	   2
#define EGT		   3
#define CRCSEL	   4
#define RSTAT	   5
#define SCON_60	   6
#define INTCON	   7
#define RSCH       8
#define RSCL	   9
#define CRCH	   0X0A
#define CRCL	   0X0B
#define TMRH	   0X0C
#define TMRL	   0X0D
#define BPOS	   0X0E
#define SMOD	   0X10
#define PWTH	   0X11




//SPI 模式芯片复位
void reset_prf()
{

	SPI_CLK =0;	     //保持 SPI_CLK 常态为低
	SS_N   = 1;		 //SPI 从机片选，低有效
	POR = 0;		 //复位
	POR = 1;  

}
	

//发送一个字节

static void send_byte(unsigned char dat)
{
   unsigned char i;
   for (i =0;i<8;i++)
   {
		SPI_CLK = 0;
		 _nop_();
		 _nop_();
		 _nop_();
		 _nop_();
		if (dat & 0x80)
			MOSI = 1;
		 else
		 	MOSI = 0;
		 dat = dat << 1;
	     SPI_CLK =1;
		 _nop_();
		 _nop_();
		 _nop_();
		 _nop_();

	   }
   SPI_CLK =0;
}

// 接收一个字节
static unsigned char recv_byte()
{
    unsigned char i,dat,temp;
	SPI_CLK =0;
	dat =0; temp =0x80;
	for (i=0;i<8;i++)
	{
		SPI_CLK =1;
	   	 _nop_();
		 _nop_();
		 _nop_();
		 _nop_();
		if ( MISO)
			dat|= temp;					    
		SPI_CLK =0;
		temp >>= 1;		
		 _nop_();
		 _nop_();
		 _nop_();
		 _nop_();
	}
	return dat;
}

// 通过SPI 总线向 THM3060 写入 num 长度个字节
static void send_buff(unsigned char *buf,unsigned int num)
{
 	if ((buf== NULL)||(num ==0)) return;
	while( num--)
	{
	 	send_byte(*buf++);
	}  
}	   

// 通过SPI 总线向 THM3060 读出 num 长度个字节
static void receive_buff(unsigned char *buf,unsigned int num)
{
	if ((buf== NULL)||(num ==0)) return;
	while(num--)
	{
		*buf++ = recv_byte();	
	 }
}


// 通过 SPI 总线读 address 寄存器的值
extern unsigned char read_reg(unsigned char address)
{
	unsigned char temp_buff[1];
	SS_N = 0;
	temp_buff[0] = address & 0x7F;	
	send_buff(temp_buff,1);
	receive_buff(temp_buff,1);
	SS_N = 1;
	return(temp_buff[0]);
}

// 通过 SPI 总线写 address 寄存器的值
extern void write_reg(unsigned char address,unsigned char content)
{
	unsigned char temp_buff[2];
	temp_buff[0] = address | 0x80;
	temp_buff[1] = content;
	SS_N = 0;
	send_buff(temp_buff,2);
	SS_N = 1;
}

/////////////////////////////////////////////////////////////////////////////
//
//   下面程序用户需要调用
//
/////////////////////////////////////////////////////////////////////////////


// SPI 模式读出接收数据,数据存在缓冲区 buffer 中，返回值为收到数据

extern unsigned int read_prf(unsigned char *buffer)
{
	unsigned char temp;
	unsigned int num =0;
	
	temp = read_reg(RSTAT);
	    
	while( (temp &0x80) != 0x80 )
	{  	  	
	temp = read_reg(RSTAT);
	}
	
	if(temp&0x01)
	{
		    
			 	//计算长度	
        	num = (unsigned int)read_reg(RSCL)+ ((unsigned int)(read_reg(RSCH))<<8);
			if(num==0)return (num); 
			temp = 0x00;              //读取 DAT_REG寄存器命令
			SS_N = 0;	
			send_buff(&temp,1);       //发送读取命令
			receive_buff(buffer,num); //读取数据
			SS_N = 1;
		
	}
	else
	{
		num =0;	
			
	}
	return num;

}

// 发送数据子程序		 
extern void write_prf(unsigned char *buffer,unsigned int num)
{
	unsigned char temp;	
	write_reg(SCON_60,0x5);	 //PTRCLR =1,CARRYON =1
	write_reg(SCON_60,0x01);    //PTRCLR=0;
	temp = 0x80;			 //写数据寄存器命令 
	SS_N = 0;
	send_buff(&temp,1);
	send_buff(buffer,num);	 //写入数据
	SS_N = 1;

	write_reg(SCON_60,0x03);    // STARTSEND =1 ，开始发送	



}


//打开射频
extern void open_prf()
{
    unsigned short i;  	
	write_reg(SCON_60,0x01);
	for (i=0;i<10000;i++);
	
}

	
//关闭射频
extern void close_prf()
{
    unsigned short i;  	
	write_reg(SCON_60,0x00);
	for (i=0;i<10000;i++);
		
}

