#include <stdio.h>
#include <string.h>
#include "STC15F2K08S2.h"

#define HEART_BEAT P20

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
int main(void)
{					   	
	#ifdef HEART_BEAT
	while(1){
	HEART_BEAT=0;
	DelayMs(100);
	HEART_BEAT=1;
	DelayMs(100);
	}
	#endif
	return 0;
}