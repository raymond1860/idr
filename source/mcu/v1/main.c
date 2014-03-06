#include <stdio.h>
#include "STC15F2K08S2.h"
#include "spi.h"
#include "secure.h"
#include "common.h"

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
void main()
{
	unsigned char idata buf[160];
	unsigned char len; 	
	unsigned char i;
	init_i2c();
	uart1_init();
	uart2_init();
	AUXR = 0x00;	
    reset_prf();				//SPI 模式芯片复位
    write_reg(TMRL,0x08);    	//设置最大响应时间,单位302us

	while(1)
 	{   	
	  //step 1:read uart1 


	  //step 2:check it's valid command

	  //step 3:command dispatch
	  //secure card command 
	  //read prf command
	  //mcu command

	  //step 4:execute command
	  

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


