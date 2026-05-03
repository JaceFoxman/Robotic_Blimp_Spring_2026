;***************************************************************************
;
;Filename: Blimp_Peripheral_Slave.S
;Date: 03/30/2026
;File Version: 1
;Author: Jason Permann
;Co-Author: 
;Company: Idaho State University
;Description: 
;	
			    
;**************************************************************************
	
;*************************************************************************
; 
;Revision History:
;   
;Modified as listed
;Started 03/30/2026
;
;*************************************************************************
    
; PIC16F1788 Configuration Bit Settings

; Assembly source line config statements
	#include <xc.inc>
	#include "pic16f1788.inc"
	#include "88SetUp.inc"
	#include "I2C_Handeler.inc"
	
; CONFIG1
  CONFIG  FOSC = INTOSC         ; Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
  CONFIG  WDTE = OFF            ; Watchdog Timer Enable (WDT disabled)
  CONFIG  PWRTE = OFF           ; Power-up Timer Enable (PWRT disabled)
  CONFIG  MCLRE = ON            ; MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
  CONFIG  CP = OFF              ; Flash Program Memory Code Protection (Program memory code protection is disabled)
  CONFIG  CPD = OFF             ; Data Memory Code Protection (Data memory code protection is disabled)
  CONFIG  BOREN = OFF           ; Brown-out Reset Enable (Brown-out Reset disabled)
  CONFIG  CLKOUTEN = ON         ; Clock Out Enable (CLKOUT function is enabled on the CLKOUT pin)
  CONFIG  IESO = OFF            ; Internal/External Switchover (Internal/External Switchover mode is disabled)
  CONFIG  FCMEN = OFF           ; Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is disabled)

; CONFIG2
  CONFIG  WRT = OFF             ; Flash Memory Self-Write Protection (Write protection off)
  CONFIG  VCAPEN = OFF          ; Voltage Regulator Capacitor Enable bit (Vcap functionality is disabled on RA6.)
  CONFIG  PLLEN = OFF           ; PLL Enable (4x PLL enabled)
  CONFIG  STVREN = OFF          ; Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will not cause a Reset)
  CONFIG  BORV = HI             ; Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), high trip point selected.)
  CONFIG  LPBOR = OFF           ; Low Power Brown-Out Reset Enable Bit (Low power brown-out is disabled)
  CONFIG  DEBUG = OFF           ; In-Circuit Debugger Mode (In-Circuit Debugger disabled, ICSPCLK and ICSPDAT are general purpose I/O pins)
  CONFIG  LVP = OFF             ; Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

;Properties
;-Wl,-presetVect=0h,-pisrVect=04h,-pcode=08h
  
; Code Section
;-------------------------------------------------------------------------------

;-----------------------------Common Ram----------------------------------------
;Register/Variable Setup	
//<editor-fold defaultstate="collapsed" desc="Registers">
ADDRESS_W		EQU	0x70	    ;Temp register for storing the peripheral address write (I2C Master)
ADDRESS_R		EQU	0x71	    ;Temp register for storing the peripheral address read (I2C Master)
I2C_RX_TEMP_1		EQU	0x72	    ;Temp register for storing the first data byte received 
I2C_RX_TEMP_2		EQU	0x73	    ;Temp register for storing the second data byte received
I2C_RX_TEMP_3		EQU	0x74	    ;Temp register for storing the third data byte received
I2C_RX_TEMP_4		EQU	0x75	    ;Temp register for storing the fourth data byte received
I2C_RX_COUNTER		EQU	0x76	    ;Register to track which data byte was las received
I2C_RX_COMPLETE		EQU	0x77	    ;Flag to Signal when full data packet has been received
BANK_SAVE		EQU	0x78
W_SAVE			EQU	0x79
//</editor-fold>

;Peripheral data			
//<editor-fold defaultstate="collapsed" desc="Joystick_reg">
JOY3_UD			EQU	0X7A	    ;Register to Save Joystick 3 Up-Down Data
JOY3_LR			EQU	0X7B	    ;Register to Save Joystick 3 Left-Right Data
BUTTONS_1		EQU	0X7C	    ;Register to Save BUTTONS Data Packet 1
BUTTONS_2		EQU	0X7D	    ;Register to Save BUTTONS_2 Data Packet 2//</editor-fold>
;----------------------Bank 0 Gerneral purpouse---------------------------------
PERIOD_VALUE		EQU	0x20
PWM_CHECK_BYTE		EQU	0x21
PW1_VALUE		EQU	0x22	;PWM of JOY3_UD
PW2_VALUE		EQU	0x23	;Right Servo Pitch
I2C_TIMEOUT_COUNT	EQU	0x24	;Track I2C timeout

		
		
;Start Of Program
    PSECT resetVect,class=CODE,delta=2	;Reset Vector Adress
    GOTO    SETUP
    
    PSECT isrVect,class=CODE,delta=2
    GOTO    INTERUPT_HANDLER
    
; Setup Code That Runs Once At Power Up/Reset
    PSECT code,class=CODE,delta=2
;-------------------------------------------------------------------------------
;<editor-fold defaultstate="collapsed" desc="MAIN">
MAIN:
    ;************************************************************************************************
    ;*** UPDATE_DRIVE_STATUS will run every time a full 4 byte I2C Packet is received.  The flag  ***
    ;*** I2C_RX_COMPLETE Bit 0 is set within the I2C_RECEIVE function after the fourth data       ***
    ;*** byte is received.  See Unv_Slave_I2C_SetUp.inc					          ***
    ;************************************************************************************************
    BANKSEL	I2C_RX_COMPLETE
    BTFSC	I2C_RX_COMPLETE,0
    CALL	UPDATE_DRIVE_STATUS	    ;Full I2C Data Packet Received.  Save Temp Data, Check Joysticks, Update Outputs
    CALL	UPDATE_PORTB
    BANKSEL	SSP1CON1
    BSF		SSP1CON1,4				;Re-enable Clock (stop clock strechting)
    GOTO	MAIN;</editor-fold>
    
//<editor-fold defaultstate="collapsed" desc="INTERUPT_HANDLER">
INTERUPT_HANDLER:
    MOVWF   W_SAVE		
    MOVF    BSR, 0	    ;Save Bank & W
    MOVWF   BANK_SAVE
    
    ;Handles I2C receive and temporary data save functions
    BANKSEL PIR1	    ;adding this caused the address to function properly
    BTFSC   PIR1,3
    CALL    I2C_RECEIVE	    ;Test for MSSP Interrupt
    
    ;Handles loss of I2C connection
    BANKSEL PIR1
    BTFSC   PIR1,0
    CALL    I2C_COM_TIMEOUT ;Test for Timer 1 Interrupt
    
    BANKSEL PIR1
    BTFSC   PIR1,1	;Check if TMR2 interrupt flag was set
    CALL    PWM_INC
    
    GOTO    INT_END
    //</editor-fold>
    
//<editor-fold defaultstate="collapsed" desc="INT_END">
INT_END:
    BANKSEL PIR1
    MOVLW   0x00
    MOVWF   PIR1
    
    MOVF    BANK_SAVE, 0	
    MOVWF   BSR		    ;Restore Bank & W
    MOVF    W_SAVE, 0
    RETFIE//</editor-fold>

//<editor-fold defaultstate="collapsed" desc="JOYSTICK_STARTUP">
JOYSTICK_STARTUP:
;*** SFR SETUP **********
    BANKSEL	JOY3_UD
    MOVLW	0x80	    ;Decimal 128
    MOVWF	JOY3_UD
    BANKSEL	JOY3_LR
    MOVLW	0x80	    ;Decimal 128
    MOVWF	JOY3_LR	    ;Begin With all Joysticks at "Center
    RETURN//</editor-fold>
    
//<editor-fold defaultstate="collapsed" desc="PWM_STARTUP">
PWM_STARTUP:
    BANKSEL	PWM_CHECK_BYTE
    MOVLW	0x00
    MOVWF	PWM_CHECK_BYTE
    
    BANKSEL	PERIOD_VALUE
    MOVLW	0xC8	    ;decimal 200 = period 20ms
    MOVWF	PERIOD_VALUE
    
    BANKSEL	PW2_VALUE
    MOVLW	0x0F	    ;decimal 128, start camera at middle
    MOVWF	PW2_VALUE
    
    BANKSEL	PORTA
    BSF		PORTA,0
    
    RETURN//</editor-fold>
;*************************************************************************************************
;*** SAVE_I2C_DATA is needed to convert the temporary I2C save registers into general          ***
;*** purpose registers.  This should correlate with the data sent from the master.             ***
;*** This Template assumes motors will be controlled from joystick 1 and 2.		       ***
;*************************************************************************************************
//<editor-fold defaultstate="collapsed" desc="UPDATE_DRIVE_STATUS">
UPDATE_DRIVE_STATUS:
    BANKSEL	I2C_RX_TEMP_1
    MOVF	I2C_RX_TEMP_1,0
    BANKSEL	JOY3_UD
    MOVWF	JOY3_UD
    BANKSEL	I2C_RX_TEMP_2
    MOVF	I2C_RX_TEMP_2,0
    BANKSEL	JOY3_LR	
    MOVWF	JOY3_LR
    BANKSEL	I2C_RX_TEMP_3
    MOVF	I2C_RX_TEMP_3,0
    BANKSEL	BUTTONS_1
    MOVWF	BUTTONS_1
    BANKSEL	I2C_RX_TEMP_4
    MOVF	I2C_RX_TEMP_4,0
    BANKSEL	BUTTONS_2
    MOVWF	BUTTONS_2
    RETURN//</editor-fold>

//<editor-fold defaultstate="collapsed" desc="UPDATE_PORTB">
UPDATE_PORTB:
    BANKSEL	PW1_VALUE
    MOVF	PW1_VALUE,0
    BANKSEL	PORTB
    MOVWF	PORTB
    RETURN//</editor-fold>
    
//<editor-fold defaultstate="collapsed" desc="PWM_INC">
PWM_INC:    
    BANKSEL	PWM_CHECK_BYTE
    INCF	PWM_CHECK_BYTE,1
    
    
    BANKSEL	PW1_VALUE
    MOVF	PW1_VALUE,0
    
    BANKSEL	PWM_CHECK_BYTE
    XORWF	PWM_CHECK_BYTE,0
    
    BANKSEL	STATUS
    BTFSC	STATUS,2
    CALL	CLEAR_PORTA_THROTTLE_01
    
    BANKSEL	PWM_CHECK_BYTE
    MOVF	PWM_CHECK_BYTE,0
    
    BANKSEL	PW1_VALUE
    SUBWF	PW1_VALUE,0	    ;check if PW value is greater than increment value
    
    BANKSEL	STATUS
    BTFSS	STATUS,0    
    CALL	CLEAR_PORTA_THROTTLE_01
    
    
    
    BANKSEL	PW2_VALUE
    MOVF	PW2_VALUE,0
    
    BANKSEL	PWM_CHECK_BYTE
    XORWF	PWM_CHECK_BYTE,0
    
    BANKSEL	STATUS
    BTFSC	STATUS,2
    CALL	CLEAR_PORTA_2
    
    BANKSEL	PERIOD_VALUE
    MOVF	PERIOD_VALUE,0
    
    BANKSEL	PWM_CHECK_BYTE
    XORWF	PWM_CHECK_BYTE,0
    
    BANKSEL	STATUS
    BTFSC	STATUS,2
    CALL	PERIOD_END
    RETURN//</editor-fold>
   
//<editor-fold defaultstate="collapsed" desc="CLEAR_PORTA_THROTTLE_01">
CLEAR_PORTA_THROTTLE_01:
    BANKSEL	PORTA
    BCF		PORTA,0
    BCF		PORTA,1
    BANKSEL	STATUS
    BCF		STATUS,2
    BCF		STATUS,0
    RETURN//</editor-fold>
    
//<editor-fold defaultstate="collapsed" desc="CLEAR_PORTA_2">
CLEAR_PORTA_2:
    BANKSEL	PORTA
    BCF		PORTA,2
    BANKSEL	STATUS
    BCF		STATUS,2
    BCF		STATUS,0
    RETURN//</editor-fold>

;<editor-fold defaultstate="collapsed" desc="PERIOD_END">
PERIOD_END:
    BANKSEL	PORTA
    BSF		PORTA,0
    BSF		PORTA,1
    BSF		PORTA,2
    BANKSEL	STATUS
    BCF		STATUS,2    ;clear zero bit
    BCF		STATUS,0    ;clear carry bit (check for negative number)
    BANKSEL	PWM_CHECK_BYTE
    MOVLW	0x00
    MOVWF	PWM_CHECK_BYTE
    
    BANKSEL	PORTA
    BTFSC	PORTA,3
    GOTO	COMM_LOSS
    GOTO	COMM_GOOD
    
    PERIOD_END_RETURN:
    RETURN;</editor-fold>
    
    COMM_GOOD:
    CALL	WEIGHT_JOYSTICK_3_DATA
    CALL	CAMERA_PWM
    GOTO	PERIOD_END_RETURN
    COMM_LOSS:
    BANKSEL	PW1_VALUE
    MOVLW	0x0F
    MOVWF	PW1_VALUE
    GOTO	PERIOD_END_RETURN
//<editor-fold defaultstate="collapsed" desc="WEIGHT JOYSTICK 3 DATA">
WEIGHT_JOYSTICK_3_DATA:
    BANKSEL PW1_VALUE
    CLRF    PW1_VALUE
    
    BANKSEL JOY3_UD
    MOVLW   0xFF
    XORWF   JOY3_UD,1	;We found that more thrust was achived with flipping the polarity of the motors witch means we need to invert the throttle.
    MOVF    JOY3_UD,0
    
   ;BANKSEL JOY3_UD	;If you need to uninvert the throttle uncomment this code and comment out the above section
   ;MOVF    JOY3_UD,0   
    
    BANKSEL PW1_VALUE
    MOVWF   PW1_VALUE
    
    RRF	    PW1_VALUE,1
    RRF	    PW1_VALUE,1
    RRF	    PW1_VALUE,1
    
    MOVLW   0x1F
    ANDWF   PW1_VALUE,1
    
    RETURN//</editor-fold>

//<editor-fold defaultstate="collapsed" desc="CAMERA_CONTROL">
CAMERA_PWM:
    BANKSEL BUTTONS_1
    BTFSC   BUTTONS_1,0	    ;Move camera down
    CALL    CAMERA_DOWN
    
    BANKSEL BUTTONS_1
    BTFSC   BUTTONS_1,3	    ;Move camera Up
    CALL    CAMERA_UP
    
    BANKSEL BUTTONS_1
    BTFSC   BUTTONS_1,1	    ;Move camera to middle
    CALL    CAMERA_MIDDLE
    
    RETURN
    
CAMERA_DOWN:
    BANKSEL PW2_VALUE
    BTFSS   PW2_VALUE,0
    CALL    CHECK_IF_AT_0
    DECF    PW2_VALUE,1
    RETURN
    
CAMERA_UP:    
    BANKSEL PW2_VALUE
    BTFSC   PW2_VALUE,4
    CALL    CHECK_IF_AT_26
    INCF    PW2_VALUE,1
    RETURN
    
CAMERA_MIDDLE:
    BANKSEL PW2_VALUE
    MOVLW   0x0F
    MOVWF   PW2_VALUE
    RETURN
    
CHECK_IF_AT_26:
    BANKSEL	PW2_VALUE
    MOVLW	0x1A
    XORWF	PW2_VALUE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    CALL	SET_AT_25
    RETURN
    
CHECK_IF_AT_0:
    BANKSEL	PW2_VALUE
    MOVLW	0x00
    XORWF	PW2_VALUE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    CALL	SET_AT_0
    RETURN
    
SET_AT_0:
    BANKSEL	PW2_VALUE
    MOVLW	0x01
    MOVWF	PW2_VALUE
    RETURN
    
SET_AT_25:
    BANKSEL	PW2_VALUE
    MOVLW	0x19
    MOVWF	PW2_VALUE
    RETURN
    //</editor-fold> 
    
I2C_COM_TIMEOUT:
    ;******Occurs every 130mS*******
    BANKSEL	I2C_TIMEOUT_COUNT
    DECFSZ	I2C_TIMEOUT_COUNT		    ;Slow I2C COM Timeout Count 
    GOTO	I2C_COM_TIMEOUT_END
    MOVLW	0x02
    MOVWF	I2C_TIMEOUT_COUNT		    ;Reset I2C COM Timeout Count
    
    BANKSEL	PORTA
    BSF		PORTA,3

    GOTO	I2C_COM_TIMEOUT_END
I2C_COM_TIMEOUT_END:
    BANKSEL	PIR1
    BCF		PIR1,0				    ;Clear Timer 1 Interrupt Flag
    
    RETURN
;Subroutines Section
//<editor-fold defaultstate="collapsed" desc="SETUP">
SETUP:
    CALL    SETUP88		    ;Call is needed as the setup88 include file has a return at the end 
    CALL    I2C_SETUP_PERIPHERAL    ;Call is needed as the I2C interupt include file has a return at the end of the peripheral setup
    CALL    JOYSTICK_STARTUP
    CALL    PWM_STARTUP
    BANKSEL T1CON
    BSF	    T1CON,0
    GOTO    MAIN
    
    //</editor-fold>   
;-------------------------------------------------------------------------------
END 