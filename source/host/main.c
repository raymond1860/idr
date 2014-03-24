#include "platform.h"
#include "option.h"
#include "IdentityCard.h"

void dump(const char *prefix,const unsigned char *data ,int size)
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



static void dump_id2_info(ID2Info* info){
	printf("=====Dump ID2 info=====\n");
	dump("name",info->name,30);
	dump("sex",info->sex,2);
	dump("native",info->native,4);
	dump("birthday",info->birthday,16);
	dump("address",info->address,70);
	dump("id",info->id,36);
	dump("authority",info->authority,30);
	dump("period_start",info->period_start,16);
	dump("period_end",info->period_end,16);
}


int uart_cli_loop(const char* dev){
	static char cmd[1024]; 
	static char buf[2048];
	int exit=0;
    int ret;
	int started = 0;
	int cmd_len,i;	
	int err;
	char* cmdptr;
    while (!exit) {
        // Display the usage
        printf("\n*******************************************\n");
        printf("Commands:\n"
        "Quit         ---quit\n"        
        "Read         ---read card\n"                		
        "SAMid        ---get card samid\n"
		"ICCid        ---get icc card type and id\n"
         "*******************************************\n\n");

		//print prompt
		printf(">");
        // accept the command
        memset(cmd,0,sizeof(cmd));
		cmdptr = gets(cmd);
		cmd_len = strlen(cmd); 
		if(!cmdptr||!cmd_len) {
			fflush(0);
			continue;
		}
		for (i=0; i<cmd_len;i++)
			cmd[i] = tolower(cmd[i]); 

		str2argv(cmd);
		if(_argc) {
			if(!strncmp(_argv[0],"quit",1)){
	            printf("Quit...\n");
	            exit=1;
	        }else if(!strncmp(_argv[0],"read",1)){	    
				int retlength;
				err = libid2_open(dev);
				if(err){
					printf("open libid2 failed\n");
				}else {

					if (libid2_read_id2_info(buf, &retlength,10) < 0){					
						printf("id2 getinfo failed!\n");
					}else {
						ID2Info info;
						printf("read_id2 return %d\n",retlength);
						dump("id2info",buf,retlength);

						memset(&info,0,sizeof(info));
						ret = libid2_decode_info(buf,retlength,&info);
						if(!ret){
							dump_id2_info(&info);
						}
						
					}
					
					libid2_close();
				}

	        }else if(!strncmp(_argv[0],"samid",3)){
	        	char buf[128];
				int samlength;
		        	
				err = libid2_open(dev);
				if(err){
					printf("open libid2 failed\n");
				}else {
					if(!libid2_getsamid(buf,&samlength,5)){
						printf("samid return %d\n",samlength);					
						dump("samid",buf,samlength);
					}else {
						printf("get samid failed\n");
					}
				
					libid2_close();
				}
	        }else if(!strncmp(_argv[0],"icc",1)){
	        	char iccid[128];
				int samlength;
				int icctype;
	        	//m_getsamid(ctx);
	        	
				err = libid2_open(dev);
				if(err){
					printf("open libid2 failed\n");
				}else {
					if(!libid2_getICCard(1000,&icctype,iccid)){
						printf("icctype %d\n",icctype);					
						dump("iccid",iccid,4);
					}else {
						printf("get iccid failed\n");
					}
				
					libid2_close();
				}
	        }					

				
       	}

		argv_dispose();

    }

	return 0;
	
}

static void usage(const char* program){
    fprintf(stderr, 
        "Usage: %s [-p </dev/ttySx>] [-b <baudrate>] \n"
		"in win32 ,port name should be COMxx like\n", 
        program);
    exit(-1);
}
int main(int argc, char *argv[]) {
	int i;
	char* port="/dev/ttyS1";
    int baudrate = 115200;	

    while (++i < argc){
        if (!strcmp(argv[i],"-p") || !strcmp(argv[i],"--port")){
            if (++i >= argc) goto fail;
            port = argv[i];
            continue;
        }
      
        if (!strcmp(argv[i],"-b") ){
            if (++i >= argc) goto fail;
            baudrate = atoi(argv[i]);
            continue;
        }
        
        fail:
            usage(argv[0]);
    }
    
	printf("Port [%s] will be opened\n",port);

	return uart_cli_loop(port);
}