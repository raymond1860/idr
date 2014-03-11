#include "common.h"
#include "STC15F2K08S2.h"

#define INC_PTR(e,max)	\
	if((++e) >= max) e = 0;		  


#define RCV_BUF_SIZE 128
#define SND_BUF_SIZE 128

static unsigned char xdata  uart1_read_buf[RCV_BUF_SIZE];
static unsigned char xdata  uart1_write_buf[SND_BUF_SIZE];
static unsigned char data uart1_write_in,uart1_write_out;
static unsigned char data uart1_read_in,uart1_read_out;
static bit uart1_lastchar = 1;	

void uart1_init(void)	  //115200bps@13.56MHz
{
	SCON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
	AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
	TMOD &= 0x0F;		//设定定时器1为16位自动重装方式
	TL1 = 0xE3;		//设定定时初值
	TH1 = 0xFF;		//设定定时初值
	ET1 = 0;		//禁止定时器1中断
	TR1 = 1;		//启动定时器1

	ES = 1;
	EA = 1;
	uart1_write_in = uart1_write_out = 0;
	uart1_read_in  = uart1_read_out = 0;
}



static void serial_interrupt(void) interrupt 4  using 1
{   
   unsigned char  data c;	   
   	    
	if(RI)  /* Receive mode */
	{
		RI = 0;
        c = SBUF;
		uart1_read_buf[uart1_read_in] = SBUF;
		INC_PTR(uart1_read_in,RCV_BUF_SIZE);	
	}
	if(TI)
	{
		TI = 0;
		if(uart1_write_in != uart1_write_out)
		{
			SBUF = uart1_write_buf[uart1_write_out];
			INC_PTR(uart1_write_out,SND_BUF_SIZE);
			uart1_lastchar = 0;
		}
		else
		{
			uart1_lastchar = 1;
		}							 
	}
		
  
}

void uart1_writechar(char c){
	uint8 temp;
	temp = uart1_write_in;
	INC_PTR(temp,SND_BUF_SIZE);	
	while (temp == uart1_write_out);
	uart1_write_buf[uart1_write_in] = c;
	uart1_write_in = temp;
	
	if(uart1_lastchar){
        uart1_lastchar = 0;
		SBUF = uart1_write_buf[uart1_write_out];
		INC_PTR(uart1_write_out,SND_BUF_SIZE);
	}
}

void uart1_write(const char* buf,int size){
	int i=0;
	while(i < size){
		uart1_writechar(buf[i++]);
	}
}
char uart1_readchar(){
	char c;
	while(uart1_read_in == uart1_read_out) ;
    c = uart1_read_buf[uart1_read_out];
    INC_PTR(uart1_read_out,RCV_BUF_SIZE);
	return(c);
}
//return read char count
int uart1_read(char* buf,int max_size){
	int ret=0;
	if(uart1_read_in != uart1_read_out) {
		do {
			buf[ret++]=uart1_read_buf[uart1_read_out];
			INC_PTR(uart1_read_out,RCV_BUF_SIZE);			
		}while((ret<max_size)&&uart1_read_in != uart1_read_out);
	}
	return ret;
}



static unsigned char xdata  uart2_read_buf[RCV_BUF_SIZE];
static unsigned char xdata  uart2_write_buf[SND_BUF_SIZE];
static unsigned char data uart2_write_in,uart2_write_out;
static unsigned char data uart2_read_in,uart2_read_out;
static bit uart2_lastchar = 1;	

#define S2RI 0x01
#define S2TI 0x02
#define S2RB8 0x04
#define S2TB8 0x08
void uart2_init()	  //115200bps@13.56MHz
{
	S2CON = 0x50;		//8位数据,可变波特率
	AUXR |= 0x04;		//定时器2时钟为Fosc,即1T
	T2L = 0xE3;		//设定定时初值
	T2H = 0xFF;		//设定定时初值
	AUXR |= 0x10;		//启动定时器2

	IE2 = 0x1;
	EA = 1;

	uart2_write_in = uart2_write_out = 0;
	uart2_read_in  = uart2_read_out = 0;

}

static void serial2_interrupt(void) interrupt 8  using 1
{   
   unsigned char  data c;	   
   	    
	if(S2CON&S2RI)  /* Receive mode */
	{
		S2CON &=~S2RI;
        c = S2BUF;
		uart2_read_buf[uart2_read_in] = S2BUF;
		INC_PTR(uart2_read_in,RCV_BUF_SIZE);	
	}
	if(S2CON&S2TI)
	{
		S2CON&=~S2TI;
		if(uart2_write_in != uart2_write_out)
		{
			SBUF = uart1_write_buf[uart1_write_out];
			INC_PTR(uart2_write_out,SND_BUF_SIZE);
			uart2_lastchar = 0;
		}
		else
		{
			uart2_lastchar = 1;
		}							 
	}
		
  
}

void uart2_writechar(char c){
	uint8 temp;
	temp = uart2_write_in;
	INC_PTR(temp,SND_BUF_SIZE);	
	while (temp == uart2_write_out);
	uart2_write_buf[uart2_write_in] = c;
	uart2_write_in = temp;
	
	if(uart2_lastchar){
        uart2_lastchar = 0;
		SBUF = uart2_write_buf[uart2_write_out];
		INC_PTR(uart2_write_out,SND_BUF_SIZE);
	}
}

void uart2_write(const char* buf,int size){
	int i=0;
	while(i < size){
		uart2_writechar(buf[i++]);
	}
}

char uart2_readchar(){
	char c;
	while(uart2_read_in == uart2_read_out) ;
    c = uart2_read_buf[uart2_read_out];
    INC_PTR(uart2_read_out,RCV_BUF_SIZE);
	return(c);
}
int uart2_read(char* buf,int max_size){
	int ret=0;
	if(uart2_read_in != uart2_read_out) {
		do {
			buf[ret++]=uart2_read_buf[uart2_read_out];
			INC_PTR(uart2_read_out,RCV_BUF_SIZE);			
		}while((ret<max_size)&&uart2_read_in != uart2_read_out);

	}
	return ret;
}


