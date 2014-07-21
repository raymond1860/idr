#include "platform.h"
#include "option.h"
#include "IdentityCard.h"
#include "download.h"
#include "packet.h"
#include "config.h"

//simple utility
static unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned long result = 0,value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

static long simple_strtol(const char *cp,char **endp,unsigned int base)
{
	if(*cp=='-')
		return -simple_strtoul(cp+1,endp,base);
	return simple_strtoul(cp,endp,base);
}

static int ustrtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned long result = simple_strtoul(cp, endp, base);
	switch (**endp) {
	case 'G' :
		result *= 1024;
		/* fall through */
	case 'M':
		result *= 1024;
		/* fall through */
	case 'K':
	case 'k':
		result *= 1024;
		if ((*endp)[1] == 'i') {
			if ((*endp)[2] == 'B')
				(*endp) += 3;
			else
				(*endp) += 2;
		}
	}
	return result;
}

//string to hex
//translate 050000 ->0x05 0x00 0x00 in hex array
//return element number
static unsigned int str2hex(unsigned char *str,unsigned char *hex)
{
    unsigned char ctmp, ctmp1,half;
    unsigned int num=0;
    do{
            do{
                    half = 0;
                    ctmp = *str;
                    if(!ctmp) break;
                    str++;
            }while((ctmp == 0x20)||(ctmp == 0x2c)||(ctmp == '\t'));
            if(!ctmp) break;
            if(ctmp>='a') ctmp = ctmp -'a' + 10;
	            else if(ctmp>='A') ctmp = ctmp -'A'+ 10;
	            else ctmp=ctmp-'0';
            ctmp=ctmp<<4;
            half = 1;
            ctmp1 = *str;
            if(!ctmp1) break;
            str++;
            if((ctmp1 == 0x20)||(ctmp1 == 0x2c)||(ctmp1 == '\t'))
            {
                    ctmp = ctmp>>4;
                    ctmp1 = 0;
            }
            else if(ctmp1>='a') ctmp1 = ctmp1 - 'a' + 10;
	            else if(ctmp1>='A') ctmp1 = ctmp1 - 'A' + 10;
	            else ctmp1 = ctmp1 - '0';
	            ctmp += ctmp1;
            *hex = ctmp;
            hex++;
            num++;
     }while(1);
     if(half)
     {
            ctmp = ctmp>>4;
            *hex = ctmp;
            num++;
     }
     return(num);

}


struct cli_menu;
typedef int (*menu_func)(struct cli_menu* cli);
typedef struct cli_menu{
	struct cli_menu* parent;
	struct cli_menu* children;
	struct cli_menu* sibling;
	char* menuname;
	menu_func func;
}cli_menu;


cli_menu_shared shared;

static cli_menu menu_top;
static cli_menu menu_sam;
static cli_menu menu_mcu;
static cli_menu menu_reader;

static void init_menu(cli_menu* parent,cli_menu* menu,char* menuname,menu_func func){
	cli_menu* it;
	if(NULL!=parent->children){
		it=parent->children;
		while(NULL!=it->sibling) it=it->sibling;
		it->sibling = menu;
	}else
		parent->children = menu;

	menu->parent = parent;
	menu->func = func;
	menu->menuname = menuname;
	menu->children=menu->sibling=NULL;
}

static int exec_menu(cli_menu* cli,char* shoot){
	cli_menu* it=cli;
	if(NULL!=it->children){
		it=it->children;
		if(!strcmp(it->menuname,shoot))
			return it->func(it);		
		it = it->sibling;
		while(NULL!=it){
			if(!strcmp(it->menuname,shoot))
				return it->func(it);
			it = it->sibling;
		}
	}
	return 0;
}

static void show_menu_help(cli_menu* cli,char* help){
	char* menu_helps[16];
	int menu_level=0;
	cli_menu* it=cli;
	memset(menu_helps,0,sizeof(menu_helps)/sizeof(menu_helps[0]));
	//iterate to top
	do{
		if(it->menuname)
			menu_helps[menu_level++]=it->menuname;
		it = it->parent;
	}while(it);
	printf("\n\n");
	for(;menu_level>0;menu_level--){
		printf("\%s",menu_helps[menu_level-1]);
	}
	printf("\n*******************************************\n");
	printf("Available Commands:\n"
		"quit               ---exit whole program\n"
		"exit               ---exit whole program\n");
	if(cli->parent)
		printf("back or ..         ---back to parent menu\n");	
	printf("\n*******************************************\n");
	//show sub menus
	it = cli;
	if(NULL!=it->children){
		it=it->children;		
		if(it->menuname)
			printf("%s                ---Enter sub menu %s\n",it->menuname,it->menuname);
		it = it->sibling;
		while(NULL!=it){
			if(it->menuname)
				printf("%s                ---Enter sub menu %s\n",it->menuname,it->menuname);
			it = it->sibling;
		}
	}
	
	//print current menu help
	if(help)
		printf("%s",help);

	//print prompt
	printf("\n\n>");
}

static char* fetch_seq_cmd(cli_menu_shared * ms){
	char* p;
	char* curcmd;
	if(ms->seqcmd&&*ms->seqcmd!='\0'){
		curcmd=p=ms->seqcmd;
		while(*p!='\0'&&*p!=';') p++;
		if(*p==';') {
			*p='\0';
			ms->seqcmd=++p;
			printf("%s\n",curcmd);
			return curcmd;
		}
		if(*p=='\0'){
			ms->seqcmd=NULL;			
			printf("%s\n",curcmd);
			return curcmd;
		}
	}

	return NULL;
	
}

static void dump_id2_info(ID2Info* info){
	printf("=====Dump ID2 info=====\n");
	dumpdata("name",info->name,30);
	dumpdata("sex",info->sex,2);
	dumpdata("native",info->native,4);
	dumpdata("birthday",info->birthday,16);
	dumpdata("address",info->address,70);
	dumpdata("id",info->id,36);
	dumpdata("authority",info->authority,30);
	dumpdata("period_start",info->period_start,16);
	dumpdata("period_end",info->period_end,16);
}

int cli_loop_mcu(cli_menu* cli){
	static char cmd[1024]; 
	static char buf[2048];
	int exit=0;
    int ret;
	int started = 0;
	int cmd_len,i;	
	int err;
	char* cmdptr;
	const char* dev = shared.devname;
    while (!exit) {
        // Display the usage
        show_menu_help(cli,
		"reset [isp]        ---reset mcu normal or into isp mode\n"        
        "ver                ---read mcu firmware version\n"  
        "fw [firmware]      ---download firmware to mcu with manual reset\n"
        "autofw [firmware]  ---download firmware to mcu with auto reset(mcu feature required)\n");

        // accept the command
        memset(cmd,0,sizeof(cmd));
		if(cmdptr=fetch_seq_cmd(&shared)){
			strcpy(cmd,cmdptr);
			cmd_len = strlen(cmd);
		}else {
			cmdptr = gets(cmd);
			cmd_len = strlen(cmd); 
			if(!cmdptr||!cmd_len) {
				fflush(0);
				continue;
			}
		}
		for (i=0; i<cmd_len;i++)
			cmd[i] = tolower(cmd[i]); 

		str2argv(cmd);
		if(_argc) {
			if(!strncmp(_argv[0],"reset",5)){
				uint8 resettype=MCU_RESET_TYPE_NORMAL;
				if(_argc>1&&!strncmp(_argv[1],"isp",3))
					resettype=MCU_RESET_TYPE_ISP;
				err = xfer_packet_wrapper(dev,buf,64,CMD_CLASS_MCU,MCU_SUB_CMD_RESET,2,resettype,0);
				if(!err&&STATUS_CODE(buf)==STATUS_CODE_SUCCESS){
					printf("reset mcu okay\n");
				}else {
					printf("reset mcu failed\n");
				}
	        }else if(!strncmp(_argv[0],"ver",3)){	
				err = xfer_packet_wrapper(dev,buf,64,CMD_CLASS_MCU,MCU_SUB_CMD_FIRMWARE_VERSION,0);
				if(!err&&STATUS_CODE(buf)==STATUS_CODE_SUCCESS){
					printf("mcu version=%2x\n",*(PACKET_RESP(buf)));
				}else {
					printf("fetch mcu version failed\n");
				}
	        }else if(!strncmp(_argv[0],"fw",4)){
	        	char* firmware_name = "idr.hex";
	        	if(_argc>1){
					firmware_name = _argv[1];
	        	}
				printf("download firmware %s to mcu\n",firmware_name);
				err = download_firmware(dev,firmware_name);
				printf("\n\nresult=%s\n",download_error_code2string(err));
	        }else if(!strncmp(_argv[0],"autofw",4)){
	        	char* firmware_name = "idr.hex";
	        	if(_argc>1){
					firmware_name = _argv[1];
	        	}
				printf("download firmware %s to mcu\n",firmware_name);
				err=download_firmware_all_in_one(dev,firmware_name);
				printf("\n\nresult = %s\n",download_error_code2string(err));
	        }else if(!strncmp(_argv[0],"back",1)||!strncmp(_argv[0],"..",2)){
				printf("back to parent menu...\n");
				exit=1;
			}if(!strncmp(_argv[0],"exit",4)||!strncmp(_argv[0],"quit",1)){
				printf("exit program...\n");
				platform_program_exit(0);
			}else{
	        	char* newmenu = strdup(_argv[0]);
				argv_dispose();
	        	exec_menu(cli,newmenu);
				free(newmenu);
			}						

				
       	}

		argv_dispose();

    }

	return 0;
	
}

int cli_loop_sam(cli_menu* cli){
	static char cmd[1024]; 
	static char buf[2048];
	int exit=0;
    int ret;
	int started = 0;
	int cmd_len,i;	
	int err;
	char* cmdptr;
	
	const char* dev = shared.devname;
    while (!exit) {
        // Display the usage
        
        show_menu_help(cli,
		"reset               ---reset sam\n"        
        "info                ---read card info\n"                		
        "sam                 ---get sam id\n"
		"icc                 ---get icc card type and id\n"
		"idsam               ---get id2 card  id\n"
		"fc                  ---find card\n"
		"sc                  ---select card\n"
		"rc                  ---read card\n"
		"cc                  ---combo find+select+read card\n"		
         "*******************************************\n\n");
        // accept the command
        memset(cmd,0,sizeof(cmd));
		if(cmdptr=fetch_seq_cmd(&shared)){
			strcpy(cmd,cmdptr);
			cmd_len = strlen(cmd);
		}else {
			cmdptr = gets(cmd);
			cmd_len = strlen(cmd); 
			if(!cmdptr||!cmd_len) {
				fflush(0);
				continue;
			}
		}
		for (i=0; i<cmd_len;i++)
			cmd[i] = tolower(cmd[i]); 

		str2argv(cmd);
		if(_argc) {
			if(!strncmp(_argv[0],"reset",5)){				
	        	err = libid2_open(dev);
				if(err){
					printf("open libid2 failed\n");
				}else{
					ret=libid2_reset();
					printf("reset sam %s\n",ret<0?"failed":"okay");
					libid2_close();
				}
	        }else if(!strncmp(_argv[0],"fc",2)){
	        	port_property prop;
		        mcu_xfer xfer;
				memset(&prop,0,sizeof(port_property));
				prop.port_setting.serial.baudrate = shared.baudrate;
				xfer.xfer_impl = NULL;	
				xfer.req = "\xAA\xAA\xAA\x96\x69\x00\x03\x20\x01\x22";
				xfer.xfer_to = DEF_XFER_TIMEOUT_MS;
				xfer.reqsize = 10;
				xfer.resp = buf;
				xfer.respsize = 2048;
				xfer.xfer_type = XFER_TYPE_OUT_IN;
				err = submit_xfer(dev,&prop,&xfer);
				printf("find card result %s\n",!err?"okay":"failure");
				if(!err&&xfer.respsize){
					printf("SAM result %s\n",
						(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS)?"success":
						(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS2)?"success":"failed");
					dumpdata("return data",xfer.resp,xfer.respsize);
				}
	        	
	        }else if(!strncmp(_argv[0],"sc",2)){
	        	port_property prop;
		        mcu_xfer xfer;
				memset(&prop,0,sizeof(port_property));
				prop.port_setting.serial.baudrate = shared.baudrate;
				xfer.xfer_impl = NULL;		
				xfer.req = "\xAA\xAA\xAA\x96\x69\x00\x03\x20\x02\x21";				
				xfer.xfer_to = DEF_XFER_TIMEOUT_MS;
				xfer.reqsize = 10;
				xfer.resp = buf;
				xfer.respsize = 2048;
				xfer.xfer_type = XFER_TYPE_OUT_IN;
				err = submit_xfer(dev,&prop,&xfer);
				printf("select card result %s\n",!err?"okay":"failure");
				if(!err&&xfer.respsize){					
					printf("SAM result %s\n",
						(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS)?"success":
						(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS2)?"success":"failed");
					dumpdata("return data",xfer.resp,xfer.respsize);
				}
	        	
	        }else if(!strncmp(_argv[0],"rc",2)){
	        	port_property prop;
		        mcu_xfer xfer;
				memset(&prop,0,sizeof(port_property));
				prop.port_setting.serial.baudrate = shared.baudrate;
				xfer.xfer_impl = NULL;			
				xfer.req = "\xAA\xAA\xAA\x96\x69\x00\x03\x30\x01\x32";				
				xfer.xfer_to = DEF_XFER_TIMEOUT_MS;
				xfer.reqsize = 10;
				xfer.resp = buf;
				xfer.respsize = 2048;
				xfer.xfer_type = XFER_TYPE_OUT_IN;
				err = submit_xfer(dev,&prop,&xfer);
				printf("read card result %s\n",!err?"okay":"failure");
				if(!err&&xfer.respsize){					
					printf("SAM result %s\n",
						(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS)?"success":
						(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS2)?"success":"failed");
					dumpdata("return data",xfer.resp,xfer.respsize);
				}
	        	
	        }else if(!strncmp(_argv[0],"cc",2)){
	        	port_property prop;
		        mcu_xfer xfer;
				memset(&prop,0,sizeof(port_property));
				prop.port_setting.serial.baudrate = shared.baudrate;
				xfer.xfer_impl = NULL;	
				xfer.req = "\xAA\xAA\xAA\x96\x69\x00\x03\x20\x01\x22";
				xfer.xfer_to = DEF_XFER_TIMEOUT_MS;
				xfer.reqsize = 10;
				xfer.resp = buf;
				xfer.respsize = 2048;
				xfer.xfer_type = XFER_TYPE_OUT_IN;
				err = submit_xfer(dev,&prop,&xfer);
				printf("find card result %s\n",!err?"okay":"failure");
				if(!err){
					if(xfer.respsize){						
						printf("SAM result %s\n",
							(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS)?"success":
							(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS2)?"success":"failed");
						dumpdata("return data",xfer.resp,xfer.respsize);
					}					
					memset(&prop,0,sizeof(port_property));
					prop.port_setting.serial.baudrate = shared.baudrate;
					xfer.xfer_impl = NULL;		
					xfer.req = "\xAA\xAA\xAA\x96\x69\x00\x03\x20\x02\x21";				
					xfer.xfer_to = DEF_XFER_TIMEOUT_MS;
					xfer.reqsize = 10;
					xfer.resp = buf;
					xfer.respsize = 2048;
					xfer.xfer_type = XFER_TYPE_OUT_IN;
					err = submit_xfer(dev,&prop,&xfer);
					printf("select card result %s\n",!err?"okay":"failure");
					if(!err){						
						printf("SAM result %s\n",
							(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS)?"success":
							(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS2)?"success":"failed");
						if(xfer.respsize){
							dumpdata("return data",xfer.resp,xfer.respsize);
						}

						memset(&prop,0,sizeof(port_property));
						prop.port_setting.serial.baudrate = shared.baudrate;
						xfer.xfer_impl = NULL;			
						xfer.req = "\xAA\xAA\xAA\x96\x69\x00\x03\x30\x01\x32";				
						xfer.xfer_to = DEF_XFER_TIMEOUT_MS;
						xfer.reqsize = 10;
						xfer.resp = buf;
						xfer.respsize = 2048;
						xfer.xfer_type = XFER_TYPE_OUT_IN;
						err = submit_xfer(dev,&prop,&xfer);
						printf("read card result %s\n",!err?"okay":"failure");
						if(!err&&xfer.respsize){							
							printf("SAM result %s\n",
								(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS)?"success":
								(STATUS_CODE_SAM(xfer.resp)==STATUS_CODE_SAM_SUCCESS2)?"success":"failed");
							dumpdata("return data",xfer.resp,xfer.respsize);
						}
						
					}
					
				}
	        	
	        }else if(!strncmp(_argv[0],"info",4)){	    
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
						dumpdata("id2info",buf,retlength);

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
					if(!libid2_getsamid(buf,&samlength,5/*timeout*/)){
						printf("samid[%s]\n",buf);
					}else {
						printf("get samid failed\n");
					}
				
					libid2_close();
				}
	        }else if(!strncmp(_argv[0],"icc",3)){
	        	char iccid[128];
				int samlength;
				int icctype;
	        	//m_getsamid(ctx);
	        	
				err = libid2_open(dev);
				if(err){
					printf("open libid2 failed\n");
				}else {
					if(!libid2_getICCard(1000,&icctype,iccid)){
						printf("read iccid success\n");
						printf("icctype:%02x\n",icctype);					
						printf("iccid:%s\n",iccid);						
					}else {
						printf("get iccid failed\n");
					}
				
					libid2_close();
				}
	        }else if(!strncmp(_argv[0],"idsam",4)){
	        	char iccid[128];
				int samlength;
				int icctype;
	        	//m_getsamid(ctx);
	        	
				err = libid2_open(dev);
				if(err){
					printf("open libid2 failed\n");
				}else {
					if(!libid2_getID2Number(1000,&icctype,iccid)){
						printf("read iccid success\n");
						printf("icctype:%02x\n",icctype);					
						printf("iccid:%s\n",iccid);						
					}else {
						printf("get iccid failed\n");
					}
				
					libid2_close();
				}
	        }else if(!strncmp(_argv[0],"back",1)||!strncmp(_argv[0],"..",2)){
	            printf("back to parent menu...\n");
	            exit=1;
	        }if(!strncmp(_argv[0],"exit",4)||!strncmp(_argv[0],"quit",1)){
	            printf("exit program...\n");
	            platform_program_exit(0);
	        }else{
	        	char* newmenu = strdup(_argv[0]);
				argv_dispose();
	        	exec_menu(cli,newmenu);
				free(newmenu);
	        }						

				
       	}

		argv_dispose();

    }

	return 0;
	
}

int cli_loop_reader(cli_menu* cli){
	static char cmd[1024]; 
	static char buf[2048];
	int exit=0;
    int ret;
	int started = 0;
	int cmd_len,i;	
	int err;
	char* cmdptr;
	const char* dev = shared.devname;
    while (!exit) {
        // Display the usage
        show_menu_help(cli,
		"info                     ---get reader info\n"        
        "reg [address] [value]    ---read/write register\n"
        "send <frame data>        ---send frame data to reader,all data is hex bcd encoding\n"
        "                            e.g send 050000 send 3bytes 0x05 0x00 0x00");

        // accept the command
        memset(cmd,0,sizeof(cmd));
		if(cmdptr=fetch_seq_cmd(&shared)){
			strcpy(cmd,cmdptr);
			cmd_len = strlen(cmd);
		}else {
			cmdptr = gets(cmd);
			cmd_len = strlen(cmd); 
			if(!cmdptr||!cmd_len) {
				fflush(0);
				continue;
			}
		}
		for (i=0; i<cmd_len;i++)
			cmd[i] = tolower(cmd[i]); 

		str2argv(cmd);
		if(_argc) {
			if(!strncmp(_argv[0],"info",1)){
				err = xfer_packet_wrapper(dev,buf,64,CMD_CLASS_READER,READER_SUB_CMD_INFOMATION,0);
				if(!err&&STATUS_CODE(buf)==STATUS_CODE_SUCCESS){
					printf("reader adapter id:0x%x\n",*(PACKET_RESP(buf)));
				}else {
					printf("get reader info failed\n");
				}			
	        }else if(!strncmp(_argv[0],"send",1)){
	        	if(_argc<2){
					printf("format:\n"
						   "  send 050000");
	        	}else {
	        		unsigned int frame_len=str2hex(_argv[1],buf);
					dumpdata("frame->",buf,frame_len);					
					err = xfer_packet_wrapper2(dev,buf,64,CMD_CLASS_READER,READER_SUB_CMD_XFER_FRAME,buf,frame_len);					
					if(!err&&STATUS_CODE(buf)==STATUS_CODE_SUCCESS){
						printf("result okay\n");
						frame_len = PACKET_PAYLOAD_LEN(buf)-PACKET_STATUS_LEN;
						dumpdata("frame<-",PACKET_RESP(buf),frame_len);						
					}else {
						printf("result failed=0x%04x\n",STATUS_CODE(buf));
					}
	        	}
	        	
	        }else if(!strncmp(_argv[0],"reg",1)){
	        	char* e;
	        	unsigned int r,v;
	        	if(_argc>2){
					r = ustrtoul(_argv[1],&e,0);
					v = ustrtoul(_argv[2],&e,0);
					r&=0xff;
					v&=0xff;
					printf("write reg[0x%02x]=0x%02x\n",r,v);
					err = xfer_packet_wrapper(dev,buf,64,CMD_CLASS_READER,READER_SUB_CMD_WRITE_REG,2,r,v);
					if(!err&&STATUS_CODE(buf)==STATUS_CODE_SUCCESS){
						printf("result okay\n");
					}else {
						printf("result failed\n");
					}
	        	}else if(_argc>1){
	        		r = ustrtoul(_argv[1],&e,0);
					r&=0xff;
					err = xfer_packet_wrapper(dev,buf,64,CMD_CLASS_READER,READER_SUB_CMD_READ_REG,1,r);
					if(!err&&STATUS_CODE(buf)==STATUS_CODE_SUCCESS){
						v = *(PACKET_RESP(buf));
						printf("read reg:0x%02x=0x%02x  okay\n",r,v);
					}else {
						printf("read reg failed\n");
					}
	        	}else {
	        		int i=1;
	        		printf("read all register available(THM3060)\n");
					for(;i<0x12;i++){
						r = i;
						err = xfer_packet_wrapper(dev,buf,64,CMD_CLASS_READER,READER_SUB_CMD_READ_REG,1,r);
						if(!err&&STATUS_CODE(buf)==STATUS_CODE_SUCCESS){
							v = *(PACKET_RESP(buf));
							printf("0x%02x:0x%02x \n",(unsigned char)r,(unsigned char)v);
						}else {
							printf("0x%02x:failed\n");
						}
					}
	        	}
	        }else if(!strncmp(_argv[0],"back",1)||!strncmp(_argv[0],"..",2)){
				printf("back to parent menu...\n");
				exit=1;
			}if(!strncmp(_argv[0],"exit",4)||!strncmp(_argv[0],"quit",1)){
				printf("exit program...\n");
				platform_program_exit(0);
			}else{
	        	char* newmenu = strdup(_argv[0]);
				argv_dispose();
	        	exec_menu(cli,newmenu);
				free(newmenu);
			}						

				
       	}

		argv_dispose();

    }

	return 0;
	
}


int cli_loop_main(cli_menu* cli){
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
        show_menu_help(cli,NULL);
        // accept the command
        memset(cmd,0,sizeof(cmd));
		if(cmdptr=fetch_seq_cmd(&shared)){
			strcpy(cmd,cmdptr);
			cmd_len = strlen(cmd);
		}else {
			cmdptr = gets(cmd);
			cmd_len = strlen(cmd); 
			if(!cmdptr||!cmd_len) {
				fflush(0);
				continue;
			}
		}
		for (i=0; i<cmd_len;i++)
			cmd[i] = tolower(cmd[i]); 

		str2argv(cmd);
		if(_argc) {
			if(!strncmp(_argv[0],"back",1)||!strncmp(_argv[0],"..",2)){
	            printf("back to parent menu...\n");
	            exit=1;
	        }if(!strncmp(_argv[0],"exit",4)||!strncmp(_argv[0],"quit",1)){
	            printf("exit program...\n");
	            platform_program_exit(0);
	        }else{
	        	char* newmenu = strdup(_argv[0]);
				argv_dispose();
	        	exec_menu(cli,newmenu);
				free(newmenu);
	        }	
       	}

		argv_dispose();

    }

	return 0;
	
}

static void usage(const char* program,int exitprogram){
    fprintf(stderr, 
        "Usage: %s [-p </dev/ttySx>] [-b <baudrate>] [-c <commands>] \n\n"
        "Specify config in command line\n"
		"For linux,port name should be /dev/ttySx\n"
		"For win32,port name should be COMx\n"		
		"For example:\n"
		"%s -p /dev/ttyUSB0 \n"
		"%s -p /dev/ttyUSB0 -b 115200\n"
		"%s -p COM4 \n"
		"%s -p COM4 -b 115200\n"
		"\nSpecify config in config file id2reader.cfg\n"
		"Example config file:\n"
		"port=COM4\n"
		"baudrate=115200\n"
		"<commands> is commands sequence to execute,format is \"<cmd1>;<cmd2>\"\n",
        program,program,program,program,program);
    if(exitprogram)
	    exit(-1);
}
int main(int argc, char **argv) {
	int i=0;
	char cfgfile_port[MAX_ENTRY_LEN];
	char cfgfile_baudrate[MAX_ENTRY_LEN];
	char* port=DEFAULT_PORT;
	char cfgile_seqcmds[MAX_ENTRY_LEN]={0};
    int baudrate = 115200;	
	
	if(!get_config(0,"port",cfgfile_port))
		port = cfgfile_port;
	if(!get_config(0,"baudrate",cfgfile_baudrate)){
		baudrate = atoi(cfgfile_baudrate);
	}
	
	get_config(0,"seqcmd",cfgile_seqcmds);

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
        if (!strcmp(argv[i],"-c") ){
            if (++i >= argc) goto fail;
			strcpy(cfgile_seqcmds,argv[i]);
            continue;
        }
        
        fail:
            usage(argv[0],1);
    }
    
    usage(argv[0],0);
    printf("-------------------------------------------\n");
	printf("\n\nPort [%s@%d] will be opened\n",port,baudrate);
	if(cfgile_seqcmds)
		printf("Sequence commands:%s\n",cfgile_seqcmds);

	menu_top.children=menu_top.sibling = NULL;
	menu_top.func = cli_loop_main;
	menu_top.menuname = "";
	shared.devname = port;
	shared.baudrate = baudrate;
	shared.seqcmd = cfgile_seqcmds;

	init_menu(&menu_top,&menu_sam,"sam",cli_loop_sam);
	init_menu(&menu_top,&menu_mcu,"mcu",cli_loop_mcu);
	init_menu(&menu_top,&menu_reader,"idr",cli_loop_reader);
	
	return cli_loop_main(&menu_top);
}
