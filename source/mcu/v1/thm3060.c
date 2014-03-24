#include <stdio.h>
#include "spi.h"
#include "thm3060.h"

// ͨ�� SPI ����д address �Ĵ�����ֵ
//write REG bu spi
extern void THM_WriteReg(unsigned char address,unsigned char content)
{
	#if 0
	unsigned char temp_buff[2];
	// д���� BIT7 =1	
	//write mode BIT7 = 1
	temp_buff[0] = address | 0x80;
	temp_buff[1] = content;
	// SPI ֡
	//SPI frame
	SPI_FRAME_START();	
	SPI_SendBuff(temp_buff,2);	
	SPI_FRAME_END();
	#else
	write_reg(address,content);
	#endif
}


// ͨ�� SPI ���߶� address �Ĵ�����ֵ
//read REG by SPI
extern unsigned char THM_ReadReg(unsigned char address)
{
	#if 0
	unsigned char temp_buff[1];
	// SS_N =0;
	SPI_FRAME_START();	
	// ������ BIT7 = 0;
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


									

#if 0
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
    THM_WriteReg(SCNTL, 0x01);      
  
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
#endif
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


	   
    
  