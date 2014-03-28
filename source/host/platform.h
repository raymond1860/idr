#ifndef PLATFORM_H
#define PLATFORM_H
//Standard C library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

typedef unsigned char uint8;
typedef unsigned short uint16;

typedef struct cli_menu_shared{
	char* devname;
	int  baudrate;
}cli_menu_shared;
extern cli_menu_shared shared;

#define alignment_down(a, size) ((a/size)*size)
#define alignment_up(a, size) (((a+size-1)/size)*size)

#if defined(_WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <errno.h>
#include <ctype.h>

typedef int bool;
#define false 0
#define true !false
#define DEFAULT_PORT "COM34"
#elif defined(__linux__ )
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <termios.h>
#include <stdbool.h>
#include <errno.h>
#include <dlfcn.h>
#define DEFAULT_PORT "/dev/ttyUSB0"


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif

typedef int BOOL;
#else
#error "unknown platform"
#endif

//platform 
void 
	platform_program_exit(int code);
void 
	platform_usleep(int usec);
//return file size	
long 
	platform_filesize(const char* filename);
//return buffer size and buffer hold by platform,user must release buffer by 
//platform_releasebuffer	
long 
	platform_readfile2buffer(const char* filename,int alignment/*align buffer into size,0 means unused*/,char** buffer);
//release buffer allocated by platform_readfile2buffer	
void 
	platform_releasebuffer(char* buffer);

//thread operation
typedef void* (*platform_threadfunc)(void* arg);
void* 
	platform_createthread(platform_threadfunc entry,void* thread_arg);
int
	platform_terminatethread(void* threadhandle);

//dynamic library loading
void* LoadSharedLibrary(const char* path,int flags);
void ReleaseSharedLibrary(void* handle);
void* LoadSymbol(void* handle,const char* symbol);



//utility
void dump(const char *prefix,const unsigned char *data ,int size);



/*serial port API*/

int serialport_open(const char* name);

int serialport_close(int fd);
/*
 * baudrate:300,600,1200,2400,4800,9600,19200,38400,115200
 * databits:5,6,7,8
 * stopsbits:1,2
 * parity:none,even,odd
 *
 * example:12008N1
 * serialport_set(fd,1200,8,1,'n'); or serialport_set(fd,1200,8,1,0);
*/
int serialport_config(int fd,int baudrate,int databits,int stopbits,int parity);

/*
 * return written bytes,otherwise return error code
*/
int serialport_write(int fd,unsigned char* buf,int size);

/*
* return read bytes,otherwise return error code
*/
int serialport_read(int fd, unsigned char* buf,int size,int timeout_ms);

/*
 * type:
 * 0 -> flush in and out buffer
 * 1 -> flush out buffer
 * 2 -> flush in buffer
*/
int serialport_flush(int fd, int type);

#endif
