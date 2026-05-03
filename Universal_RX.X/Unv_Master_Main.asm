;****************************************************************************
;									    *
;	    File Name: Unv_Master_Main.asm				    *
;	    Date: 4/29/25						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State Universisty;	RCET			    *
;	    Description:Main Source Code for the Universal Control	    *
;			System Master Board				    *
;									    *
;****************************************************************************
;									    *
;	    Revision History:						    *
;     1.Added Robot Address Updating from Port B; 1/13/25		    *
;     2.Added UART RX Interrupt Handler and Include file; 1/14/25	    *
;     3.Added New Data Flag Checking and URC Decoding; 1/15/25		    *
;     4.Added Timer 2 Interrupt Handler; 1/20/25			    *
;     5.Added Timer 2 Interrupt Connection Time out Check(0.5s); 1/22/25    *
;     6.Tested UART RX, Decoding, and Connection Timeout; 1/27/25	    *
;     7.Added I2C COM Timer into Timer 2; 1/29/25			    *
;     8.Added SEND_I2C Subroutine to Handle Drive Control Board; 1/29/25    *
;     9.Added Peripheral Board I2C TX to SEND_I2C Subroutine; 3/3/25	    *
;     10.Added Comments for Template Code; 4./28/25			    *
;     x.								    *
;     x.								    *
;									    *
;****************************************************************************
    
#INCLUDE <p16f1788.inc>				;Processor specific variable definitions
#INCLUDE <Unv_Master_PIC_SetUp.inc>		;Universal Main Board PIC Set Up
#INCLUDE <URC_RX_Handler.inc>			;RX of URC controller Routines
#INCLUDE <Unv_Master_I2C_SetUp.inc>		;I2C Set Up and Read/Write Routines
;LIST	 P=16f1788				;list directive to define processor
;errorlevel -302,-207,-305,-206,-203		;suppress "not in bank 0" message,  Found label after column 1,
    
;******************************************    
;Configuration
;******************************************
    
; CONFIG1
; __config 0xC9E4
 __CONFIG _CONFIG1, _FOSC_INTOSC & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_OFF & _CLKOUTEN_OFF & _IESO_OFF & _FCMEN_OFF
; CONFIG2
; __config 0xDFFF
 __CONFIG _CONFIG2, _WRT_OFF & _VCAPEN_OFF & _PLLEN_OFF & _STVREN_ON & _BORV_LO & _LPBOR_OFF & _LVP_OFF

;******************************************		
;Interrupt Vectors
;******************************************
    ORG H'00'					
    GOTO SETUP				;RESET CONDITION GOTO SETUP
    ORG H'04'
    GOTO INTERRUPT			;Interrupt occur GOTO INTERRUPT
    
;******************************************
;Setup Routine
;******************************************
SETUP
    CALL	INITIALIZE		;Set Up SFR and GPR For PIC Microcontroller
    CALL	I2C_SETUP_MASTER	;Set Up PIC for I2C as Master
    
    BANKSEL	RCSTA
    BSF		RCSTA,4			;Enable Continous Receive UART
    
    BANKSEL	T2CON
    BSF		T2CON,2			;Enable Timer 2
    
    BANKSEL	INTCON
    BSF		INTCON,7		;Enable Global Interrupts
    
    GOTO	MAIN
    
;******************************************
;Interrupt Service Routine
;******************************************
INTERRUPT

      BANKSEL	PIR1
      BTFSC	PIR1,5
      CALL	UART_RX			;Check if RX interrupt and Run RX Save Routine if Yes
      
      BANKSEL	PIR1
      BTFSC	PIR1,1
      CALL	TMR2_INTERRUPT		;Check for Timer 2 Interrupt.  
      
      BANKSEL	PIR1
      BCF	PIR1,5
      BCF	PIR1,1			;Clear Interrupt Flags
    RETFIE
    
;******************************************
;Sub Routines
;******************************************
    
;***************************************************************************************
;*** TMR2_INTERRUPT Does two things. 1. Tracks how long since last UART recieve      ***
;*** And 2. Delays I2C COM to send packets every 96mS				     ***
;***************************************************************************************
TMR2_INTERRUPT
    
    ;***************************************************************************************
    ;**** CONNECT_CHK_COUNT is incremented and if it surpasses 15 a UART COM Timeout is  ***
    ;**** Identified.  This should result in a timeout of no UART RX in 0.5s		 ***
    ;***************************************************************************************
    BANKSEL	CONNECT_CHK_COUNT
    INCF	CONNECT_CHK_COUNT
    MOVLW	D'16'			;32mS * 16 = 0.522S Before Connection Error
    SUBWF	CONNECT_CHK_COUNT,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	CONNECT_TIMEOUT		;Connection Time out Occured (no UART RX in 0.5s)
    
    ;***************************************************************************************
    ;**** I2C_COM_DELAY is incremented until it surpases a count of 3.  When the count	 ***
    ;**** reaches 3 the Flag to Decode UART Data is Set.  This should result in I2C Data ***
    ;**** transmitted every 96mS							 ***
    ;***************************************************************************************
    BANKSEL	I2C_COM_DELAY
    INCF	I2C_COM_DELAY
    MOVLW	D'3'			;32mS * 3 = 96mS Per I2C COM Send
    SUBWF	I2C_COM_DELAY,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	UPDATE_I2C		;96mS Passed Clear Count and Upate Flag
    GOTO	TMR2_INTERRUPT_END	;Timeout has not occured Exit Routine
    
    ;***************************************************************************************
    ;****UPDATE_I2C occurs when 96mS has Passed.  This will reset the delay count and	 ***
    ;**** Set the flag to signal an I2C Send routine in Main Loop			 ***
    ;***************************************************************************************
UPDATE_I2C
    BANKSEL	I2C_COM_DELAY
    CLRF	I2C_COM_DELAY		;Clear I2C Com Delay Count
    BANKSEL	NEW_RX_DATA
    BSF		NEW_RX_DATA,1		;Set Update I2C Flag
    GOTO	TMR2_INTERRUPT_END	;Exit Routine
    
    ;***************************************************************************************
    ;**** CONNECT_TIMEOUT occurs when 0.5S has passed without UART receive. This signals ***
    ;**** that the master lost connection to the controller. Can Send "All Stop Command" ***
    ;***  to the slave boards (Note: This version of the code handles an All Stop on the ***
    ;***  slaves when they do not receive updated I2C; i.e. I2C COM Timeout on the slave)***
    ;***************************************************************************************
CONNECT_TIMEOUT
    MOVLW	D'15'
    MOVWF	CONNECT_CHK_COUNT	;Reset Count to 15 so next itteration will timeout
    CALL	CONTROLLER_DISCONNECT	;Run All stop Commands to slaves
TMR2_INTERRUPT_END
    RETURN

CONTROLLER_DISCONNECT  ;******Connection Timeout Occured.  Run Stop Functions******
    ;**** Here Add an All Stop Command to the slaves for UART Timeout ****
    ;**** i.e. Send Motor Stop VIA I2C to Peripheral Boards	      ****
    RETURN
   
;***************************************************************************************
;**** SEND_I2C Occurs when the I2C Delay Flag is set (NEW_RX_DATA,1 ; Every 96mS)    ***
;****  This function Loads the Board Addresses and Desired Data to be sent and runs  ***
;****  the I2C_WRITE Function to Transmit I2C Data to Each Slave.		     ***
;****------------------------------Important Note:-----------------------------------***
;**** This is the function you will most likely change.  Determine which slaves get  ***
;****  which controller data bytes(4 Max) and initiate the sending of I2C Data.      ***
;****--------------------------------------------------------------------------------***
;***************************************************************************************
SEND_I2C
    BANKSEL	DRIVE_CONTROL_W
    MOVFW	DRIVE_CONTROL_W
    BANKSEL	ADDRESS_W
    MOVWF	ADDRESS_W	    ;Set Drive Control Board I2C Address
    BANKSEL	JOY1_UD
    MOVFW	JOY1_UD
    BANKSEL	DATA_TX_1
    MOVWF	DATA_TX_1	    ;Load Joystick 1 Up and Down Data Byte as Data Byte 1
    BANKSEL	JOY1_LR
    MOVFW	JOY1_LR
    BANKSEL	DATA_TX_2
    MOVWF	DATA_TX_2	    ;Load Joystick 1 Left and Right Data Byte as Data Byte 2
    BANKSEL	JOY2_UD
    MOVFW	JOY2_UD
    BANKSEL	DATA_TX_3
    MOVWF	DATA_TX_3	    ;Load Joystick 2 Up and Down Data Byte as Data Byte 3
    BANKSEL	JOY2_LR
    MOVFW	JOY2_LR
    BANKSEL	DATA_TX_4
    MOVWF	DATA_TX_4	    ;Load Joystick 2 Left and Right Data Byte as Data Byte 4
    CALL	I2C_WRITE
    ;Load and Send Peripheral Board Data 
    BANKSEL	PERIPHERAL_CONTROL_W
    MOVFW	PERIPHERAL_CONTROL_W
    BANKSEL	ADDRESS_W
    MOVWF	ADDRESS_W	    ;Set Peripheral Control Board I2C Address
    BANKSEL	JOY3_UD
    MOVFW	JOY3_UD
    BANKSEL	DATA_TX_1
    MOVWF	DATA_TX_1	    ;Load Joystick 3 Up and Down Data Byte as Data Byte 1
    BANKSEL	JOY3_LR
    MOVFW	JOY3_LR
    BANKSEL	DATA_TX_2
    MOVWF	DATA_TX_2	    ;Load Joystick 3 Left and Right Data Byte as Data Byte 2
    BANKSEL	BUTTON_STATUS_1
    MOVFW	BUTTON_STATUS_1
    BANKSEL	DATA_TX_3
    MOVWF	DATA_TX_3	    ;Load Button Data Packet 1 as Data Byte 3
    BANKSEL	BUTTON_STATUS_2
    MOVFW	BUTTON_STATUS_2
    BANKSEL	DATA_TX_4
    MOVWF	DATA_TX_4	    ;Load button Data Packet 2 as Data Byte 4
    CALL	I2C_WRITE
    ;*******************************************************************************
    ;***Add More Board Here by Loading Data Bytes and Setting New Board Address  ***
    ;*** You will also need to add these addresses into Unv_Master_I2C_SetUp.inc ***
    ;*******************************************************************************
SEND_I2C_END
    BANKSEL	NEW_RX_DATA
    BCF		NEW_RX_DATA,1	    ;Clear Flag For Handling I2C Data
    RETURN
 
;******************************************
;Main Code
;******************************************
MAIN
    ;***************************************************************************************
    ;**** Update Robot Address from Port B Input. (Allows for changing address without	 ***
    ;**** rebooting Robot).								 ***
    ;***************************************************************************************
    BANKSEL	PORTB
    MOVFW	PORTB
    BANKSEL	ROBOT_ADDR
    MOVWF	ROBOT_ADDR	;Upate Current Status of Port B into Robot Address Register
    
    ;***************************************************************************************
    ;**** Checks the UART receive flag.  Should occur everytime a full UART data packet	 ***
    ;**** is received (Flag is Set in UART_RX function within URC_RX_Handler.inc	 ***
    ;***************************************************************************************
    BANKSEL	NEW_RX_DATA
    BTFSC	NEW_RX_DATA,0
    CALL	DECODE_URC	;Check for New Data to Decode.  Decode From URC Data Structure
    
    ;***************************************************************************************
    ;**** Check if the I2C Send flag is set.  Should occur every 96mS.  Flag is set in	 ***
    ;**** Timer 2 Interrupt.  Will not Send I2C data if UART COM timeout occurs.  	 ***
    ;***************************************************************************************
    BANKSEL	NEW_RX_DATA
    BTFSC	NEW_RX_DATA,1
    CALL	SEND_I2C	;Check if enough time has passed to send I2C Data
    
MAIN_END
    GOTO	MAIN
    END				;END PROGRAM DIRECTIVE *******************************
;*************************************************************************************************************
    