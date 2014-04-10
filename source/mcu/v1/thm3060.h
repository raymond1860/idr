#ifndef THM3060_H
#define	THM3060_H 1
/*THM3060 register definition*/
#define DATA        0x00
#define PSEL        0x01
#define FCONB       0x02
#define EGT         0x03
#define CRCSEL      0x04
#define RSTAT       0x05
#define SCNTL       0x06
#define INTCON      0x07
#define RSCH        0x08
#define RSCL        0x09    
#define CRCH        0x0a
#define CRCL        0x0b
#define TMRH        0x0c
#define TMRL        0x0d
#define BPOS        0x0e
#define SMOD        0x10
#define PWTH        0x11

#define SCON_60		0x06

//#define RSTAT bits
#define FEND        0x01
#define CRCERR      0x02
#define TMROVER     0x04
#define DATOVER     0x08
#define FERR        0x10
#define PERR        0x20
#define CERR        0x40

#define TYPE_A      0x10
#define TYPE_B      0x00
#define ISO15693    0x20
#define ETK         0x30
#define MIFARE      0x50
#define SND_BAUD_106K   0x00
#define SND_BAUD_212K   0x04
#define SND_BAUD_424K   0x08
#define SND_BAUD_848K   0x0c

#define RCV_BAUD_106K   0x00
#define RCV_BAUD_212K   0x01
#define RCV_BAUD_424K   0x02
#define RCV_BAUD_848K   0x03


    unsigned char THM_Anticollision(unsigned char * b_uid);

    unsigned char THM_ISO14443_B(unsigned char * b_uid);
    unsigned char THM_Anticollision2(unsigned char * anti_flag,unsigned short * uid_len_all,unsigned char * b_uid);
    unsigned char THM_Anti(unsigned char cmm_head , unsigned short * uid_len,unsigned char * b_uid);

#endif
