#ifndef PACKET_H
#define PACKET_H


#define PERR_NONE 		0
#define PERR_CRC  		-1
#define PERR_INVALID	-2

//packet protocol,0->stanard SAM packet,1->vendor private packet
#define PPROT_SAM		0
#define PPROT_PRIV		1

typedef struct st_general_packet{
	char* pbuf;
	int plen;
}GenPacket;

int packet_protocol(GenPacket* p);

#endif