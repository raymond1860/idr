#ifndef SECURE_H
#define SECURE_H 1
    void init_i2c();
	unsigned char read_sec(unsigned char idata * dat);
	void  write_sec(unsigned char * dat,unsigned short len);
#endif