;***************************************************************************
;Filename: I2C_Handeler
;Date: 03/30/2026
;File Version: 1
;Author: Jason Permann
;Co-Author: 
;Company: Idaho State University
;Description: 
;***************************************************************************
    
#include <xc.inc>     
    

PSECT    code
    
I2C_SETUP_PERIPHERAL:
;***************************************************************************    
    BANKSEL	    TRISC
    BSF		    TRISC,3
    BSF		    TRISC,4		;Set SCL and SDA as inputs
;***************************************************************************************
;*** Set SSPADD for the desired Slave Address					     ***
;***************************************************************************************    
    BANKSEL	    SSP1ADD
    MOVLW	    0x20
    MOVWF	    SSP1ADD		;Peripheral Address Read=0x11; Write=0x10
    BANKSEL	    SSP1CON1
    MOVLW	    0X36
    MOVWF	    SSP1CON1	;Enable SDA and SCL Pins, Release Clock, and Set as 7 bit address Slave
    BANKSEL	    SSP1CON2
    MOVLW	    0X09
    MOVWF	    SSP1CON2	;Disable General Call and Enable Receive and Clock Strechting
    BANKSEL	    SSP1CON3		
    MOVLW	    0X20
    MOVWF	    SSP1CON3	;Disable Slave Interrupts, SDA Hold Time of 300nS, Auto Enable Address and Data Clock Stretching
    BANKSEL	    SSP1STAT
    MOVLW	    0X40
    MOVWF	    SSP1STAT	;Disable Slew Rate Control and flags
    BANKSEL	    SSP1MSK
    MOVLW	    0XFE
    MOVWF	    SSP1MSK		;Enable address matching
    BANKSEL	    I2C_RX_COUNTER
    CLRF	    I2C_RX_COUNTER	;Begin at first Data Byte
    BANKSEL	    PORTB
    BSF		    PORTB,1		;made it to I2C program
    RETURN
    
;********************************************************************************************
;*** I2C_RECEIVE Should be called each time an MSSP Interrupt Occurs.  This function will ***
;*** Check which byte has been received from I2C and save them into temporary registers.  ***
;*** Additionally, this routine sets the flag when a full data byte is received and       ***
;*** clears the timer 1 counter registers used for I2C COM Timeout.  			  ***
;********************************************************************************************
I2C_RECEIVE:
	BANKSEL	    SSP1CON1
	BCF	    SSP1CON1,4				;Hold Clock
	BANKSEL	    SSP1STAT
	BTFSC	    SSP1STAT,5
	GOTO	    I2C_DATA_RX				;Check if last receive was data or address
ADDRESS_RX:  
	BANKSEL	    SSP1BUF
	MOVF	    SSP1BUF,0				;Clear Address out of Buffer
	BANKSEL	    I2C_RX_COUNTER
	CLRF	    I2C_RX_COUNTER			;Clear Data Received Counter
	GOTO	    I2C_RECEIVE_END
I2C_DATA_RX:
	BANKSEL	    I2C_RX_COUNTER
	MOVLW	    0x00
	SUBWF	    I2C_RX_COUNTER,0			;Subtract W from F (F-W).  Store in W
	BANKSEL	    STATUS
	BTFSC	    STATUS,2
	GOTO	    FIRST_DATA_BYTE			;Subtraction = 0.  Counter = 0
	BANKSEL	    I2C_RX_COUNTER
	MOVLW	    0x01
	SUBWF	    I2C_RX_COUNTER,0			;Subtract W from F (F-W).  Store in W
	BANKSEL	    STATUS
	BTFSC	    STATUS,2
	GOTO	    SECOND_DATA_BYTE			;Subtraction = 0.  Counter = 1
	BANKSEL	    I2C_RX_COUNTER
	MOVLW	    0x02
	SUBWF	    I2C_RX_COUNTER,0			;Subtract W from F (F-W).  Store in W
	BANKSEL	    STATUS
	BTFSC	    STATUS,2
	GOTO	    THIRD_DATA_BYTE			;Subtraction = 0.  Counter = 2
	BANKSEL	    I2C_RX_COUNTER
	MOVLW	    0x03
	SUBWF	    I2C_RX_COUNTER,0			;Subtract W from F (F-W).  Store in W
	BANKSEL	    STATUS
	BTFSC	    STATUS,2
	GOTO	    FOURTH_DATA_BYTE			;Subtraction = 0.  Counter = 3
	GOTO	    I2C_RECEIVE_END			;Not Data 1,2,3, or 4 Exit Routine
FIRST_DATA_BYTE:
	BANKSEL	    SSP1BUF
	MOVF	    SSP1BUF,0
	BANKSEL	    I2C_RX_TEMP_1
	MOVWF	    I2C_RX_TEMP_1			;Save First Data Byte into Temporary Register
	GOTO	    I2C_RECEIVE_DATA_END
SECOND_DATA_BYTE:
	BANKSEL	    SSP1BUF
	MOVF	    SSP1BUF,0
	BANKSEL	    I2C_RX_TEMP_2
	MOVWF	    I2C_RX_TEMP_2			;Save Second Data Byte into Temporary Register
	GOTO	    I2C_RECEIVE_DATA_END
THIRD_DATA_BYTE:
	BANKSEL	    SSP1BUF
	MOVF	    SSP1BUF,0
	BANKSEL	    I2C_RX_TEMP_3
	MOVWF	    I2C_RX_TEMP_3			;Save Third Data Byte into Temporary Register
	GOTO	    I2C_RECEIVE_DATA_END
FOURTH_DATA_BYTE:
	BANKSEL	    SSP1BUF
	MOVF	    SSP1BUF,0
	BANKSEL	    I2C_RX_TEMP_4
	MOVWF	    I2C_RX_TEMP_4			;Save Fourth Data Byte into Temporary Register
	BANKSEL	    I2C_RX_COMPLETE
	BSF	    I2C_RX_COMPLETE,0			;Set flag to indicate full data packet has been received
	BANKSEL	    TMR1H
	CLRF	    TMR1H
	CLRF	    TMR1L				;Clear Timer 1 Count For I2C COM Timeout
	BANKSEL	    PORTA
	BCF	    PORTA,3
	GOTO	    I2C_RECEIVE_DATA_END
I2C_RECEIVE_DATA_END:
	BANKSEL	    I2C_RX_COUNTER
	INCF	    I2C_RX_COUNTER			;Increment Data Byte Counter to Save The next received Byte
	GOTO	    I2C_RECEIVE_END
I2C_RECEIVE_END:
	BANKSEL	    SSP1CON1
	BCF	    SSP1CON1,7
	BANKSEL	    SSP1CON1
	BCF	    SSP1CON1,6
	BANKSEL	    SSP1BUF
	MOVF	    SSP1BUF,0
    
	BANKSEL	    SSP1CON1
	BSF	    SSP1CON1,4				;Re-enable Clock (stop clock strechting)
	BANKSEL	    PIR1    
	BCF	    PIR1,3				;Clear MSSP Interrupt Flag
	RETURN