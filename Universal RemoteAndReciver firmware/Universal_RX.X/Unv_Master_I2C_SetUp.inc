 ;****************************************************************************
;									    *
;	    File Name: Unv_Master_I2C_SetUp.INC				    *
;	    Date:   3/3/25						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State University; RCET			    *
;	    Description:  I2C COM SetUp and functions for Universal	    *
;			    Control System Master Board.		    *
;									    *
;****************************************************************************
    ;Before an I2C_WRITE call Main Code Should:
    ;1. Load ADDRESS_W(R) Registers for Peripheral (Read and Write Registers)
    ;2. Load DATA_TX_1 Register with First Byte Data
    ;3. Load DATA_TX_2 Register with Second Byte Data
    ;4. Load DATA_TX_3 Register with Third Byte Data
    ;5. Load DATA_TX_4 Register with Fourth Byte Data
    ;6. Call I2C_WRITE
    
    ;Before an I2C_READ call Main Code Should:
    ;1. Load ADDRESS_W(R) Registers for Peripheral (Read and Write Registers)
    ;2. Load DATA_TX_1 Register with the Desired Address To be Read
    ;3. Call I2C_READ
    ;4. Save DATA_RECEIVED to Desired Register
    
    ;***************************************************************************************
    ;****		Memory Save Locations Specifically for I2C COM			 ***
    ;***************************************************************************************
ADDRESS_W			EQU	H'040'
ADDRESS_R			EQU	H'041'
DATA_TX_1			EQU	H'042'
DATA_TX_2			EQU	H'043'
DATA_TX_3			EQU	H'044'
DATA_TX_4			EQU	H'045'
DATA_RECEIVED			EQU	H'046'
    
   
DRIVE_CONTROL_W			EQU	H'047'
DRIVE_CONTROL_R			EQU	H'048'
PERIPHERAL_CONTROL_W		EQU	H'049'
PERIPHERAL_CONTROL_R		EQU	H'04A'
    ;***************************************************************************************
    ;****Enable and Rename For additional Slave Board Address Saving
    ;***************************************************************************************
    ;AUDIO_CONTROL_W		EQU	H'04B'
    ;AUDIO_CONTROL_R		EQU	H'04C'
    
I2C_CODE    CODE
    
I2C_SETUP_MASTER
    ;***************************************************************************************
    ;****		    Set Up Slave Board Addresses				 ***
    ;***************************************************************************************
	BANKSEL	    DRIVE_CONTROL_W
	MOVLW	    H'10'		
	MOVWF	    DRIVE_CONTROL_W	;Save Drive Control Daugher Board Write Address
	BANKSEL	    DRIVE_CONTROL_R
	MOVLW	    H'11'
	MOVWF	    DRIVE_CONTROL_R	;Save Drive Control Daugher Board Read Address
	BANKSEL	    PERIPHERAL_CONTROL_W
	MOVLW	    H'20'
	MOVWF	    PERIPHERAL_CONTROL_W ;Save Peripheral Control Daugher Board Write Address
	BANKSEL	    PERIPHERAL_CONTROL_R
	MOVLW	    H'21'
	MOVWF	    PERIPHERAL_CONTROL_R ;Save Peripheral Control Daugher Board Read Address
	
    ;***************************************************************************************
    ;****		Enable and Set Additional Slave Board Addresses			 ***
    ;***************************************************************************************
	;BANKSEL    AUDIO_CONTROL_W
	;MOVLW	    H'__'
	;MOVWF	    AUDIO_CONTROL_W
	;BANKSEL    AUDIO_CONTROL_R
	;MOVLW	    H'__'
	;MOVWF	    AUDIO_CONTROL_R
	
    ;******************************************************************************************************
    ;****			    Set Up Pic for I2C Master COM					***
    ;******************************************************************************************************
	BANKSEL	    TRISC
	BSF	    TRISC,3
	BSF	    TRISC,4 		;Set SCL and SDA as inputs
	BANKSEL	    SSP1ADD
	MOVLW	    H'27'
	MOVWF	    SSP1ADD		;Set BAUD Generator to 100Khz (16M Occilator)
	BANKSEL	    SSP1CON1
	MOVLW	    H'28'		
	MOVWF	    SSP1CON1		;Enable SDA and SLC Pins and Set as I2C Master Mode
	BANKSEL	    SSP1CON2
	MOVLW	    H'00'
	MOVWF	    SSP1CON2		;Disable General Call
	BANKSEL	    SSP1CON3
	MOVLW	    H'08'		;Min SDA Hold time 300ns
	MOVWF	    SSP1CON3
	BANKSEL	    SSP1STAT
	MOVLW	    H'A0'		
	MOVWF	    SSP1STAT		;Not in Tx mode, Slew Rate Control Disabled
	RETURN
	
;******************************************************************************************************
;****I2C_WRITE Function is Used to Send 4 Bytes to the Addressed slave.  See Above for Instructions ***
;******************************************************************************************************
I2C_WRITE
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BSF	    SSP1CON2,0		;Generate Start Condition
START_WRITE_DONE
	BTFSC	    SSP1CON2,0
	GOTO	    START_WRITE_DONE	;Wait Until Start Condition Completes
	BANKSEL	    ADDRESS_W
	MOVFW	    ADDRESS_W
	BANKSEL	    SSP1BUF
	MOVWF	    SSP1BUF		;Send Peripheral Address (Write)
ADDRESS_SENT_W
	BANKSEL	    SSP1STAT
	BTFSC	    SSP1STAT,0		
	GOTO	    ADDRESS_SENT_W	;Wait Until Buffer is Clear (Address is Sent)
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BTFSC	    SSP1CON2,6		;Was an Acknowledge Received?
	GOTO	    BAD_COND		;Acknowledge Not Received
	BANKSEL	    DATA_TX_1
	MOVFW	    DATA_TX_1
	BANKSEL	    SSP1BUF
	MOVWF	    SSP1BUF		;Send First Data Byte
DATA_1_SENT
	BANKSEL	    SSP1STAT
	BTFSC	    SSP1STAT,0
	GOTO	    DATA_1_SENT		;Wait Until Buffer is Clear (Data is Sent)
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BTFSC	    SSP1CON2,6		;Was an Acknowledge Received?
	GOTO	    BAD_COND		;Acknowledge Not Received
	BANKSEL	    DATA_TX_2
	MOVFW	    DATA_TX_2
	BANKSEL	    SSP1BUF
	MOVWF	    SSP1BUF		;Send Second Data Byte
DATA_2_SENT
	BANKSEL	    SSP1STAT
	BTFSC	    SSP1STAT,0
	GOTO	    DATA_2_SENT		;Wait Unitl Buffer is Clear (Data is Sent)
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BTFSC	    SSP1CON2,6		;Was an Acknowledge Received?
	GOTO	    BAD_COND		;Acknowledge not Received
	BANKSEL	    DATA_TX_3
	MOVFW	    DATA_TX_3
	BANKSEL	    SSP1BUF
	MOVWF	    SSP1BUF		;Send Third Data Byte
DATA_3_SENT
	BANKSEL	    SSP1STAT
	BTFSC	    SSP1STAT,0
	GOTO	    DATA_3_SENT		;Wait Until Buffer is Clear (Data is Sent)
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BTFSC	    SSP1CON2,6		;Was an Acknowledge Received?
	GOTO	    BAD_COND		;Acknowledge Not Received
	BANKSEL	    DATA_TX_4
	MOVFW	    DATA_TX_4
	BANKSEL	    SSP1BUF
	MOVWF	    SSP1BUF		;Send Fourth Data Byte
DATA_4_SENT
	BANKSEL	    SSP1STAT
	BTFSC	    SSP1STAT,0
	GOTO	    DATA_4_SENT		;Wait Until Buffer is Clear (Data is Sent)
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BTFSC	    SSP1CON2,6		;Was an Acknowledge Received?
	GOTO	    BAD_COND		;Acknowledge Not Received
GOOD_STOP
	BANKSEL	    SSP1CON2
	BSF	    SSP1CON2,2		;Generate a Stop Condition
GOOD_STOP_DONE
	BTFSC	    SSP1CON2,2
	GOTO	    GOOD_STOP_DONE	;Wait Until Stop Condition Completes
	MOVLW	    H'00'
	RETURN				;Write Complete.  Return With 0x00 in Working Register
	
;******************************************************************************************************************
;****I2C_READ Function is Meant to Retreive Data From Slave. See Above for Instructions		                ***
;****NOTE: The Project This Template was Designed alongside did Not Read From Slave.  THIS FUNCTION IS UNTESTED ***
;******************************************************************************************************************
I2C_READ
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BSF	    SSP1CON2,0		;Generate Start Condition
START_READ_DONE_1
	BTFSC	    SSP1CON2,0
	GOTO	    START_READ_DONE_1	;Wait Until Start Condition Completes
	BANKSEL	    ADDRESS_W
	MOVFW	    ADDRESS_W
	BANKSEL	    SSP1BUF
	MOVWF	    SSP1BUF		;Send Peripheral Address (Write)
ADDRESS_SENT_R_1
	BANKSEL	    SSP1STAT
	BTFSC	    SSP1STAT,0		
	GOTO	    ADDRESS_SENT_R_1	;Wait Until Buffer is Clear (Address is Sent)
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BTFSC	    SSP1CON2,6		;Was an Acknowledge Received?
	GOTO	    BAD_COND		;Acknowledge Not Received
	BANKSEL	    PIR1
	BCF	    PIR1,3		;Clear SSP interrupt Flag
	BANKSEL	    DATA_TX_1
	MOVFW	    DATA_TX_1
	BANKSEL	    SSP1BUF
	MOVWF	    SSP1BUF		;Send First Data Byte
DATA_1_SENT_R
	BANKSEL	    SSP1STAT
	BTFSC	    SSP1STAT,0
	GOTO	    DATA_1_SENT_R	;Wait Until Buffer is Clear (Data is Sent)
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BTFSC	    SSP1CON2,6		;Was an Acknowledge Received?
	GOTO	    BAD_COND		;Acknowledge Not Received
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BSF	    SSP1CON2,0		;Generate Repeated Start Condition
START_READ_DONE_2
	BTFSC	    SSP1CON2,0
	GOTO	    START_READ_DONE_2	;Wait Until Repeated Start Condition Completes
	BANKSEL	    PIR1
	BCF	    PIR1,3		;Clear SSP interrupt Flag
	BANKSEL	    ADDRESS_R
	MOVFW	    ADDRESS_R
	BANKSEL	    SSP1BUF
	MOVWF	    SSP1BUF		;Send Peripheral Address (Read)
ADDRESS_SENT_R_2
	BANKSEL	    SSP1STAT
	BTFSC	    SSP1STAT,0		
	GOTO	    ADDRESS_SENT_R_2	;Wait Until Buffer is Clear (Address is Sent)
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BTFSC	    SSP1CON2,6		;Was an Acknowledge Received?
	GOTO	    BAD_COND		;Acknowledge Not Received
	BANKSEL	    SSP1CON2
	BSF	    SSP1CON2,3		;Enable Receive Mode
	BANKSEL	    PIR1
	BCF	    PIR1,3		;Clear SSP interrupt Flag
	BANKSEL	    SSP1BUF
	MOVFW	    SSP1BUF		;Clear SSP1BUF Buffer
	CALL	    I2C_IDLE		;Ensure the I2C BUS is idle
	BANKSEL	    SSP1CON2
	BSF	    SSP1CON2,5		;Set Acknowledge Data to NACK
	BSF	    SSP1CON2,4		;Send NACK (Stop Data Receive)
NACK_DONE
	BTFSC	    SSP1CON2,4
	GOTO	    NACK_DONE		;Wait Until NACK Completes
	BANKSEL	    SSP1CON2
	BCF	    SSP1CON2,3		;Disable Receive Mode
	BANKSEL	    SSP1CON2
	BSF	    SSP1CON2,2		;Generate Stop Condition
READ_STOP_DONE_2
	BTFSC	    SSP1CON2,2
	GOTO	    READ_STOP_DONE_2	;Wait Until Stop Condition Completes
	BANKSEL	    SSP1BUF
	MOVFW	    SSP1BUF
	BANKSEL	    DATA_RECEIVED
	MOVWF	    DATA_RECEIVED	;Save Data Sent By the Peripheral
	RETURN
	
;******************************************************************************************************
;****BAD_COND Function is GOTO'd When a Slave Does not ACK.  Will Exit Current Routine		    ***
;******************************************************************************************************
BAD_COND
	BANKSEL	    SSP1CON2
	BSF	    SSP1CON2,2		;Generate Stop Condition
BAD_STOP_DONE
	BTFSC	    SSP1CON2,2
	GOTO	    BAD_STOP_DONE	;Wait Until Stop Condition Completes
	MOVLW	    H'FF'
	RETURN				;Write/Read Failed.  Return With 0xFF in Working Register
	
;************************************************************************************************************
;****I2C_Idle Function is Used to Ensure I2C Line is Not Busy.  Will Return to Function When Line is Idle ***
;************************************************************************************************************
I2C_IDLE
	BANKSEL	    SSP1CON2
	MOVLW	    H'1F'
	ANDWF	    SSP1CON2		;Compare to 0x1F to Test for 5 Busy Conditions
	BANKSEL	    STATUS
	BTFSS	    STATUS,2
	GOTO	    I2C_IDLE		;If any Conditions Still Busy Wait
CHECK_R_W
	BANKSEL	   SSP1STAT
	BTFSC	   SSP1STAT,2
	GOTO	   CHECK_R_W		;Wait Until  a Transmission is Not in Process
	RETURN				;I2C BUS is Idle Exit Routine
	
	
	
	
	