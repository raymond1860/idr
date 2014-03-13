#ifndef IDENTIFYCARD_H
#define IDENTIFYCARD_H
#ifdef __cplusplus
extern "C" {
#endif

typedef struct tag_ID2Info
{
	char name[30];
	char sex[2];
	char native[4];
	char birthday[16];
	char address[70];
	char id[36];
	char authority[30];
	char period_start[16];
	char period_end[16];
	char new_address[36];
	char imgage[1024+64];
}ID2Info;

/*
 * return value:
 * 	-1 - argument error or open xh_misc error
 * 	0  - success
 */

int libid2_open(char *uart_name);
/*
 *  
 *
 *description 
 *samid -- samid content,malloc by user,suggest malloc 128 bytes.
 *the content do not contain the head.the head is "\xAA\xAA\xAA\x96\x69\xxx\xxx".
 *samlenth -- samid length,return by function
 *time_out -- time out of the funtion ,Unit s.
 *
 * return value:
 * 	-1 - argument error or open xh_misc error
 * 	0  - success
 */

int libid2_getsamid(char *samid,int *samlenth,int time_out);

/*
*description 
*id2info -- id2 information ,malloc by user,suggest malloc 2048 bytes.
*the content do not contian the head.the head is "\xAA\xAA\xAA\x96\x69\xxx\xxx".
*id2infolength -- the length of id2 information,return by function
*time_out -- time out of the funtion ,Unit s.
*
* return value:
*  -1 - argument error or open xh_misc error
*  0  - success
*/

int libid2_read_id2_info(char *id2info,int *id2infolength,int time_out);

void  libid2_close(void);

/*
*description 
*if use this function.must be wait 2~3s,then used the other function;
*
*

 * return value:
 * 	-1 - argument error or open xh_misc error
 * 	0  - success
 */
int libid2_power(int on_off);
int libid2_reset(void);

/*
 * return value:
 * 	-1 - failed
 * 	1 - power 0f;
 *     2  - power sleep;
 *     3 - STATE_WORKINGl
 *
 */

int libid2_get_power_state(void);

int libid2_decode_info(char *id2infobuf,int length,ID2Info *info);

int libid2_getICCard(int DelayTime,int * aCardType,char * CardId);


#ifdef __cplusplus
}
#endif

#endif

