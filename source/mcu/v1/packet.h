#ifndef PACKET_H
#define PACKET_H


#define PERR_NONE 		0
#define PERR_CRC  		-1
#define PERR_INVALID	-2
#define PERR_MEM		-3

//packet protocol,0->stanard SAM packet,1->vendor private packet
#define PPROT_SAM		0
#define PPROT_PRIV		1


int packet_protocol(unsigned char* buf,unsigned int len);

//Vendor private definition
#define VENDOR_PACKET_PREFIX 0x02
#define VENDOR_PACKET_SUFFIX 0x03
#define CMD_CLASS_COMM  	0x30
#define CMD_CLASS_READER	0x31
#define CMD_CLASS_CARD		0x32
#define CMD_CLASS_EXT		0x33
#define CMD_CLASS_MCU		0x40


//CMD_CLASS_MCU sub commands
#define MCU_SUB_CMD_RESET 0x10
#define MCU_RESET_TYPE_NORMAL 0x00
#define MCU_RESET_TYPE_ISP 		0x01
/*
  Command format
  -----------------------------------------------------
  |CMD_CLASS_MCU |MCU_SUB_CMD_RESET | PARAMS          |
  ----------------------------------------------------
  | 0x40         |0x10              | 2bytes         |
  ---------------------------------------------------
  params:
  byte 1: reset type
  0  --> normal reset
  1  --> reset to isp support
  byte 2: delay time(second) to reset
  
  Response format
  ----------------
  |STATUS Code 	 |
  ---------------
  |2bytes       |
  --------------
  0x00,0x00 ,Okay  
*/
#define MCU_SUB_CMD_FIRMWARE_VERSION 0x11
/*
  Command format
  ----------------------------------------------
  |CMD_CLASS_MCU |MCU_SUB_CMD_FIRMWARE_VERSION |
  ---------------------------------------------
  | 0x40         |0x11                        |
  --------------------------------------------  
  Response format
  ----------------------------------
  |Status Code 	 |firmware version |
  ----------------------------------
  |2bytes       |1bytes           |
  ---------------------------------
  Status code:
  0x00,0x00,Okay,firmware version is valid
  
  firmware version:
  BCD encoding,e.g.
  v1.0->0x10
  v1.1->0x11
*/


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