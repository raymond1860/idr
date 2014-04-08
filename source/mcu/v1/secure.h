#ifndef SECURE_H
#define SECURE_H 1

#define SAM_CMD_RESET 			0x10
#define SAM_CMD_READ_STATUS		0x11
#define SAM_CMD_READ_SAMID		0x12
#define SAM_CMD_FIND_CARD		0x20
#define SAM_CMD_SELECT_CARD		0x20
#define SAM_CMD_READ_CARD		0x30


    void init_i2c();
	unsigned char read_sec(unsigned char idata * dat);
	void  write_sec(unsigned char * dat,unsigned short len);
#endif
