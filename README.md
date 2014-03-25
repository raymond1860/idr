idr
===
This project is for China ID2 card reader. ID2 card is ISO14443 type B, and Shenzhen resident ID card is type A.
All materials are for type A and type B card reader.

How idr works?
===
    * idr work flow
    *			  HOST(PC-like uart terminal)
    *             /|\              
    *              |  	
    * THM3060<--->MCU<----->SAM
    *
    * THM3060 is RF non contact ISO/IEC14443 A/B ,ISO/IEC15693 card reader
    * SAM is ID2 card security module
    *
    * Interface between THM3060 and MCU is SPI
    * Interface between SAM and MCU are I2C and UART
    * Interface between HOST and MCU is UART
    *
    * For MCU STC15F2K08S2 based application:
    *     SPI and I2C is gpio simulated.
    *     UART1 is for HOST&MCU
    *     UART2 is for SAM&MCU
    *
    
Structure
===
    ├─document  -->project documents
    ├─source    -->source code for both host and mcu
    │  ├─host   -->host side card reader program
    │  └─mcu
    │      ├─SOURCE -->reference design source
    │      │  ├─main
    │      │  ├─secure
    │      │  └─spi 
    │      ├─UV2    -->reference design project 
    │      └─v1     -->new designed mcu project
    └─tools    -->tools for debugging and provisioning


Feedback
===
Any suggestion is welcomed.
