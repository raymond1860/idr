#include "platform.h"
int serialport_open(const char* name){
 	COMMTIMEOUTS timeouts={0};
	int fd;
	char devname[256];
	
	if(!name) return -1;
	sprintf(devname,"%s",name);
	//COM1~COM9 is okay
	//handle name if higher than COM10
	if(!strncmp(name,"COM",3)){
		char* comname = strdup(name);
		char* comid=&comname[3];
		if(*comid){
			int id = atoi(comid);
			if(id>=10){
				sprintf(devname,"\\\\.\\COM%d",id);
			}
		}
		free(comname);
	}
	
	fd = CreateFile(devname,  // Specify port device: default "COM1"
	GENERIC_READ | GENERIC_WRITE,       // Specify mode that open device.
	0,                                  // the devide isn't shared.
	NULL,                               // the object gets a default security.
	OPEN_EXISTING,                      // Specify which action to take on file. 
	0,                                  // default.
	NULL);
	if(fd==INVALID_HANDLE_VALUE){
	   if(GetLastError()==ERROR_FILE_NOT_FOUND)
	   {
	     //serial port does not exist. Inform user.
	     printf("Error: Serial port does not exist. Please check the port name and try again\n");
	   }
 		return -ENOENT;
	}

	//set default timeout params
	timeouts.ReadIntervalTimeout=0;
	timeouts.ReadTotalTimeoutConstant=1;
	timeouts.ReadTotalTimeoutMultiplier=1;
	timeouts.WriteTotalTimeoutConstant=2000;
	timeouts.WriteTotalTimeoutMultiplier=0;

	if(!SetCommTimeouts((HANDLE)fd, &timeouts)){
 		return -ENOENT;
	}
             
	return fd;
}


int serialport_close(int fd){
	if(INVALID_HANDLE_VALUE!=(HANDLE)fd)
		CloseHandle((HANDLE)fd);
	return 0;
}

int serialport_write(int fd,unsigned char* buf,int size){	
	if(INVALID_HANDLE_VALUE!=(HANDLE)fd){
		DWORD dwBytesWriten;
		 /* Write to Port */
 		 if(!WriteFile((HANDLE)fd, buf, size, &dwBytesWriten, NULL)){
 		 	printf("write failed in serialport_writen");
		 	return -EINVAL;
  		}
		return (int)dwBytesWriten;
	}
	return -EINVAL;
}
int serialport_read(int fd, unsigned char* buf,int size,int timeout_ms){
	if(INVALID_HANDLE_VALUE!=(HANDLE)fd){
		int ret;		
		COMMTIMEOUTS cto;
		DWORD dwBytesRead;
		GetCommTimeouts((HANDLE)fd,&cto);
		cto.ReadTotalTimeoutMultiplier = (0==timeout_ms)?0:timeout_ms/size;
		SetCommTimeouts((HANDLE)fd,&cto);
		ret = ReadFile((HANDLE)fd,buf,size,&dwBytesRead,0);
		if(!ret)
 			return -EIO;
 		return (int)dwBytesRead;
	}

	return -EINVAL;
}

int serialport_flush(int fd, int type){
	if(INVALID_HANDLE_VALUE!=(HANDLE)fd){
		if(!type) type =  PURGE_TXCLEAR | PURGE_RXCLEAR;
		else if(1==type) type = PURGE_TXCLEAR;
		else type = PURGE_RXCLEAR;
		PurgeComm((HANDLE)fd,type);
		return 0;
	}
	return -EINVAL;
}

int serialport_config(int fd,int baudrate,int databits,int stopbits,int parity)
{
	int status;
	DCB dcb={0};

  
  	dcb.DCBlength=sizeof(dcb);
  	if (!GetCommState(fd, &dcb)) {
   		printf("Error Getting State\n");  
   		return -ENOENT;
  	}
	switch(baudrate)
    {	
		case 110: baudrate = CBR_110;break;
		case 300: baudrate = CBR_300;break;
		case 600: baudrate = CBR_600;break;
		case 1200: baudrate = CBR_1200;break;
		case 2400: baudrate = CBR_2400;break;
		case 4800: baudrate = CBR_4800;break;
		case 9600: baudrate = CBR_9600;break;
		case 14400: baudrate = CBR_14400;break;
		case 19200: baudrate = CBR_19200;break;
		case 38400: baudrate = CBR_38400;break;
		case 56000: baudrate = CBR_56000;break;
		case 57600: baudrate = CBR_57600;break;
		case 115200: baudrate = CBR_115200;break;
		case 128000: baudrate = CBR_128000;break;
		case 256000: baudrate = CBR_256000;break;
    }
        
    switch(databits)
    {    
    	case 5:	databits=5;	break;
		case 6: databits=6; break;
        case 7: databits=7; break;
        case 8:	default: databits = 8; break;
    } 
    switch(parity)
    {
    	default:
		case 0:
        case 'n':                 /*无校验*/
        case 'N':
			parity = NOPARITY;
             break;
		case 1:
        case 'o':                 /*奇检验*/
        case 'O':
			parity = ODDPARITY;
            break;
		case 2:
        case 'e':                 /*偶校验*/
        case 'E':
			parity = EVENPARITY;
             break;
        case 's':                /*Space校验*/
        case 'S':
			parity = SPACEPARITY;
             break;
    }
    switch(stopbits)
    {
    	default:
        case 1:
            stopbits = ONESTOPBIT;
            break;
        case 2:
            stopbits = TWOSTOPBITS;
            break;
    }
    //设置停止位结束
    
     //fill up params
  	dcb.BaudRate=baudrate;
  	dcb.ByteSize=databits;
  	dcb.StopBits=stopbits;
  	dcb.Parity=parity;

	 if(!SetCommState((HANDLE)fd, &dcb)){
		/* Error occurred. Inform user */ 
		printf("Error getting state\n"); 
		return -EINVAL;
	}

       
     return 0;

}
#warning "FIXME:loading dynamic library on windows"
void* LoadSharedLibrary(const char* path,int flags){
	return 0;
}
void ReleaseSharedLibrary(void* handle){
	
}
void* LoadSymbol(void* handle,const char* symbol){
	return 0;
}

void 
	platform_program_exit(int code){
	exit(0);
}
//platform usleep
void platform_usleep(int usec){
	/*
	HANDLE timer; 
    LARGE_INTEGER ft; 

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time

    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer); */
    usleep(usec);
}


//return file size	
long 
	platform_filesize(const char* filename){
    BOOL                        fOk;
    WIN32_FILE_ATTRIBUTE_DATA   fileInfo;

    if (NULL == filename)
        return -1;

    fOk = GetFileAttributesEx(filename, GetFileExInfoStandard, (void*)&fileInfo);
    if (!fOk)
        return -1;
    if(0 != fileInfo.nFileSizeHigh)
		return 0;
	
    
    return (long)fileInfo.nFileSizeLow;
}
//return buffer size and buffer hold by platform,user must release buffer by 
//platform_releasebuffer	
long 
	platform_readfile2buffer(const char* filename,int alignment/*align buffer into size,0 means unused*/,char** buffer){
	FILE* handle;
	long ret;
	char* filebuffer=NULL;
	long alignsize;
	long filesize = platform_filesize(filename);
	if(filesize<0)
		return filesize;
	if(alignment)
		alignsize = alignment_up(filesize,alignment);
	else
		alignsize = filesize;
	filebuffer = (char*)malloc(sizeof(char)*alignsize);
	if(NULL==filebuffer) 
		return -ENOMEM;
	handle = fopen(filename,"rb");
	if(NULL==handle) 
		return -ENOENT;
	ret = fread(filebuffer,1,filesize,handle);
	if(ret!=filesize){
		free(filebuffer);
		return -EIO;
	}

	fclose(handle);

	*buffer = filebuffer;
	
	return alignsize;
}
//release buffer allocated by platform_readfile2buffer	
void 
	platform_releasebuffer(char* buffer){
	free(buffer);
}


void* 
	platform_createthread(platform_threadfunc entry,void* thread_arg){
	HANDLE threadhandle = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)entry,thread_arg,0,NULL);
	return threadhandle;
}
int
	platform_terminatethread(void* threadhandle){
	int ret = TerminateThread(threadhandle,0);
	CloseHandle((HANDLE)threadhandle);
	return ret;
}

