#ifndef HEX2BIN_H
#define  HEX2BIN_H
enum {
	eHex2BinSuccess=0,
	eHex2BinErrRecordChecksum,
	eHex2BinErrRecordUnsupported,
	eHex2BinErrOutOfBuffer,
};
int hex2bin(unsigned char* pInBuffer,unsigned int inlen,unsigned char* pOutBuffer,unsigned int *outlen);

#endif
