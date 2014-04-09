#include "platform.h"

void dumpdata(const char *prefix,const unsigned char *data ,int size)
{
#ifndef isprint
#define isprint(c)	(c>='!'&&c<='~')
#endif
//((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
	char* ptr;
	static char digits[2048] = {0};
	int i, j;	
	unsigned char *buf = (unsigned char*)data;
	fprintf(stdout,"%s[%d]\n",
			   prefix?prefix:" ", size);

	
	for (i=0; i<size; i+=16) 
	{
	  ptr = &digits[0];
	  ptr+=sprintf(ptr,"%06x: ",i);
	  for (j=0; j<16; j++) 
		if (i+j < size)
		 ptr+=sprintf(ptr,"%02x ",buf[i+j]);
		else
		 ptr+=sprintf(ptr,"%s","   ");

	  ptr+=sprintf(ptr,"%s","  ");
		
	  for (j=0; j<16; j++) 
		if (i+j < size)			
			ptr+=sprintf(ptr,"%c",isprint(buf[i+j]) ? buf[i+j] : '.');
	  *ptr='\0';
	  fprintf(stdout,"%s\n",digits);
	}
}

