#ifndef __SPI_H__
#define __SPI_H__   
	extern unsigned short FWT;
	extern unsigned char read_reg(unsigned char address);
	extern void write_reg(unsigned char address,unsigned char content);
	extern unsigned int read_prf(unsigned char *buffer);
	extern void write_prf(unsigned char *buffer,unsigned int num);

	extern void init_prf();			
	extern void open_prf();		
	extern void close_prf();
	extern void	reset_prf();

	extern void THM_SendFrame(unsigned char *buffer,unsigned short num);
	extern unsigned char THM_WaitReadFrame(unsigned short *len, unsigned char *buffer);
#endif
