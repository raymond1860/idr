#include <string.h>
#include "common.h"
#include "packet.h"
/*
 * SAM packet protocol ,See details in GA467-2004
 * UART In 
 * |Preamble  |Len1  |Len2  |CMD  |Param  |Data  |CHK_SUM| 
*/

/*
 * Vendor private packet protocol 
 * UART In 
 * |STX  |Len  |Data  |LRC  |ETX| 
 * STX  -->0x02
 * Len  -->Big endian 16bit length
 * Data -->Variable
 * LRC	-->Xor checksum of data
 * ETX	-->0x03
*/
static  uint8 checksum(uint8* start,int size){
	uint8 crc8=0;
	int i;
	for ( i = 0 ; i < size ; i ++ ) {
   		crc8 = crc8 ^ start[i];
	}
	return crc8;
}


//with less security checking???
int packet_protocol(unsigned char* buf,unsigned int len){
	//check header
	if(len<3) return PERR_INVALID;
	if(0x02==buf[0]){
	    uint8 payload_len = (buf[1]<<8)+buf[2];
		if(payload_len!=(len-5)){
		    return PERR_INVALID;
		}
		if(buf[4+payload_len]!=0x03)
			return PERR_INVALID;
		//now check checksum
		if(buf[3+payload_len]!=checksum(buf+3,payload_len))
			return PERR_CRC;
		return PPROT_PRIV;
	}else {
		//SAM packet branch
		if((buf[0]!=0xAA)||
			(buf[1]!=0xAA)||
			(buf[2]!=0xAA)||
			(buf[3]!=0x96)||
			(buf[4]!=0x69)){
			return PERR_INVALID;
		}
		//should we check all fields?
		return PPROT_SAM;
	}
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
