;****************************************************************************
;									    *
;	    File Name: URC_RX_Handler.inc				    *
;	    Date: 1/14/25						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State Univerisy; RCET			    *
;	    Description:  Handles UART RX and URC Data Packet Decoding      *
;									    *
;****************************************************************************
;									    *
;	    Revision History:						    *
;	1.Added UART_RX to Save RX Data into Temp Registers; 1/14/25	    *
;	2.Added DECODE_URC.  Decodes Temp Reg into Controller State; 1/15/25*
;	3.								    *
;	4.								    *
;	5.								    *
;									    *
;****************************************************************************
    
    
URC_RX_HANDLERS CODE
;****************************************************************************************
;****IMPORTANT NOTE: To properly use this include file the PIC setup needs to define  ***
;**** general purpose registers used in this file:				      ***
; BYTE_COUNT		EQU	0X__	    ;Register to count Receied Bytes
; ROBOT_ADDR		EQU	0X__	    ;Register to Save Robot Address (Port B Dip Switch Values)
; JOY1_UD		EQU	0X__	    ;Register to Save Joystick 1 Up-Down Data
; JOY1_LR		EQU	0X__	    ;Register to Save Joystick 1 Left-Right Data
; JOY2_UD		EQU	0X__	    ;Register to Save Joystick 2 Up-Down Data
; JOY2_LR		EQU	0X__	    ;Register to Save Joystick 2 Left-Right Data
; JOY3_UD		EQU	0X__	    ;Register to Save Joystick 3 Up-Down Data
; JOY3_LR		EQU	0X__	    ;Register to Save Joystick 3 Left-Right Data
; BUTTON_STATUS_1	EQU	0X__	    ;Register to Save Button Status
; BUTTON_STATUS_2	EQU	0X__	    ;Register to Save Button Status
; CURRENT_DATA_PACK     EQU	0X__	    ;Register to Save Which Data Packet was Last Received
; MEM_SAVE_1		EQU	0X__	    ;Register to temp save RX Data Variable 1
; MEM_SAVE_2		EQU	0X__	    ;Register to temp save RX Data Variable 2
; MEM_SAVE_3		EQU	0X__	    ;Register to temp save RX Data Variable 3
; NEW_RX_DATA		EQU	0X__	    ;Register to Track New Data Status (bit 0 Set = New Data Available, Bit 1 for I2C Data Send)
; CONNECT_CHK_COUNT     EQU	0X__	    ;Register to Track Connection Status (Increment On TMR 2; Clear on RX)
;****************************************************************************************************************************************
 
;****************************************************************************************
;****			Data Pack from Universal Remote Control(URC)		      ***
;****************************************************************************************
;____________________________________________________________________________________________________________________________________________
;| Sync	 | Robot Address | Command |		 Byte 1			|	     Byte 2		  |		Byte 3		    |
;|0x24($)| Robot Address | 0x4A(J) | Joystick 1 Up-Down High Byte	| Joystick 1 Right-Left High Byte | Joystick 2 Up-Down High Byte    |
;|0x24($)| Robot Address | 0x6A(j) | Joystick 2 Right-Left High Byte	| Joystick 3 Up-Down High Byte	  | Joystick 3 Right-Left High Byte |
;|0x24($)| Robot Address | 0x42(B) | Button Data Byte 1			| Button Data Byte 2		  | 0x00 (Not Used)		    |
;____________________________________________________________________________________________________________________________________________
 
;****************************************************************************************
;****				Button Data Bit Maps				      ***
;****************************************************************************************
;_____________________________          _____________________________
;|    Button Data Byte 1     |		|     Button Data Byte 2    |
;| Bit   |      Button       |		|  Bit  |       Button      |
;|   0   | Down Face Button  |		|   0   | Joystick 3 Button |
;|   1   | Right Face Button |		|  1-7  |     Not Used      |
;|   2   | Left Face Button  |		_____________________________
;|   3   | Up Face Button    |
;|   4   | Left Bumper       |
;|   5   | Right Bumper      |
;|   6   | Joystick 1 Button |
;|   7   | Joystick 2 Button |
;_____________________________
 
;***************************************************************************************
;****UART_RX Function Should be Called when a Byte is Received From UART	     ***
;****This Function will Determine the Byte Received, Verify Handshake and Address,   ***
;****Save the Received Byte into Temp Variables, and Set a flag to signal to Decode  ***
;***************************************************************************************
UART_RX
 BANKSEL    BYTE_COUNT
 MOVLW	    H'00'
 SUBWF	    BYTE_COUNT,0	    ;Subtract BYTE_COUNT - W; Result Store in W
 BTFSC	    STATUS,2		    ;Check Zero Bit.  1 if Zero; 0 if not Zero
 GOTO	    BYTE_1		    ;First Received Byte Check for HandShake CHAR 
 
 MOVLW	    H'01'
 SUBWF	    BYTE_COUNT,0	    ;Subtract BYTE_COUNT - W; Result Store in W
 BTFSC	    STATUS,2		    ;Check Zero Bit.  1 if Zero; 0 if not Zero
 GOTO	    BYTE_2		    ;Second Received Byte Check for Robot Address 
 
 MOVLW	    H'02'
 SUBWF	    BYTE_COUNT,0	    ;Subtract BYTE_COUNT - W; Result Store in W
 BTFSC	    STATUS,2		    ;Check Zero Bit.  1 if Zero; 0 if not Zero
 GOTO	    BYTE_3		    ;Third Received Byte Save as Data Packet Command 
 
 MOVLW	    H'03'
 SUBWF	    BYTE_COUNT,0	    ;Subtract BYTE_COUNT - W; Result Store in W
 BTFSC	    STATUS,2		    ;Check Zero Bit.  1 if Zero; 0 if not Zero
 GOTO	    BYTE_4		    ;Fourth Received Byte Save as Memory Save 1 
    
 MOVLW	    H'04'
 SUBWF	    BYTE_COUNT,0	    ;Subtract BYTE_COUNT - W; Result Store in W
 BTFSC	    STATUS,2		    ;Check Zero Bit.  1 if Zero; 0 if not Zero
 GOTO	    BYTE_5		    ;Fifth Received Byte Save as Memory Save 2 
 
 MOVLW	    H'05'
 SUBWF	    BYTE_COUNT,0	    ;Subtract BYTE_COUNT - W; Result Store in W
 BTFSC	    STATUS,2		    ;Check Zero Bit.  1 if Zero; 0 if not Zero
 GOTO	    BYTE_6		    ;Sixth Received Byte Save as Memory Save 3 
 
 CLRF	    BYTE_COUNT
 GOTO	    UART_RX_END		    ;Unknown Byte Count (NOT 0-6 Bytes) Clear Count and Exit
 
BYTE_1	;**First Received Byte Check for HandShake CHAR**
 MOVLW	    H'24'
 BANKSEL    RCREG
 SUBWF	    RCREG,0		    ;Subtract RCREG - W; Result Store in W
 BTFSC	    STATUS,2
 GOTO	    HANDSHAKE_VERIFIED	    ;HandShake Confirmed Increment Byte Count
 BANKSEL    BYTE_COUNT
 CLRF	    BYTE_COUNT
 GOTO	    UART_RX_END		    ;HandShake Not Verified. Clear Count and Exit
HANDSHAKE_VERIFIED
 BANKSEL    BYTE_COUNT
 INCF	    BYTE_COUNT
 GOTO	    UART_RX_END		    ;HandShake Verified Increment Byte Count and Exit
 
BYTE_2	;**Second Received Byte Check for Robot Address**
 BANKSEL    ROBOT_ADDR
 MOVFW	    ROBOT_ADDR
 BANKSEL    RCREG
 SUBWF	    RCREG,0		    ;Subtract RCREG - ROBOT_ADDR; Result Store in W
 BTFSC	    STATUS,2
 GOTO	    ADDR_VERIFIED	    ;Robot Address Confirmed Increment Byte Count
 BANKSEL    BYTE_COUNT
 CLRF	    BYTE_COUNT
 GOTO	    UART_RX_END		    ;Robot Address Not Verified. Clear Count and Exit
ADDR_VERIFIED
 BANKSEL    BYTE_COUNT
 INCF	    BYTE_COUNT
 BANKSEL    CONNECT_CHK_COUNT
 CLRF	    CONNECT_CHK_COUNT	    ;Clear Connection Timeout Counter
 GOTO	    UART_RX_END		    ;Robot Address Verified Increment Byte Count and Exit
 
BYTE_3	;**Third Received Byte Save as Data Packet Command** 
 BANKSEL    RCREG
 MOVFW	    RCREG
 BANKSEL    CURRENT_DATA_PACK
 MOVWF	    CURRENT_DATA_PACK	    ;Save Command Byte into Current Data Packet
 BANKSEL    BYTE_COUNT
 INCF	    BYTE_COUNT		    ;Increment Byte Count
 GOTO	    UART_RX_END		    ;Exit Routine
 
BYTE_4	;**Fourth Received Byte Save as Memory Save 1**
 BANKSEL    RCREG
 MOVFW	    RCREG
 BANKSEL    MEM_SAVE_1
 MOVWF	    MEM_SAVE_1		    ;Save Data Variable 1 into Memory Save 1
 BANKSEL    BYTE_COUNT
 INCF	    BYTE_COUNT		    ;Increment Byte Count
 GOTO	    UART_RX_END		    ;Exit Routine
 
BYTE_5	;**Fifth Received Byte Save as Memory Save 2** 
 BANKSEL    RCREG
 MOVFW	    RCREG
 BANKSEL    MEM_SAVE_2
 MOVWF	    MEM_SAVE_2		    ;Save Data Variable 2 into Memory Save 2
 BANKSEL    BYTE_COUNT
 INCF	    BYTE_COUNT		    ;Increment Byte Count
 GOTO	    UART_RX_END		    ;Exit Routine
 
BYTE_6	;**Sixth Received Byte Save as Memory Save 3 and Set New Data Flag**
 BANKSEL    RCREG
 MOVFW	    RCREG
 BANKSEL    MEM_SAVE_3
 MOVWF	    MEM_SAVE_3		    ;Save Data Variable 3 into Memory Save 3
 BANKSEL    NEW_RX_DATA
 BSF	    NEW_RX_DATA,0	    ;Set New Data Flag to Initiate URC Decoding
 BANKSEL    BYTE_COUNT
 CLRF	    BYTE_COUNT		    ;Clear Byte Count for Next Data Packet
 
UART_RX_END
 RETURN

 
;********************************************************************************************
;****DECODE_URC Function Will Determine The Data Saved in Temporary "MEM_SAVE" Registers. ***
;**** This is Based on the 3rd Data Byte UART Received Known as "CURRENT_DATA_PACK"	  ***
;**** See Data Pack Layout Above and Additional Materials for URC			  ***
;********************************************************************************************
DECODE_URC ;***Decode Memory Save 1-3 From Command Byte***
 BANKSEL    CURRENT_DATA_PACK
 MOVLW	    H'4A'		    ;ASCII "J"
 SUBWF	    CURRENT_DATA_PACK,0	    ;Subtract CURRENT_DATA_PACK - W; Result Store in W
 BTFSC	    STATUS,2		    ;Check Zero Bit.  1 if Zero; 0 if not Zero
 GOTO	    JOYSTICK_DATA_1	    ;Data is Joystick Data 1 Packet
 
 MOVLW	    H'6A'		    ;ASCII "j"
 SUBWF	    CURRENT_DATA_PACK,0	    ;Subtract CURRENT_DATA_PACK - W; Result Store in W
 BTFSC	    STATUS,2		    ;Check Zero Bit.  1 if Zero; 0 if not Zero
 GOTO	    JOYSTICK_DATA_2	    ;Data is Joystick Data 2 Packet
 
 MOVLW	    H'42'
 SUBWF	    CURRENT_DATA_PACK,0	    ;Subtract CURRENT_DATA_PACK - W; Result Store in W
 BTFSC	    STATUS,2		    ;Check Zero Bit.  1 if Zero; 0 if not Zero
 GOTO	    BUTTON_DATA		    ;Data is Button Data Packet
 GOTO	    DECODE_URC_END	    ;Data Pack Unkown Exit Routine (Should Not Happen)
 
JOYSTICK_DATA_1
 BANKSEL    MEM_SAVE_1
 MOVFW	    MEM_SAVE_1
 BANKSEL    JOY1_UD
 MOVWF	    JOY1_UD		    ;First Data Variable is Joystick 1 Up and Down
 
 BANKSEL    MEM_SAVE_2
 MOVFW	    MEM_SAVE_2
 BANKSEL    JOY1_LR
 MOVWF	    JOY1_LR		    ;Second Data Variable is Joystick 1 Left and Right
 
 BANKSEL    MEM_SAVE_3
 MOVFW	    MEM_SAVE_3
 BANKSEL    JOY2_UD
 MOVWF	    JOY2_UD		    ;Third Data Variable is Joystick 2 Up and Down
 
 GOTO	    DECODE_URC_END	    ;Exit Routine
 
JOYSTICK_DATA_2
 BANKSEL    MEM_SAVE_1
 MOVFW	    MEM_SAVE_1
 BANKSEL    JOY2_LR
 MOVWF	    JOY2_LR		    ;First Data Variable is Joystick 2 Left and Right
 
 BANKSEL    MEM_SAVE_2
 MOVFW	    MEM_SAVE_2
 BANKSEL    JOY3_UD
 MOVWF	    JOY3_UD		    ;Second Data Variable is Joystick 3 Up and Down
 
 BANKSEL    MEM_SAVE_3
 MOVFW	    MEM_SAVE_3
 BANKSEL    JOY3_LR
 MOVWF	    JOY3_LR		    ;Third Data Variable is Joystick 3 Left and Right
 
 GOTO	    DECODE_URC_END	    ;Exit Routine
 
BUTTON_DATA
 BANKSEL    MEM_SAVE_1
 MOVFW	    MEM_SAVE_1
 BANKSEL    BUTTON_STATUS_1
 MOVWF	    BUTTON_STATUS_1	    ;First Data Variable is Controller Button Status
 BANKSEL    MEM_SAVE_2
 MOVFW	    MEM_SAVE_2
 BANKSEL    BUTTON_STATUS_2
 MOVWF	    BUTTON_STATUS_2	    ;Second Data Variable is Last Controller Button Status (bit 0)
				    ;Third Data Variable is not used in the button data packet
				   
DECODE_URC_END
 CLRF	    CURRENT_DATA_PACK
 CLRF	    MEM_SAVE_1
 CLRF	    MEM_SAVE_2
 CLRF	    MEM_SAVE_3		    ;Data is Saved Clear Temp Save Registers
 BCF	    NEW_RX_DATA,0	    ;Remove New Data Flag
 
 RETURN
 
	
 
 
 