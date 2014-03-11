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
	uint8 xor=0;
	int i;
	for ( i = 0 ; i < size ; i ++ ) {
   		xor = xor ^ start[i];
	}
	return xor;
}


//with less security checking???
int packet_protocol(GenPacket* p){
	uint8* buf = p->pbuf;
	unsigned int payload_len;
	//check header
	if(p->plen<3) return PERR_INVALID;
	if(0x02==buf[0]){
	    payload_len = (buf[1]<<8)+buf[2];
		if(payload_len!=(p->plen-5)){
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
