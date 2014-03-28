#include "platform.h"
#include "option.h"
#include "IdentityCard.h"
#include "download.h"
#include "packet.h"
#include "config.h"

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
         "*******************************************\n\n");
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
			if(!strncmp(_argv[0],"reset",5)){				
	        	err = libid2_open(dev);
				if(err){
					printf("open libid2 failed\n");
				}else{
					ret=libid2_reset();
					printf("reset sam %s\n",ret<0?"failed":"okay");
					libid2_close();
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
					if(!libid2_getsamid(buf,&samlength,5/*timeout*/)){
						printf("samid[%s]\n",buf);
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
        "Usage: %s [-p </dev/ttySx>] [-b <baudrate>] \n\n"
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
		"baudrate=115200\n",		
        program,program,program,program,program);
    if(exitprogram)
	    exit(-1);
}
int main(int argc, char **argv) {
	int i=0;
	char cfgfile_port[MAX_ENTRY_LEN];
	char cfgfile_baudrate[MAX_ENTRY_LEN];
	char* port=DEFAULT_PORT;
    int baudrate = 115200;	
	
	if(!get_config(0,"port",cfgfile_port))
		port = cfgfile_port;
	if(!get_config(0,"baudrate",cfgfile_baudrate)){
		baudrate = atoi(cfgfile_baudrate);
	}

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
            usage(argv[0],1);
    }
    
    usage(argv[0],0);
    printf("-------------------------------------------\n");
	printf("\n\nPort [%s@%d] will be opened\n",port,baudrate);
	menu_top.children=menu_top.sibling = NULL;
	menu_top.func = cli_loop_main;
	menu_top.menuname = "";
	shared.devname = port;
	shared.baudrate = baudrate;

	init_menu(&menu_top,&menu_sam,"sam",cli_loop_sam);
	init_menu(&menu_top,&menu_mcu,"mcu",cli_loop_mcu);
	return cli_loop_main(&menu_top);
}
