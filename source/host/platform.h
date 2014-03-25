#ifndef PLATFORM_H
#define PLATFORM_H
//Standard C library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>




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
#else
#error "unknown platform"
#endif


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
