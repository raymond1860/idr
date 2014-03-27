#include "platform.h"
#include "transfer.h"


int default_xfer_packet(mcu_xfer* xfer){
	int err=0;
	int writelength,writetotal;
	int readlength,readtotal;
	writelength=writetotal=readlength=readtotal=0;
	int handle = serialport_open(xfer->xfer_port);
	if(handle<0){
		return -ENOENT;
	}
	while(1)
	{
		writelength = serialport_write(handle, xfer->req, xfer->reqsize) ;
		if(writelength <= 0){err=-EIO;goto failed;}
		writetotal +=  writelength;
		if(xfer->reqsize == writetotal) break;
	}
	

	if(XFER_TYPE_OUT_IN==xfer->xfer_type){		
		if(xfer->xfer_to>MAX_TIMEOUT_MS)
			xfer->xfer_to = MAX_TIMEOUT_MS;
		printf("read port %s in timeout %d ms\n",xfer->xfer_port,xfer->xfer_to);
		//wait for response
		readlength = serialport_read(handle, xfer->resp+readtotal,xfer->respsize-readtotal,xfer->xfer_to) ;
		printf("0x%x\n",readlength);
		if(readlength<=0){err = -EIO;goto failed;}
	}
	
failed:
	if(handle>=0)
		serialport_close(handle);
	return err;
}

int submit_xfer(const char* xfer_port,mcu_xfer* xfer){
	xfer->xfer_port = xfer_port;
	if(!xfer->xfer_impl)
		xfer->xfer_impl = default_xfer_packet;
	return xfer->xfer_impl(xfer);
}


