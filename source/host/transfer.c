#include "platform.h"
#include "transfer.h"


int default_xfer_packet(mcu_xfer* xfer){
	int err=0;
	int writelength,writetotal;
	int readlength,readtotal;
	writelength=writetotal=readlength=readtotal=0;
	int handle = serialport_open(xfer->xfer_port);
	if(handle<0){
		printf("open port %s failed\n",xfer->xfer_port);
		return -ENOENT;
	}

	serialport_flush (handle, 0);
	//dumpdata("xfer req",xfer->req,xfer->reqsize);

	//set port prop 
	serialport_config(handle,
		xfer->port_prop.port_setting.serial.baudrate,
		xfer->port_prop.port_setting.serial.databits,
		xfer->port_prop.port_setting.serial.stopbits,
		xfer->port_prop.port_setting.serial.parity);
	while(1)
	{
		writelength = serialport_write(handle, xfer->req, xfer->reqsize) ;
		if(writelength <= 0){
			printf("failed to write to port %s\n",xfer->xfer_port);
			err=-EIO;
			goto failed;
		}		
		writetotal +=  writelength;
		if(xfer->reqsize == writetotal) break;
	}
	

	if(XFER_TYPE_OUT_IN==xfer->xfer_type){	
		struct timeval t_start,t_end; 
		long ltimestart,ltimeend;
		if(xfer->xfer_to>MAX_TIMEOUT_MS)
			xfer->xfer_to = MAX_TIMEOUT_MS;	
		//wait for response
		gettimeofday(&t_start, NULL); 
		ltimestart = ((long)t_start.tv_sec)*1000+(long)t_start.tv_usec/1000; 
		do {
			platform_usleep(100*1000);
			readlength = serialport_read(handle, xfer->resp+readtotal,xfer->respsize-readtotal,0) ;		
			if(readlength>0)
				readtotal+=readlength;
			gettimeofday(&t_end, NULL); 
			ltimeend = ((long)t_end.tv_sec)*1000+(long)t_end.tv_usec/1000;
			if((ltimeend - ltimestart) >= xfer->xfer_to){
				//printf("read timeout\n");
				err = -EIO;goto failed;
			}
							
		}while(readtotal<5);//5 is packet minimum size
		
					
		//dumpdata("xfer resp",xfer->resp,readtotal);
		xfer->respsize = (unsigned int)readtotal;
	}
	
failed:
	if(handle>=0)
		serialport_close(handle);
	return err;
}

int submit_xfer(const char* xfer_port,port_property* port_prop,mcu_xfer* xfer){
	xfer->xfer_port = xfer_port;
	if(port_prop&&port_prop->port_setting.serial.baudrate)
		xfer->port_prop.port_setting.serial.baudrate = port_prop->port_setting.serial.baudrate;
	else if(shared.baudrate)
		xfer->port_prop.port_setting.serial.baudrate = shared.baudrate;
	else 
		xfer->port_prop.port_setting.serial.baudrate = DEF_XFER_SERIAL_BAUDRATE;
	if(port_prop&&port_prop->port_setting.serial.databits)
		xfer->port_prop.port_setting.serial.databits = port_prop->port_setting.serial.databits;
	else
		xfer->port_prop.port_setting.serial.databits = 8;

	if(port_prop&&port_prop->port_setting.serial.stopbits)
		xfer->port_prop.port_setting.serial.stopbits = port_prop->port_setting.serial.stopbits;
	else
		xfer->port_prop.port_setting.serial.stopbits = 1;
	
	if(port_prop&&port_prop->port_setting.serial.parity)
		xfer->port_prop.port_setting.serial.parity = port_prop->port_setting.serial.parity;
	else
		xfer->port_prop.port_setting.serial.parity = 'n';
	if(!xfer->xfer_impl)
		xfer->xfer_impl = default_xfer_packet;
	return xfer->xfer_impl(xfer);
}


