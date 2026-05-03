;****************************************************************************
;									    *
;	    File Name: URC_UART.inc					    *
;	    Date: 11/22/24						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State Univerisy; RCET			    *
;	    Description:  UART Functions for Transmitting Controller State  *
;									    *
;****************************************************************************
    
UART_CODE   CODE
  
;******************************************************************************************		
;Sends the Data Packet Containing Handshake, Address, Command, Byte 1, Byte 2, and Byte 3
;******************************************************************************************
SEND_DATA_PACKET
    BANKSEL	TXSTA
    BTFSS	TXSTA,1
    GOTO	SEND_DATA_PACKET	;Ensure the Buffer is Empty
    BANKSEL	TX_HANDSHAKE
    MOVFW	TX_HANDSHAKE
    BANKSEL	TXREG
    MOVWF	TXREG			;Load Handshake Byte ($; 0x24)
    BANKSEL	TXSTA
    BTFSS	TXSTA,1
    GOTO	$-2			;Wait Until HandShake TX Completes
    BANKSEL	TX_ADDR
    MOVFW	TX_ADDR
    BANKSEL	TXREG
    MOVWF	TXREG			;Load Robot Address Byte
    BANKSEL	TXSTA
    BTFSS	TXSTA,1
    GOTO	$-2			;Wait Until Robot Address TX Completes
    BANKSEL	TX_CMND
    MOVFW	TX_CMND
    BANKSEL	TXREG
    MOVWF	TXREG			;Load Command Byte
    BANKSEL	TXSTA
    BTFSS	TXSTA,1
    GOTO	$-2			;Wait Until Command Byte TX Completes
    BANKSEL	TX_DATA_1
    MOVFW	TX_DATA_1
    BANKSEL	TXREG
    MOVWF	TXREG			;Load First Data Byte
    BANKSEL	TXSTA
    BTFSS	TXSTA,1
    GOTO	$-2			;Wait Until First Data TX Completes
    BANKSEL	TX_DATA_2
    MOVFW	TX_DATA_2
    BANKSEL	TXREG
    MOVWF	TXREG			;Load Second Data Byte
    BANKSEL	TXSTA
    BTFSS	TXSTA,1
    GOTO	$-2			;Wait Until Seond Data TX Completes
    BANKSEL	TX_DATA_3
    MOVFW	TX_DATA_3
    BANKSEL	TXREG
    MOVWF	TXREG			;Load Third Data Byte
    BANKSEL	TXSTA
    BTFSS	TXSTA,1
    GOTO	$-2			;Wait Until Third Data TX Completes
   RETURN
   
;******************************************************************************************		
;Determine Packet To Be Sent and Call Packet Initialiazation
;******************************************************************************************
DETERMINE_PACKET
   BANKSEL	TX_COUNT
   MOVLW	H'00'
   SUBWF	TX_COUNT,0		;Subtract 0 from TX_COUNT Store in W
   BANKSEL	STATUS
   BTFSC	STATUS,2		;Skip if TX_COUNT is not 0
   GOTO		PACKET_1
   BANKSEL	TX_COUNT
   MOVLW	H'01'
   SUBWF	TX_COUNT,0		;Subtract 1 from TX_COUNT Store in W
   BANKSEL	STATUS
   BTFSC	STATUS,2		;Skip if TX_COUNT is not 1
   GOTO		PACKET_2
   BANKSEL	TX_COUNT
   MOVLW	H'02'
   SUBWF	TX_COUNT,0		;Subtract 2 from TX_COUNT Store in W
   BANKSEL	STATUS
   BTFSC	STATUS,2		;Skip if TX_COUNT is not 2
   GOTO		PACKET_3
;*************************************************************************************
;*****To add More Data Packets add Packet Check Here**********************************
;*************************************************************************************
;   BANKSEL	TX_COUNT
;   MOVLW	H'xx'
;   SUBWF	TX_COUNT,0		;Subtract x from TX_COUNT Store in W
;   BANKSEL	STATUS
;   BTFSC	STATUS,2		;Skip if TX_COUNT is not x
;   GOTO	PACKET_x
;**************************^^Template For Extending Packet #^^*************************
;**************************************************************************************
   CLRF		TX_COUNT		;Packet Count Unknown (Not 0-2) Reset to Packet 1
   GOTO		DETERMINE_PACKET_END	;TX_COUNT IS Not 0, 1, OR 2 Clear Count and Exit
PACKET_1
   INCF		TX_COUNT		;Increment Packet Count to TX Joystick 2 Packet Next
   CALL		INIT_JOY1_TX		;Set Command Byte and Data Bytes for Joystick 1 Packet
   GOTO		DETERMINE_PACKET_END	;Exit Routine
PACKET_2
   INCF		TX_COUNT		;Increment Packet Count to TX Buttons Next
   CALL		INIT_JOY2_TX		;Set Command Byte and Data Bytes for Joystick 2 Packet
   GOTO		DETERMINE_PACKET_END	;Exit Routine
PACKET_3
   CLRF		TX_COUNT		;Clear Packet Count to TX Joystick 1 Packet Next 
   CALL		INIT_BUTTONS_TX		;Set Command Byte and Data Bytes for Buttons Packet
   GOTO		DETERMINE_PACKET_END	;Exit Routine
;*************************************************************************************
;**********To add More Data Packets Add Packet Command and Data Here******************
;*************************************************************************************
;PACKET_x
;   INCF		TX_COUNT		;Increment Packet Count to TX Next Packet (CLEAR IF LAST PACKET) 
;   CALL		INIT_X_TX		;Set Command Byte and Data Bytes for Desired Packet
;   GOTO		DETERMINE_PACKET_END	;Exit Routine
;**************************^^Template For Extending Packet #^^*************************
;**************************************************************************************
DETERMINE_PACKET_END
   RETURN
   
;******************************************************************************************		
;Load TX Registers With First Joystick Data Packet Bytes
;******************************************************************************************
INIT_JOY1_TX
   BANKSEL	JOY_1_CMND
   MOVFW	JOY_1_CMND
   BANKSEL	TX_CMND
   MOVWF	TX_CMND			;Save Command Byte as: "J" for First Joystick Data Packet
   BANKSEL	JOYSTICK_1_UD
   MOVFW	JOYSTICK_1_UD
   BANKSEL	TX_DATA_1
   MOVWF	TX_DATA_1		;Save Left Joystick(1) Up Down Data as First Byte
   BANKSEL	JOYSTICK_1_LR
   MOVFW	JOYSTICK_1_LR
   BANKSEL	TX_DATA_2
   MOVWF	TX_DATA_2		;Save Left Joystick(1) Left Right Data as Second Byte
   BANKSEL	JOYSTICK_2_UD
   MOVFW	JOYSTICK_2_UD
   BANKSEL	TX_DATA_3
   MOVWF	TX_DATA_3		;Save Right Lower Joystick(2) Up Down Data as Third Byte
   RETURN
   
;******************************************************************************************		
;Load TX Registers With Second Joystick Data Packet Bytes
;******************************************************************************************
INIT_JOY2_TX
   BANKSEL	JOY_2_CMND
   MOVFW	JOY_2_CMND
   BANKSEL	TX_CMND
   MOVWF	TX_CMND			;Save Command Byte as: "j" for Second Joystick Data Packet
   BANKSEL	JOYSTICK_2_LR
   MOVFW	JOYSTICK_2_LR
   BANKSEL	TX_DATA_1
   MOVWF	TX_DATA_1		;Save Right Joystick(2) Left Right Data as First Byte
   BANKSEL	JOYSTICK_3_UD
   MOVFW	JOYSTICK_3_UD
   BANKSEL	TX_DATA_2
   MOVWF	TX_DATA_2		;Save Right Upper Joystick(3) Up Down Data as Second Byte
   BANKSEL	JOYSTICK_3_LR
   MOVFW	JOYSTICK_3_LR
   BANKSEL	TX_DATA_3
   MOVWF	TX_DATA_3		;Save Right Upper Joystick(3) Left Right Data as Third Byte
   RETURN
   
;******************************************************************************************		
;Load TX Registers With Buttons Data Packet Bytes
;******************************************************************************************
INIT_BUTTONS_TX
   BANKSEL	BUTTONS_CMND
   MOVFW	BUTTONS_CMND
   BANKSEL	TX_CMND
   MOVWF	TX_CMND			;Save Command Byte as: "b" for Button Data Packet
   BANKSEL	BUTTON_DATA_1
   MOVFW	BUTTON_DATA_1
   BANKSEL	TX_DATA_1
   MOVWF	TX_DATA_1		;Save First Button Data Packet as First Byte
   BANKSEL	BUTTON_DATA_2
   MOVFW	BUTTON_DATA_2
   BANKSEL	TX_DATA_2
   MOVWF	TX_DATA_2		;Save Second Button Data Packet as Second Byte
   BANKSEL	TX_DATA_3
   CLRF		TX_DATA_3		;Clear The Third Byte as this Packet Does not Need this Data
   BANKSEL	BUTTON_DATA_1
   CLRF		BUTTON_DATA_1
   CLRF		BUTTON_DATA_2		;Clear Buttons Pressed Data Already Stored For TX
   RETURN
   
;******************************************************************************************		
;***To add More Data Packets. Load TX Registers With Command and Packet Bytes Here*********
;******************************************************************************************
;INIT_x_TX
;   BANKSEL	x_CMND
;   MOVFW	x_CMND
;   BANKSEL	TX_CMND
;   MOVWF	TX_CMND			;Save Command Byte as: "x" for ____ Data Packet
;   BANKSEL	X_DATA_VAR_1
;   MOVFW	X_DATA_CAR_1
;   BANKSEL	TX_DATA_1
;   MOVWF	TX_DATA_1		;Save First _____ Data Packet as First Byte
;   BANKSEL	X_DATA_VAR_2
;   MOVFW	X_DATA_VAR_2
;   BANKSEL	TX_DATA_2
;   MOVWF	TX_DATA_2		;Save Second ______ Data Packet as Second Byte
;   BANKSEL	X_DATA_VAR_2
;   MOVFW	X_DATA_VAR_3
;   BANKSEL	TX_DATA_3
;   MOVWF	TX_DATA_3		;Save Third ______ Data Packet as Second Byte
;   RETURN
;**************************^^Template For Extending Packet #^^*************************
;**************************************************************************************   