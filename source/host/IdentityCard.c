#include "platform.h"
#include "IdentityCard.h" 

#define DEBUG 0

#define FILENAME_ID2     "/system/usr/id.wlt"  //
#define FILENAME_BMP    "/id.bmp"

#define ATTR_POWER  "/sys/class/gpio/gpio45/value"
static long  time_start =0;

static int   cancel_flag =0;


typedef struct
{
	int state;// 
	int action;	// 
	int option;
}EJ_DEV_PARAM;

static int fdport = -1; 

enum {
	CMD_RESET = 0 ,
	CMD_GET_SAMID = 1,
	CMD_FIND_CARD = 2,
	CMD_SELECT_CARD =3,
	CMD_READ_CARD = 4,
	CMD_GET_ICCARD = 5,
	CMD_MAX_SIZE = 6
};

#define CMD_LENGTH 10
/* header + size */
#define RESP_HEADER_LEN 7  
/* header */
#define HEADER_LEN 5
#define RESP_HEADER       "\xAA\xAA\xAA\x96\x69"
#define ID2INFOCHECK                 "\x00\x00\x90"
#define ID2CHECKLENGTH        3

static struct cmd_list {
	int cmd_index; 	// CMD_XXXX
	char *cmd;	// Contents of the command
	char *resp;	// repsonsed, NULL - unknown respone, should caculate the data length and read the data
}cmds_list[] = {
	{CMD_RESET, "\xAA\xAA\xAA\x96\x69\x00\x03\x10\xff\xec", "\x00\x00\x90\x94"},
	{CMD_GET_SAMID, "\xAA\xAA\xAA\x96\x69\x00\x03\x12\xff\xee", NULL},
	{CMD_FIND_CARD, "\xAA\xAA\xAA\x96\x69\x00\x03\x20\x01\x22", "\x00\x00\x9f\x00\x00\x00\x00\x97"},
	{CMD_SELECT_CARD, "\xAA\xAA\xAA\x96\x69\x00\x03\x20\x02\x21", "\x00\x00\x90\x00\x00\x00\x00\x00\x00\x00\x00\x9c"},
	{CMD_READ_CARD, "\xAA\xAA\xAA\x96\x69\x00\x03\x30\x01\x32", NULL},
	{CMD_GET_ICCARD, "\xAA\xAA\xAA\x96\x69\x00\x03\x30\x01\x32", NULL},
};
#undef SUCCESSED
#undef FAILED

#define   BAUDRATE    115200
#define   SUCCESSED     0
#define   FAILED           -1

#define INTERNAlTIMEOUT   1

#define  ERRTIMEOUT   5

#define   POWERID  1017
#define ID2INFOLENGTH   (256 + 1024)


static int send_cmd(int cmd_index);
static int cmd_resp(int cmd_index, char *buf, int *len,int time_out);

/* 
 * Decription for TIMEOUT_SEC(buflen,baud);
 * baud bits per second, buflen bytes to send.
 * buflen*20 (20 means sending an octect-bit data by use of the maxim bits 20)
 * eg. 9600bps baudrate, buflen=1024B, then TIMEOUT_SEC = 1024*20/9600+1 = 3 
 * don't change the two lines below unless you do know what you are doing.
*/
#define TIMEOUT_SEC(buflen) (buflen*20/BAUDRATE)




//OpenComPort(RF_SERIAL_PORT,115200,8,"1",0)==-1?false:true;

int libid2_open(const char *uart_name)
{
	int retval;

	fdport = serialport_open (uart_name);//O_NONBLOCK //  | O_NOCTTY
	if (fdport<0) {
		printf ("cannot open port %s\n",uart_name);
		return -1;
	}

	retval = serialport_config (fdport,BAUDRATE, 8, 1, 0);
	if (retval<0) {
		printf("SetPortAttr err \n");
		serialport_close(fdport);
		return -1;
	}
	return 0;
}

/*
send cmd an resp buf.  if resp buf is internal time out ,  send cmd until cmd_resp return -1; 

*/
static int receive_buff(int cmd_index, char *buff,int *bufflenth ,int time_out)
{
	int ret;

	while(1)
	{
		ret =  send_cmd(cmd_index);
		if(-1 == ret)
		{
			usleep(1000);
			return -1;
		}
		

		ret = cmd_resp(cmd_index,buff,bufflenth,time_out);

		if(ERRTIMEOUT == ret)
		{
			usleep(1000);
			continue;
		}
		else if(-1 == ret)
		{
			return -1;
		}
			
		return 0;		
	}
	
	return 0;

}



int  longtoSrting(long lsrc,int length,char *sdesc)
{
	char szResult[20] = "0000000000000000000";
	char temp[20] = "";
	int nwr=0;
	
	if(sdesc == NULL)
	{
		return -1;
	}
	
	nwr = sprintf(temp, "%ld", lsrc);
	memcpy(&szResult[length - nwr],temp,nwr);
	szResult[length] = '\0';
	strcpy(sdesc,szResult);
	return 0;
	
}

int  SamID_Caculate(char *byteSamID, int iPos,char * descsamid)
{
	char  szResult[40] = "";
	char   stemp[40] ="";
	long iLow = byteSamID[0+iPos] & 0xff;
	long iHight = ((long)(byteSamID[1+iPos] & 0xff)) << 8;
	long iTmp = iHight + iLow;
	
	if(byteSamID == NULL )
	{
		printf("byteSamID is err \n");
		return -1;
	}
	
	longtoSrting(iTmp,2,stemp);
	strcat(szResult,stemp);
	strcat(szResult,"-");
	
	iLow = byteSamID[2+iPos] & 0xff;
	iHight = ((long)(byteSamID[3+iPos] & 0xff)) << 8;			
	iTmp = iHight + iLow;

	longtoSrting(iTmp,2,stemp);
	strcat(szResult,stemp);
	strcat(szResult,"-");

	iLow = byteSamID[4+iPos] & 0xff;
	iHight = ((long)(byteSamID[5+iPos] & 0xff)) << 8;	 
	iTmp = iHight + iLow;
	iLow = ((long)(byteSamID[6+iPos] & 0xff)) << 16;
	iHight = ((long)(byteSamID[7+iPos] & 0xff)) << 24;	  
	iTmp = iTmp+ iHight + iLow; 		
	longtoSrting(iTmp,8,stemp);
	strcat(szResult,stemp);
	strcat(szResult,"-");

	iLow = byteSamID[8+iPos] & 0xff;
	iHight = ((long)(byteSamID[9+iPos] & 0xff)) << 8;	 
	iTmp = iHight + iLow;
	iLow = ((long)(byteSamID[10+iPos] & 0xff)) << 16;
	iHight = ((long)(byteSamID[11+iPos] & 0xff)) << 24;    
	iTmp = iTmp+ iHight + iLow; 						
	longtoSrting(iTmp,10,stemp);
	strcat(szResult,stemp);
	strcat(szResult,"-");
	iLow = byteSamID[12+iPos] & 0xff;
	iHight = ((long)(byteSamID[13+iPos] & 0xff)) << 8;	  
	iTmp = iHight + iLow;
	iLow = ((long)(byteSamID[14+iPos] & 0xff)) << 16;
	iHight = ((long)(byteSamID[15+iPos] & 0xff)) << 24;
	iTmp = iTmp+ iHight + iLow; 						
	longtoSrting(iTmp,10,stemp);	

	strcat(szResult,stemp);
	strcpy(descsamid,szResult);
	return 1;
}


int libid2_getsamid(char *samid,int *samlenth,int time_out)
{
	struct timeval t_start;
	char tempsamid[20] = {0};
	int templenth =0 ,ret =0;

	char descsamid[40]="";
	if((fdport <=0) || (libid2_get_power_state() != 1))
	{
		printf("fdport not open or power is off \n");
		return -1;
	}
	
	gettimeofday(&t_start, NULL); 
	time_start = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000; 

	ret = receive_buff(CMD_GET_SAMID, tempsamid, &templenth,(int)10);
	if(ret < 0)
	{
		return -1;
	}
	
	//memcpy(samid,&tempsamid[3],*samlenth);
	SamID_Caculate(&tempsamid[3],0,samid);
	//printf("zzzzzsamid = %d \n",strlen(samid));
	*samlenth = strlen(samid);
	return 0; 
}

void libid2_cancel_read_id2()
{
	cancel_flag = 1;
}

int libid2_read_id2_info(char *id2info,int *id2infolength,int time_out)
{
	int ret ;
	if((fdport <=0) || (libid2_get_power_state() != 1))
	{
		printf("fdport not open or power is off \n");
		return -1;
	}
	cancel_flag = 0;
	struct timeval t_start,t_end; 
	long  time_end =0; //ms

	gettimeofday(&t_start, NULL); 
	time_start = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000; 

	while(1)
	{
		gettimeofday(&t_end, NULL); 
		time_end = ((long)t_end.tv_sec)*1000+(long)t_end.tv_usec/1000;

		if((time_end - time_start) >= time_out *1000)
		{
			return -1;
		}
		serialport_flush (fdport, 2);
		if(cancel_flag ==1)
		{
			return -1;
		}
		usleep(10000);
		ret = receive_buff((int)CMD_FIND_CARD, NULL, NULL,time_out);
		if(ret <0)
		{
			usleep(10000);
			continue;
		}
		serialport_flush (fdport, 2);
		if(cancel_flag ==1)
		{
			return -1;
		}
		usleep(10000);
		ret =  receive_buff((int)CMD_SELECT_CARD, NULL, NULL,time_out);
		if(ret <0)
		{
			usleep(10000);
			continue;
		}
		serialport_flush (fdport, 2);
		if(cancel_flag ==1)
		{
			return -1;
		}
		usleep(10000);
		 ret = receive_buff((int)CMD_READ_CARD, id2info, id2infolength,time_out);
		if(cancel_flag ==1)
		{
			return -1;
		}
		 if(ret <0)
		{
			usleep(10000);
			continue;
		}
		 usleep(10000);
		 break;
	}

	return 0;

}


void libid2_close(void)
{
	if((fdport <=0) || (libid2_get_power_state() != 1))
	{
		printf("fdport not open or power is off");
		return ;
	}
	serialport_close (fdport);
	fdport = -1;	
}

static int check_header(char *header)
{
	/* aa aa aa 96 69 */
	if (!memcmp(header, RESP_HEADER, HEADER_LEN))
		return 0;
	else
		return -1;
}

static int send_cmd(int cmd_index)
{
	int writelenth = 0;
	int  writeTotalLen = 0 ;

	if (cmd_index < 0 || cmd_index >= CMD_MAX_SIZE) {
		printf("cmd isn't valid! cmd = %d\n", cmd_index);
		return -1;
	}

	while(1)
	{
		writelenth = serialport_write(fdport, cmds_list[cmd_index].cmd, CMD_LENGTH) ;
		if(writelenth <= 0)
		{
			printf("write %d\n", writelenth);
			return -1;
		}
		writeTotalLen +=  writelenth;
		if(CMD_LENGTH == writeTotalLen)
		{
			return 0;	
		}
	}

	return 0;
}


/*


*/

static int cmd_resp(int cmd_index, char *buf, int *len,int timeout)
{

	fd_set   PortRead;
	int Ret;
	int readlenth = 0;
	int  readTotalLen = 0 ;
	struct timeval tvTimeout;
	struct timeval t_start,t_end; 

	char header[RESP_HEADER_LEN], *tmp_buf;
	int data_len;

	long  time_start_receive =0,time_end =0; //ms

	gettimeofday(&t_start, NULL); 
	time_start_receive = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000; 

	while(1)
	{

		gettimeofday(&t_end, NULL); 
		time_end = ((long)t_end.tv_sec)*1000+(long)t_end.tv_usec/1000;

		//printf("time_end - time_start == %ld\n",time_end - time_start);

		if((time_end - time_start) >= timeout *1000)
		{
			//printf("zzm time out \n");
			return -1;
		}

		if((time_end - time_start_receive) >= 3*1000)
		{
			return ERRTIMEOUT;
		}

		readlenth = serialport_read(fdport, header + readTotalLen, RESP_HEADER_LEN - readTotalLen,0) ;
		if(cancel_flag == 1)
		{
			return -1;
		}
		if(readlenth <= 0)
		{
			//printf("read readlength %d\n", readlenth);
			usleep(1000);
			continue;
		}
		else 
		{
			readTotalLen +=  readlenth;
		}

		if(readTotalLen >= RESP_HEADER_LEN)
		{
			break;
		}
		continue;
	
	}
	
 	


	if (check_header(header)) {
#if (DEBUG)			
		printf("header error! %x %x %x %x %x\n", 
				header[0], header[1], header[2], header[3], header[4]);
#endif
		return -1;
	}

	//printf("header: %x %x\n", header[5], header[6]);
	data_len =(header[5]<<8) + header[6];


	//printf("resp data length = %d\n", data_len);

	if(data_len == 0)
	{
		//printf("resp data lenth = %d \n",data_len);
		return -1;
	}

	if(cmd_index == CMD_READ_CARD && data_len  < (256 + 1024))
	{
		//data_len = 256 + 1024;
		//printf("read card lenth == %d err ! ,cmd_index == %d",data_len,cmd_index);
		return -1;
		
	}
	
	tmp_buf = malloc(sizeof(char) * data_len);
	if (!tmp_buf) {
		printf("malloc %d memory failed with %s!\n", data_len, strerror(errno));
		return -1;
	}


#if 1	


	/* FIXME: rewrite this!!! */
	readTotalLen = 0;
	gettimeofday(&t_start, NULL); 
	time_start_receive= ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000; 

	while(1)
	{

		gettimeofday(&t_end, NULL); 
		time_end = ((long)t_end.tv_sec)*1000+(long)t_end.tv_usec/1000;

		//printf("zzm::::::::time_end - time_start == %ld\n",time_end - time_start);
		if((time_end - time_start) >= timeout *1000)
		{
#if (DEBUG)	
			printf("zzm time out \n");
#endif
			free(tmp_buf);
			return -1;
		}

		if((time_end - time_start_receive) >= 3000)
		{
			free(tmp_buf);
			return ERRTIMEOUT;
		}

		readlenth = serialport_read(fdport, tmp_buf + readTotalLen , data_len - readTotalLen,0) ;
		if(cancel_flag ==1)
		{
			return -1;
		}
		if(readlenth <= 0)
		{
			usleep(1000);
			continue;
		}
		else 
		{
			readTotalLen +=  readlenth;
		}

		if(readTotalLen >= data_len)
		{
			break;
		}
		continue;

	}


#else	
	while (read(fdport, tmp_buf, data_len) != data_len);
#endif

	/* if resp isn't null, compare it with the tmp_buf */
	if (cmds_list[cmd_index].resp) 
	{
#if (DEBUG)	
		int i = 0;
		for(i =0;i<data_len;i++)
		{
			printf("%2x ", tmp_buf[i]);
			if(i%6 == 0)
				printf("\n");

		}
		printf("cmd_index = %d\n",cmd_index);
#endif		
		if (memcmp(tmp_buf, cmds_list[cmd_index].resp, data_len)) {
#if (DEBUG)	
			printf("resp buf is err \n");
#endif				
			goto err_out;
		}
	}
	else
	{
		if (!buf)
		{
			printf("cmd_resp buf is null!\n");
			goto err_out;
		}
		else 
		{
#if (DEBUG)			
			int i;
			for(i =0;i<data_len;i++)
			{
				printf("%2x ", tmp_buf[i]);
				if(i%6 == 0)
					printf("\n");

			}
			printf("\n");

#endif
			*len = data_len;
			memcpy(buf, tmp_buf, data_len);

		}
	}

	free(tmp_buf);
	return 0;
err_out:
	free(tmp_buf);
	return -1;

}

int libid2_reset(void)
{
	struct timeval t_start;
	if((fdport <=0) || (libid2_get_power_state() != 1))
	{
		printf("fdport not open or power is off \n");
		return -1;
	}
	
	gettimeofday(&t_start, NULL); 
	time_start = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000; 

	return receive_buff(CMD_RESET, NULL, NULL,5);
}



#if 0
static int power_opt(int action,int option)
{
	int fd;
	EJ_DEV_PARAM param;

	fd = open("/dev/xh_misc" ,O_RDWR);

	if(fd == -1 ){
		printf("open fail\n");
		return -1;
	}

	param.action=action;
	param.option=option;

	if (ioctl(fd,POWERID,&param) < 0)
	{
		close(fd);
		return -1;
	}

	close(fd);
	return param.state;
}
#else
static int power_opt(int action,int option)
{
	return 3;
}
#endif
static int read_attr_file( const char* file, int* pResult)
{
	int fd = -1;
	fd = open( file, O_RDONLY );
	if( fd >= 0 ){
		char buf[20];
		int rlt = read( fd, buf, sizeof(buf) );
		if( rlt >= 0 ){
			buf[rlt] = '\0';
			*pResult = atoi(buf);
			close(fd);
			return 0;
		}
		close(fd);
		return -1;
	}
	return -1;
}


static int write_attr_file(const char* path,int val)
{
	int fd,nwr;
	char value[20]; 			
	fd = open(path, O_RDWR);
 	if(fd<0) return -1;
	
	nwr = sprintf(value, "%d\n", val);
	write(fd, value, nwr);
	close(fd);		

	return 0;

}


/*
 * return value:
 * 	-1 - argument error or open xh_misc error
 * 	0  - success
 */
int libid2_power(int on_off)
{

	if(1 == on_off)
	{
		write_attr_file(ATTR_POWER,1);
		//printf(" power on\n");
	}
	else if(0 == on_off)
	{
		write_attr_file(ATTR_POWER,0);//off
	}
	else 
	{
		printf("int parm is err\n");
		return -1;
	}
	return 0;
}

/*
 * return value:
 * 	-1 - failed
 * 	1 - power 0f;
 *     2  - power sleep;
 *     3 - STATE_WORKINGl
 *
 */


int libid2_get_power_state(void)
{
	int  pResult = 0;
	read_attr_file( ATTR_POWER, &pResult);
	if(pResult == 1)
	{
		return 1;
	}
	//always return 1 for test
	return 1/*0*/;
}

static int check_id2_info(char *id2info)
{
	int textlength = 0;
	int imagelength = 0;
		/*00 00 90 */
	if (memcmp(id2info, ID2INFOCHECK, ID2CHECKLENGTH))
	{
		printf("id2_head_err:\n");
		return -1;
	}


	textlength = (id2info[ID2CHECKLENGTH ]<<8) + id2info[ID2CHECKLENGTH+1];
	if(textlength != 256)
	{
		printf("textlength err \n");
		return -1;
	}

	imagelength = (id2info[ID2CHECKLENGTH +2]<<8) + id2info[ID2CHECKLENGTH+3];	
	if(imagelength != 1024)
	{
		printf("imagelength err \n");
		return -1;
	}
	return 0;
}


int libid2_decode_info(char *id2infobuf,int length,ID2Info * info)
{

	//memset(info,0,sizeof(ID2Info));
	if(id2infobuf == NULL || info == NULL || length  < ID2INFOLENGTH)
	{
		printf("int parm is err\n");
		return -1;
	}

	if(check_id2_info(id2infobuf))
	{
		return -1;
	}

	memcpy(info->name,&id2infobuf[ID2CHECKLENGTH + 4],ID2INFOLENGTH);

	return 0;
}

int libid2_decode_image(char *decodebuf)
{
	int ret;
	void *handle;//
	typedef int(*FuncPtr)(char *,int);
	const char fso[]="libwltu.so";

	if(decodebuf == NULL)
	{
		return -1;
	}
	
	FILE *file = fopen(FILENAME_ID2,"wb");
	if(file ==  NULL)
	{
		printf("open filed \n");
		return -1;
	}
	
	fwrite(decodebuf,1,1024,file);
	fclose(file);

	//extern int GetBmp(char *filename, int intf);

	//GetBmp(FILENAME_ID2, 1);
	handle = LoadSharedLibrary(fso, 0);

	if( handle == NULL )
	{
		printf("loading library %s failed\n",fso);//
		return -1;
	}
	// fun1 = dlsym(handle, "GetBmp" );
	FuncPtr fun1=(FuncPtr)LoadSymbol(handle,"GetBmp");
	ret  =(*fun1)(FILENAME_ID2,1);
	//should close dl handle
	ReleaseSharedLibrary(handle);
	return ret;
	
}




int BCD2ASCII(const unsigned char* bcd, int bcdLen,unsigned char* ascii )
{
    	const unsigned char* pbcd;
   	 unsigned char* pascii;
	int i;
	unsigned char c;

    	pbcd = bcd ;
	pascii = ascii ;

	for(i=0;i<bcdLen;i++)
	{
		c = ((*pbcd) & 0xF0) >>4 ;
		if(c <= 0x09 )
		{
			*pascii++ = c + '0' ;
		}
		else
		{
			*pascii++ = c - 0x0a + 'A' ;
		}
		c = (*pbcd) & 0x0F ;
		if(c <= 0x09 )
		{
			*pascii++ = c + '0' ;
		}
		else
		{
			*pascii++ = c - 0x0a + 'A' ;
		}
		pbcd++;
	}
	*pascii = '\0';

	return 0;
}

int ASCII2BCD(const unsigned char* ascii, unsigned char* bcd,int *bcdLen )
{
	unsigned char* pbcd;
	int i;
        int len = strlen((char*)ascii);

	pbcd = bcd ;

	if(len%2) return -1;
	*bcdLen = len/2 ;

	for(i=0;i<len;i+=2)
	{
		//\u9ad8\u534a\u5b57\u8282
		if(ascii[i] <= '9' && ascii[i] >='0')
		{
			*pbcd = ((ascii[i] - '0')<<4) & 0xf0 ;
		}
		else if(ascii[i] <= 'f' && ascii[i] >='a')
		{
			*pbcd = ((ascii[i] - 'a' + 0x0a )<<4) & 0xf0 ;
		}
		else if (ascii[i] <= 'F' && ascii[i] >='A')
		{
			*pbcd = ((ascii[i] - 'A' + 0x0a )<<4 ) & 0xf0;
		}
		else
		{
			return -2;
		}
		//\u4f4e\u534a\u5b57\u8282
		if(ascii[i+1] <= '9' && ascii[i+1] >='0')
		{
			*pbcd |= (ascii[i+1] - '0') & 0x0f ;
		}
		else if(ascii[i+1] <= 'f' && ascii[i+1] >='a')
		{
			*pbcd |= (ascii[i+1] - 'a' + 0x0a ) & 0x0f ;
		}
		else if (ascii[i+1] <= 'F' && ascii[i+1] >='A')
		{
			*pbcd |= (ascii[i+1] - 'A' + 0x0a ) & 0x0f ;
		}
		else
		{
			return -3;
		}

		pbcd++;
	}

	return 0;
}

/*
*   the DelayTime is the time out time ,Unit ms aCardType is the card type the restult is 0x0a,0x0b,0x0c.
*   cardID is card id,the conetent have 4 bytes; the usr must malloc 4 bytes or more;
*/

int libid2_getICCard(int DelayTime,int * aCardType,char * CardId)
{
	int writelenth = 0;
	int readlenth = 0;
	int totallenth = 0;
	
	char recbuf[256]={0};
	char lrc;
	unsigned char iSendData[256] = {0};
	int i;
	char *pSrc;
	char tempstr[20];
	
	struct timeval t_time; 
	long time_start= 0,time_end =0; //ms
	cancel_flag = 0;

	if((fdport <=0) || (libid2_get_power_state() != 1))
	{
		printf("fdport not open or power is off \n");
		return -1;
	}
	
	//heard
	iSendData[0] = 0x02 ;
	//source length;
	iSendData[1] = 0x00;
	iSendData[2] = 0x04 ;

	//command type
	iSendData[3] = 0x32;
	iSendData[4] = 0x41 ;
	//time out
	iSendData[5] = (unsigned char)((DelayTime & 0xff00)>>8);
	iSendData[6] = (unsigned char)((DelayTime & 0x00ff)); 

	lrc = iSendData[3];
	pSrc = &iSendData[4];

	for(i=1;i<4;i++)
	{
		lrc = lrc^(*pSrc);
		pSrc++;
	}

	iSendData[7] = lrc;
	iSendData[8] = 0x03;
	
	dump("icc packet",iSendData,10);


	writelenth = serialport_write(fdport, iSendData, 9) ;
	
	if(writelenth != 9)
	{
		printf("write failed\n");
		return -1;
	}


	gettimeofday(&t_time, NULL); 
	time_start = ((long)t_time.tv_sec)*1000+(long)t_time.tv_usec/1000; 
	
	memset(recbuf,0,256);
	while(1)
	{
		readlenth = serialport_read(fdport, &recbuf[totallenth],256 -totallenth,0) ;
		if(readlenth >0)
		{
			totallenth +=  readlenth;
			if(recbuf[0] != 2)
			{
				printf("recive head is err\n");
				return -1;
			}
			if((recbuf[1] *256 + recbuf[2] + 5) == totallenth)
			{
				break;
			}
			continue;
		}
		
		gettimeofday(&t_time, NULL); 
		time_end = ((long)t_time.tv_sec)*1000+(long)t_time.tv_usec/1000; 
		if(cancel_flag ==1)
		{
			return -1;
		}
		//printf("readlenth = %d \n",totallenth);
		if((time_end - time_start) > (DelayTime+1000))
		{
			printf("time out \n ");
			return -1;
		}
	}

	lrc = recbuf[3];//the third bit is the ture content
	pSrc = &recbuf[4];
	for(i=1;i<totallenth- 5;i++)
	{
		lrc = lrc^(*pSrc);
		pSrc++;
	}

	if(lrc != *pSrc)
	{
		printf("check out si err = %x",lrc);
		return -1 ;
	}

	if(recbuf[3] == 0x0 && recbuf[4] == 0x0 && recbuf [totallenth -1] == 0x03)
	{
		*aCardType = recbuf[5];
		BCD2ASCII((unsigned char*)&recbuf[6],4,(unsigned char*)tempstr );

		strcpy(CardId,tempstr);
		return 0;
	}

	return -1;
}

