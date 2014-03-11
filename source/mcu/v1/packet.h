#ifndef PACKET_H
#define PACKET_H


#define PERR_NONE 		0
#define PERR_CRC  		-1
#define PERR_INVALID	-2
#define PERR_MEM		-3

//packet protocol,0->stanard SAM packet,1->vendor private packet
#define PPROT_SAM		0
#define PPROT_PRIV		1

typedef struct st_general_packet{
	char* pbuf;
	int plen;
}GenPacket;

int packet_protocol(GenPacket* p);

//Vendor private definition
#define VENDOR_PACKET_PREFIX 0x02
#define VENDOR_PACKET_SUFFIX 0x03
#define CMD_CLASS_COMM  	0x30
#define CMD_CLASS_READER	0x31
#define CMD_CLASS_CARD		0x32
#define CMD_CLASS_EXT		0x33

#define STATUS_CODE_SUCCESS 					0x0000

#define STATUS_CODE_CARD_NOT_SUPPORTED			((0x10<<8)+0x01)
#define STATUS_CODE_CARD_NOT_INSERTED			((0x10<<8)+0x02)
#define STATUS_CODE_CARD_NOT_POWERED			((0x10<<8)+0x04)
#define STATUS_CODE_CARD_NOT_ANSWERED			((0x10<<8)+0x06)
#define STATUS_CODE_CARD_NOT_VALIDDATA			((0x10<<8)+0x07)
#define STATUS_CODE_CARD_PSAM_NOT_SUPPORTED		((0x20<<8)+0x01)
#define STATUS_CODE_CARD_PSAM_NOT_POWERED		((0x20<<8)+0x04)
#define STATUS_CODE_CARD_PSAM_NOT_ANSWERED		((0x20<<8)+0x06)
#define STATUS_CODE_CARD_PSAM_NOT_VALIDDATA		((0x20<<8)+0x07)
#define STATUS_CODE_CARD_NCC_NOT_SUPPORTED		((0x30<<8)+0x01)
#define STATUS_CODE_CARD_NCC_NOT_ACTIVE			((0x30<<8)+0x04)
#define STATUS_CODE_CARD_NCC_NOT_ANSWERED		((0x30<<8)+0x06)
#define STATUS_CODE_CARD_NCC_NOT_VALIDDATA		((0x30<<8)+0x07)

int setup_vendor_packet(uint8* pbuf,uint16 max_plen,uint8* payload,uint16 payload_len);


#endif