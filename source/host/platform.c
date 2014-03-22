#include "platform.h"

#if defined(_WIN32)
int serialport_open(const char* name){
 	COMMTIMEOUTS timeouts={0};
	int fd;
	char devname[256];
	
	
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
	timeouts.ReadIntervalTimeout=MAXDWORD;
	timeouts.ReadTotalTimeoutConstant=1;
	timeouts.ReadTotalTimeoutMultiplier=1;
	timeouts.WriteTotalTimeoutConstant=1;
	timeouts.WriteTotalTimeoutMultiplier=1;

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
		cto.ReadTotalTimeoutMultiplier = (0==timeout_ms)?0:1;
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
#elif defined(__linux__)
#define DEF_SERIALPORT "/dev/ttyS1"
int serialport_open2(const char* name,int flags){
	if(!name)
		name = DEF_SERIALPORT;
	return open(name,flags);
}
int serialport_open(const char* name){
	return serialport_open2(name,O_RDWR | O_NOCTTY | O_NDELAY);
}


int serialport_close(int fd){
	if(fd>0)
		close(fd);
	return 0;
}

int serialport_write(int fd,unsigned char* buf,int size){	
	if(fd>=0)
		return write(fd,buf,size);
	return -EINVAL;
}
int serialport_read(int fd, unsigned char* buf,int size,int timeout_ms){
	if(fd>=0){
		int ret;
		struct timeval t_start,t_end; 
		long  time_start,time_end =0; //ms
		if(!timeout_ms)
			return read(fd,buf,size);
		gettimeofday(&t_start, NULL); 
		time_start = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000; 

		while(1)
		{
			gettimeofday(&t_end, NULL); 
			time_end = ((long)t_end.tv_sec)*1000+(long)t_end.tv_usec/1000;

			if((time_end - time_start) >= timeout_ms)
			{
				return -EIO;
			}
			usleep(10000);
			ret = read(fd,buf,size);
			if(ret <0)
			{
				usleep(10000);
				continue;
			}
		}
		return ret;
	}
	return -EINVAL;
}

int serialport_flush(int fd, int type){
	if(fd>=0){
		tcflush(fd,type);
	}
	return -EINVAL;
}

int serialport_config(int fd,int baudrate,int databits,int stopbits,int parity)
{
	int status;
	struct termios set_port;
	struct termios old_port;
	if(tcgetattr(fd,&old_port) != 0)//得到机器源端口的默认设置
	{		
		return -ENOENT;
	} 

	memset(&set_port,0,sizeof(set_port));  
    set_port.c_cflag |= CLOCAL | CREAD;//激活CLOCAL，CREAD用于本地连接和接收使能
    tcflush(fd,TCIOFLUSH);
        switch(baudrate)
    {	
		case 300:
			cfsetispeed(&set_port,B300);//分别设置输入和输出速率
			cfsetospeed(&set_port,B300);
			break;
		case 600:
			cfsetispeed(&set_port,B600);//分别设置输入和输出速率
			cfsetospeed(&set_port,B600);
			break;
		case 1200:
			cfsetispeed(&set_port,B1200);//分别设置输入和输出速率
			cfsetospeed(&set_port,B1200);
		break;		
		case 2400:
			cfsetispeed(&set_port,B2400);//分别设置输入和输出速率
			cfsetospeed(&set_port,B2400);
		break;
		case 4800:
            cfsetispeed(&set_port,B4800);//分别设置输入和输出速率
            cfsetospeed(&set_port,B4800);
            break; 
		default:
        case 9600:
            cfsetispeed(&set_port,B9600);//分别设置输入和输出速率
            cfsetospeed(&set_port,B9600);
            break;
        case 19200:
            cfsetispeed(&set_port,B19200);
            cfsetospeed(&set_port,B19200);
            break;
		case 38400:
            cfsetispeed(&set_port,B38400);//分别设置输入和输出速率
            cfsetospeed(&set_port,B38400);
            break;
		case 115200:
			cfsetispeed(&set_port,B115200);//分别设置输入和输出速率
			cfsetospeed(&set_port,B115200);
		break;

    }
        
    /*设置比特率结束*/
        /*********设置数据位**********/
    set_port.c_cflag &= ~CSIZE;
    switch(databits)
    {    
    	case 5:
			set_port.c_cflag |= CS5;
			break;
		case 6:
			set_port.c_cflag |= CS6;
			break;			
        case 7:
            set_port.c_cflag |= CS7;
            break;
        case 8:
		default:
            set_port.c_cflag |= CS8;
            break;
    }
    /********设置数据位结束*******/
        /*********设置校验位*********/
        switch(parity)
    {
    	default:
		case 0:
        case 'n':                 /*无校验*/
        case 'N':
            set_port.c_cflag &= ~PARENB;
            set_port.c_iflag &= ~INPCK;
            break;
		case 1:
        case 'o':                 /*奇检验*/
        case 'O':
            set_port.c_cflag |= (PARODD | PARENB);
            set_port.c_iflag |= INPCK;
            break;
		case 2:
        case 'e':                 /*偶校验*/
        case 'E':
            set_port.c_cflag |= PARENB;
            set_port.c_cflag &= ~PARODD;
            set_port.c_iflag |= INPCK;
            break;
        case 's':                /*Space校验*/
        case 'S':
            set_port.c_cflag &= ~PARENB;
            set_port.c_cflag &= ~CSTOPB;
            break;
    }
    if(parity != 'n' && parity != 0)
	    set_port.c_iflag |= INPCK;
    /**********停止位***********/
        switch(stopbits)
    {
    	default:
        case 1:
            set_port.c_cflag &= ~CSTOPB;
            break;
        case 2:
            set_port.c_cflag |= CSTOPB;
            break;
    }
    //设置停止位结束
       
        /*关掉ICRNL和IXON功能，使能接受二机制字符*/
    set_port.c_iflag &= ~(ICRNL | IXON);
    /*设置看控制时间*/
    set_port.c_lflag &=~ICANON;//设置串口为原始模式，在原始模式下下面两个字段才有效
    set_port.c_cc[VTIME]=0;
    set_port.c_cc[VMIN]=0;
    tcflush(fd,TCIOFLUSH);
    //刷新输入缓存或者输出缓存TCIFLUSH输入队列，TCIOFLUSH输入输出队列
    if(tcsetattr(fd,TCSANOW,&set_port)!=0)//立刻将设置写道串口中去
    {
        return -EIO;
    }
	
    return 0;

}


#else
#error "unknown platform"
#endif
