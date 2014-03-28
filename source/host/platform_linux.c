#include "platform.h"
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

void* LoadSharedLibrary(const char* path,int flags){
	if(!flags) flags=RTLD_NOW;
	return dlopen(path,flags);
}
void ReleaseSharedLibrary(void* handle){
	dlclose(handle);
}
void* LoadSymbol(void* handle,const char* symbol){
	return dlsym(handle,symbol);
}

void 
	platform_program_exit(int code){
	exit(0);
}
//platform usleep
void platform_usleep(int usec){
	usleep(usec);
}

//return file size	
long 
	platform_filesize(const char* filename){
	long size=-1;
	struct stat st;
	if(!stat(filename, &st))
		size = st.st_size;
	return size;
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
	pthread_attr_t attr;
	pthread_t threadhandle;
	int ret;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&threadhandle,&attr,entry,thread_arg);
	if(ret){
		printf("failed to create thread\n");
		return NULL;
	}
	return (void*)threadhandle;
}
int
	platform_terminatethread(void* threadhandle){
	int ret = pthread_cancel((pthread_t)threadhandle);
	return ret;
}
