/*
 *All Rights Reserved 2014, XingHuo Info Inc,.
*/
#include <stdio.h>
#include "spi.h"
#include "common.h"
#include "thm3060.h"

// 通过 SPI 总线写 address 寄存器的值
//write REG bu spi
extern void THM_WriteReg(unsigned char address,unsigned char content)
{
	#if 0
	unsigned char temp_buff[2];
	// 写命令 BIT7 =1	
	//write mode BIT7 = 1
	temp_buff[0] = address | 0x80;
	temp_buff[1] = content;
	// SPI 帧
	//SPI frame
	SPI_FRAME_START();	
	SPI_SendBuff(temp_buff,2);	
	SPI_FRAME_END();
	#else
	write_reg(address,content);
	#endif
}


// 通过 SPI 总线读 address 寄存器的值
//read REG by SPI
extern unsigned char THM_ReadReg(unsigned char address)
{
	#if 0
	unsigned char temp_buff[1];
	// SS_N =0;
	SPI_FRAME_START();	
	// 读命令 BIT7 = 0;
	//read mode  BIT7 =0;
	temp_buff[0] = address & 0x7F;	
	SPI_SendBuff(temp_buff,1);
	SPI_RecvBuff(temp_buff,1);
	//SS_N =1;
	
	SPI_FRAME_END();
	return(temp_buff[0]);
	#else
	return read_reg(address);
	#endif
}


									

#if 1
//Function: Change to THM3060 to TypeB Mode and Anticollision Loop 
//Parameter: OUT b_uid, card's UID , 4 bytes
//Return value:   00    OK,a card selected and it is the last card
//                other value ,error code
//		     01 ATQB error
//		     02 AttriB error
//		     03 GUID error
unsigned char THM_ISO14443_B(unsigned char * b_uid)
{
    unsigned short xdata iLen;
	unsigned char xdata ret;
	unsigned char xdata temp[20];

    //Change to B mode
    THM_WriteReg(PSEL,0x00);
	THM_WriteReg(SCNTL,0x00);
	DelayMs(5);
	THM_WriteReg(SCNTL,0x01);
	DelayMs(5);
	
  
    //Send REQB  
	temp[0]= 0x05;//APf
	temp[1]= 0x00;//AFi
	temp[2]= 0x00;//param
    THM_SendFrame(temp,3); 
    ret = THM_WaitReadFrame(&iLen, temp);
	if(!iLen||(0x50!=temp[0])){
		DbgLeds(0x01);
		return 1;
	}


    Delay1ms();
	//Send AttriB 	
	temp[0]= 0x1D;
	//temp[1]~temp[4] is PUPI
	temp[5]=0x00;
	temp[6]=0x08;
	temp[7]=0x01;
	temp[8]=0x01;//cid
    THM_SendFrame(temp,9);     
    ret = THM_WaitReadFrame(&iLen, temp);
	if(!iLen)
		return 2;
	if(0x01!=temp[0])//cid
		return 2;

	DbgLeds(0x02);

    Delay1ms();

	//Send GUID	
	//see details on http://www.amobbs.com/forum.php?mod=viewthread&tid=5548512&highlight=%E8%BA%AB%E4%BB%BD%E8%AF%81
	temp[0]=0x00;
	temp[5]=0x36;
	temp[6]=0x00;
	temp[7]=0x00;
	temp[8]=0x08;
	THM_SendFrame(temp,5);	   
	ret = THM_WaitReadFrame(&iLen, temp);
	if(!iLen)
		return 2;
	
	DbgLeds(0x04);
	if(0x01!=temp[0])//cid
		return 2;

	if(temp[8]!=0x90||temp[9]!=0x00)
		return 3;

	//only copy 4 bytes at front?
	*b_uid++ = temp[0];
	*b_uid++ = temp[1];	
	*b_uid++ = temp[2];	
	*b_uid++ = temp[3];		

	return 0;
    
}    
//Function: Change to THM3060 to TypeA Mode and Anticollision Loop 
//Parameter: OUT b_uid, card's UID , 4 bytes
//Return value:   00    OK,a card selected and it is the last card
//                01    OK,a card selected and it is not the last card
//                02    Err, 
unsigned char THM_Anticollision(unsigned char * b_uid)
{
    unsigned short iLen	;
	unsigned char i,loopi,R_sta;
	unsigned char nvb,anti_temp[10],temp[10];
	unsigned char reqa_t;
	reqa_t= 0x26;
	anti_temp[0]  = 0x93;
	anti_temp[1]  = 0x20;

    //Change to A mode
    THM_WriteReg(PSEL,0x10);
	THM_WriteReg(SCNTL,0x00);
	DelayMs(5);
	THM_WriteReg(SCNTL,0x01);
	DelayMs(5);
  
    //Send REQA    
    THM_SendFrame(&reqa_t,1);     
    R_sta = THM_WaitReadFrame(&iLen, temp);
    
	//Send anti    
    THM_SendFrame(anti_temp,2);     
    R_sta = THM_WaitReadFrame(&iLen, temp);
    
	if (R_sta & 0x40) // anticollision
	{
	   	nvb = THM_ReadReg (0x0e) + 1;
		nvb += (unsigned char)(iLen+1)<<4;
		anti_temp[0] = 0x93;
		anti_temp[1] = nvb;	   
		for (loopi = 0 ; loopi < iLen ; loopi ++)	
		{
		   anti_temp[loopi+2] = temp[loopi];		
		}  //  93 +  nvb + data
		//Send anti  
        THM_SendFrame(anti_temp,iLen+2);     
    	R_sta = THM_WaitReadFrame(&iLen, temp);
	    if (R_sta & 0x1) // 
	    {
		  for (i =0; i< 5;i++)
          {
              *b_uid++ = temp[i];
          } 
		  return 0x1;  

     	}
		else
		{
		   return 0x2;
		}

	}
	else if (R_sta & 0x1) // 
	{
	  for (i =0; i< 5;i++)
          {
              *b_uid++ = temp[i];
          } 
		  return 0x0;  
	}
	return 0x2;
    
}    
#else
//Function: Change to THM3060 to TypeA Mode and Anticollision Loop 
//Parameter: OUT b_uid, card's UID , 4 bytes
//Return value:   00    eror,
//                01    cascade 1,uid,select ok
//                02    cascade 2,uid,select ok
//                03    cascade 3,uid,select ok
unsigned char THM_Anticollision2(unsigned char * anti_flag,unsigned short * uid_len_all,unsigned char * b_uid)
{
    unsigned short iLen	,uid_len;
	unsigned char loopi,R_sta;
	unsigned char  xdata temp[10],anti_temp[10];
	unsigned char  reqa_t;
	reqa_t = 0x26;



    //Change to A mode
    THM_WriteReg(PSEL,0x10);
    THM_WriteReg(SCNTL, 0x01);      
  
    //Send REQA    
    THM_SendFrame(&reqa_t,1);     
    R_sta = THM_WaitReadFrame(&iLen, temp);
 	*anti_flag =THM_Anti(0x93,&uid_len,temp); 
	if(*anti_flag !=0x2 )
	{	//9320, uid ok.
	   anti_temp[0]  = 0x93;
	   anti_temp[1]  = 0x70;
	   for (loopi = 0 ; loopi < uid_len ; loopi ++)	
		{
		   *b_uid++ =  temp[loopi];
		   anti_temp[loopi+2] = temp[loopi];		
		}  //  93 +  70 + data
		*uid_len_all = uid_len;
		//Send select  	 93 +  70 + data

        THM_SendFrame(anti_temp,uid_len+2);     
    	R_sta = THM_WaitReadFrame(&iLen, temp);	 

		if (((R_sta & 0x01) == 0x01) && ((temp[0] & 0x04)==0x04))
		{	
		    R_sta =THM_Anti(0x95,&uid_len,temp); 
		   	if(R_sta!=0x2)  //9520
			{
				 anti_temp[0]  = 0x95;
	             anti_temp[1]  = 0x70;
	             for (loopi = 0 ; loopi < uid_len ; loopi ++)	
		         {
		         *b_uid++ =  temp[loopi];
		         anti_temp[loopi+2] = temp[loopi];		
		         }  //  95 +  70 + data
		         *uid_len_all += uid_len;
	           	//Send select  	 95 +  70 + data
                 THM_SendFrame(anti_temp,uid_len+2);     
                 R_sta = THM_WaitReadFrame(&iLen, temp);
				 if (((R_sta&0x01) == 0x01) && ((temp[0] & 0x04)==0x04))
				 {
				 	   R_sta =THM_Anti(0x97,&uid_len,temp); 
		               if(R_sta!=0x2)  //9720
			          {
				          anti_temp[0]  = 0x97;
	                      anti_temp[1]  = 0x70;
	                      for (loopi = 0 ; loopi < uid_len ; loopi ++)	
		                  {
		                      *b_uid++ =  temp[loopi];
		                      anti_temp[loopi+2] = temp[loopi];		
		                  }  //  97 +  70 + data
		                  *uid_len_all += uid_len;
	           	          //Send select  	 97 +  70 + data
                          THM_SendFrame(anti_temp,uid_len+2);     
                          R_sta = THM_WaitReadFrame(&iLen, temp);
						  return 0x3;
				      }
				 }
				 else  
				 {
				    return 0x02;
				 }
				
			  }
		}
        else 
		{		 
		return 0x1;
		}
	

	}
	
	
	
	return 0x0;
	

	    
}   

//Function:  Anticollision Loop  Comand
//Parameter: OUT b_uid, card's UID , 4 bytes
//Return value:   01    OK, uid back
//                00    Err, 
unsigned char THM_Anti(unsigned char cmm_head , unsigned short * uid_len,unsigned char * b_uid)
{
    unsigned short xdata iLen	;
	unsigned char xdata loopi,R_sta;
	unsigned char xdata nvb,anti_temp[10],temp[10];

	anti_temp[0]  = cmm_head;
	anti_temp[1]  = 0x20;
	//Send anti    
    THM_SendFrame(anti_temp,2);     
    R_sta = THM_WaitReadFrame(&iLen, temp);
    
	if (R_sta & 0x01)//no anticollision
	{
	    *uid_len = iLen;
		for(loopi = 0;loopi<iLen;loopi++)
		{
		   *b_uid++ = temp[loopi];
		}
		return 0x0;		   
	}
	else if (R_sta & 0x40) // anticollision
	{
	   	nvb = THM_ReadReg (0x0e) + 1;		  //read pos bit
		nvb += (unsigned char)(iLen+1)<<4;	  //byte nums
		anti_temp[0] = cmm_head;
		anti_temp[1] = nvb;	   
		for (loopi = 0 ; loopi < iLen ; loopi ++)	
		{
		   anti_temp[loopi+2] = temp[loopi];		
		}  //  cmm_head +  nvb + data
		//Send anti  
        THM_SendFrame(anti_temp,iLen+2);     
    	R_sta = THM_WaitReadFrame(&iLen, temp);
	    if (R_sta & 0x1) // 
	    {
		   *uid_len = iLen;
		   for(loopi = 0;loopi<iLen;loopi++)
		   {
		      *b_uid++ = temp[loopi];
		   }
	       return 0x1; 
     	}
		else if	 (R_sta & 0x40) // anticollision
	    {
	   	   nvb = THM_ReadReg (0x0e) + 1;		  //read pos bit
		   nvb += (unsigned char)(iLen+1)<<4;	  //byte nums
		   anti_temp[0] = cmm_head;
	       anti_temp[1] = nvb;	   
		   for (loopi = 0 ; loopi < iLen ; loopi ++)	
		   {
		      anti_temp[loopi+2] = temp[loopi];		
		   }  //  cmm_head +  nvb + data
		   //Send anti  
           THM_SendFrame(anti_temp,iLen+2);     
    	   R_sta = THM_WaitReadFrame(&iLen, temp);
		   if (R_sta & 0x1) // 
	       {
		      *uid_len = iLen;
		      for(loopi = 0;loopi<iLen;loopi++)
		      {
  		         *b_uid++ = temp[loopi];
		      }
	          return 0x1;
		   }
		  else if	 (R_sta & 0x40) // anticollision
	      {
	   	      nvb = THM_ReadReg (0x0e) + 1;		  //read pos bit
		      nvb += (unsigned char)(iLen+1)<<4;	  //byte nums
		      anti_temp[0] = cmm_head;
	          anti_temp[1] = nvb;	   
		      for (loopi = 0 ; loopi < iLen ; loopi ++)	
		      {
		         anti_temp[loopi+2] = temp[loopi];		
		      }  //  cmm_head +  nvb + data
		      //Send anti  
              THM_SendFrame(anti_temp,iLen+2);     
    	      R_sta = THM_WaitReadFrame(&iLen, temp);
		      if (R_sta & 0x1) // 
	          {
		         *uid_len = iLen;
		         for(loopi = 0;loopi<iLen;loopi++)
		         {
  		            *b_uid++ = temp[loopi];
		         }
	             return 0x1;
		      }
			  else
			  {
			     return 0x2;
			  }

		 }
	   }
	}
	return 0x0;
}

#endif

	   
    
  
