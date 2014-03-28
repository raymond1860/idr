#ifndef TRANSFER_H
#define TRANSFER_H 1

#define MAX_TIMEOUT_MS 10000
#define DEF_XFER_TIMEOUT_MS 2000

#define XFER_TYPE_OUT_IN    0x00  /*request and response*/
#define XFER_TYPE_OUT_ONLY 0x01  /*request only*/

#define DEF_XFER_SERIAL_BAUDRATE 115200

struct mcu_xfer;
typedef int (*xfer_packet_impl)(struct mcu_xfer* xfer);

typedef struct serial_settings{
	int baudrate;
	int databits;
	int stopbits;
	int parity;
}serial_settings;
typedef struct port_property{
	unsigned char port_type;
	union{
		unsigned int reserved;
		serial_settings serial;
	}port_setting;
}port_property;
typedef struct mcu_xfer{
	unsigned char xfer_type; //type 
	
	int  		   xfer_to;   //timeout,0->return immediately,-1,use default max timeout,all timeout values over max,will be cut to max

	unsigned char* req;
	unsigned int   reqsize;

	unsigned char* resp;
	unsigned int   respsize;

	xfer_packet_impl xfer_impl;


	//internal use?
	const char* xfer_port;
	port_property port_prop;
}mcu_xfer;

//return value
//0->success,otherwise failed
int default_xfer_packet(mcu_xfer* xfer);


int submit_xfer(const char* xfer_port,port_property* port_prop,mcu_xfer* xfer);



#endif
