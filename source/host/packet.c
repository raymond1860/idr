#include "platform.h"
#include "packet.h"


static  uint8 checksum(uint8* start,int size){
	uint8 crc8=0;
	int i;
	for ( i = 0 ; i < size ; i ++ ) {
   		crc8 = crc8 ^ start[i];
	}
	return crc8;
}

int setup_vendor_packet(uint8* pbuf,uint16 max_plen,uint8* payload,uint16 payload_len){	
	if(max_plen<(payload_len+5))
		return PERR_MEM;
	pbuf[0] = VENDOR_PACKET_PREFIX;
	pbuf[1] = (payload_len&0xff00)>>8;
	pbuf[2] =  payload_len&0xff;
	memcpy(pbuf+3,payload,payload_len);
	pbuf[3+payload_len] = checksum(payload,payload_len);
	pbuf[4+payload_len] = VENDOR_PACKET_SUFFIX;
	return (payload_len+5);
}

int setup_vendor_payload(uint8* buf,uint16 buf_len,uint8 cmd,uint8 sub_cmd, uint8 params_num, ...){
	va_list argp;
	int arg_num=0;
	uint8 param;
	
	//check minimum buf size
	if(buf_len<2) goto failure;
		
	//fill 
	*buf++=cmd;
	*buf++=sub_cmd;
	va_start( argp, params_num );
	for(;arg_num<params_num;arg_num++){
		if(arg_num>(buf_len-2)) goto failure;
		param=(uint8)va_arg(argp, int);
		*buf++=param;		
	}
	va_end( argp );  
	return (2+arg_num); 
failure:
	return PERR_MEM;	
}

int xfer_packet_wrapper(const char* dev,uint8* resp,int respsize,uint8 cmd,uint8 sub_cmd,uint8 params_num, ...){
	uint8 packetbuf[128];
	uint8 payloadbuf[64];
	int packetlen,payloadlen;
	uint8* buf;
	va_list argp;
	int arg_num=0;
	uint8 param;
	mcu_xfer xfer;
	
	//setup payload
	buf=payloadbuf;
	*buf++=cmd;
	*buf++=sub_cmd;
	va_start( argp, params_num );
	for(;arg_num<params_num;arg_num++){
		if(arg_num>(16-2)) goto failure;
		param=(uint8)va_arg(argp, int);
		*buf++=param;		
	}
	va_end( argp );  
	payloadlen=2+arg_num; 

	packetlen=setup_vendor_packet(packetbuf,128,payloadbuf,payloadlen);

	xfer.xfer_to = DEF_XFER_TIMEOUT_MS;
	xfer.req = packetbuf;
	xfer.reqsize = packetlen;
	xfer.xfer_type = resp?XFER_TYPE_OUT_IN:XFER_TYPE_OUT_ONLY;
	xfer.xfer_port = dev;
	xfer.resp = resp;
	xfer.respsize = respsize;
	xfer.xfer_impl = 0;

	return submit_xfer(dev,NULL,&xfer);	
	
failure:
	return PERR_MEM;	
}
int xfer_packet_wrapper_w_xferimpl(const char* dev,xfer_packet_impl xfer_impl,uint8* resp,int respsize,uint8 cmd,uint8 sub_cmd,uint8 params_num, ...){
	uint8 packetbuf[128];
	uint8 payloadbuf[64];
	int packetlen,payloadlen;
	uint8* buf;
	va_list argp;
	int arg_num=0;
	uint8 param;
	mcu_xfer xfer;
	
	//setup payload
	buf=payloadbuf;
	*buf++=cmd;
	*buf++=sub_cmd;
	va_start( argp, params_num );
	for(;arg_num<params_num;arg_num++){
		if(arg_num>(16-2)) goto failure;
		param=(uint8)va_arg(argp, int);
		*buf++=param;		
	}
	va_end( argp );  
	payloadlen=2+arg_num; 

	packetlen=setup_vendor_packet(packetbuf,128,payloadbuf,payloadlen);

	xfer.xfer_to = DEF_XFER_TIMEOUT_MS;
	xfer.req = packetbuf;
	xfer.reqsize = packetlen;
	xfer.xfer_type = resp?XFER_TYPE_OUT_IN:XFER_TYPE_OUT_ONLY;
	xfer.xfer_port = dev;
	xfer.resp = resp;
	xfer.respsize = respsize;
	xfer.xfer_impl = xfer_impl;

	return submit_xfer(dev,NULL,&xfer);	
	
failure:
	return PERR_MEM;	
}

