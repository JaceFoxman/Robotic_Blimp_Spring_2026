;****************************************************************************
;									    *
;	    File Name: Unv_Master_PIC_SetUp.INC				    *
;	    Date: 4/29/25						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State University; RCET			    *
;	    Description: Setup for PIC16f1788 for Universal Control	    *
;			 System	for Master Board			    * 
;									    *
;****************************************************************************
;									    *
;	    Revision History:						    *
;	1. Added Comments and Adjusted for Template Code;4/29/25	    *
;	2.								    *
;	3.								    *
;	4.								    *
;	5.								    *
;	6.								    *
;	7.								    *
;	8.								    *
;	9.								    *
;	10.								    *
;									    *
;****************************************************************************
    
;******************************************		
;Define Constants
;******************************************
#Define 	BAUD		D'100'			;Desired Baud Rate in Kbps for I2C
#Define		FOSC		D'16000'		;Oscillator Clock in KHz This must be filled
    
;******************************************		
;Define Variable Registers
;******************************************
BYTE_COUNT	    EQU	0X20	    ;Register to count Receied Bytes
ROBOT_ADDR	    EQU	0X21	    ;Register to Save Robot Address (Port B Dip Switch Values)
JOY1_UD		    EQU	0X22	    ;Register to Save Joystick 1 Up-Down Data
JOY1_LR		    EQU	0X23	    ;Register to Save Joystick 1 Left-Right Data
JOY2_UD		    EQU	0X24	    ;Register to Save Joystick 2 Up-Down Data
JOY2_LR		    EQU	0X25	    ;Register to Save Joystick 2 Left-Right Data
JOY3_UD		    EQU	0X26	    ;Register to Save Joystick 3 Up-Down Data
JOY3_LR		    EQU	0X27	    ;Register to Save Joystick 3 Left-Right Data
BUTTON_STATUS_1	    EQU	0X28	    ;Register to Save Button Status
BUTTON_STATUS_2	    EQU	0X29	    ;Register to Save Button Status
CURRENT_DATA_PACK   EQU	0X2A	    ;Register to Save Which Data Packet was Last Received
MEM_SAVE_1	    EQU 0X2B	    ;Register to temp save RX Data Variable 1
MEM_SAVE_2	    EQU 0X2C	    ;Register to temp save RX Data Variable 2
MEM_SAVE_3	    EQU 0X2D	    ;Register to temp save RX Data Variable 3
NEW_RX_DATA	    EQU 0X2E	    ;Register to Track New Data Status (bit 0 Set = New Data Available, Bit 1 for I2C Data Send)
CONNECT_CHK_COUNT   EQU	0X2F	    ;Register to Track Connection Status (Increment On TMR 2; Clear on RX)
I2C_COM_DELAY	    EQU	0X30	    ;Register to count when I2C COM should occur (Flag Set in Timer 2; NEW_RX_DATA,1)
;******************************************		
;Start of Setup File Code
;******************************************
SETUP_CODE CODE
 
INITIALIZE
;*** SFR SETUP **********
    BANKSEL	PORTB
    MOVFW	PORTB
    BANKSEL	ROBOT_ADDR
    MOVWF	ROBOT_ADDR	    ;Initialize Robot Address as the Status of PortB.
    
    CLRF	BYTE_COUNT	    ;Initialize with Byte count of 0
    
    MOVLW	D'128'
    MOVWF	JOY1_UD
    MOVWF	JOY1_LR
    MOVWF	JOY2_UD
    MOVWF	JOY2_LR
    MOVWF	JOY3_UD
    MOVWF	JOY3_LR		    ;Initialize With all Joystick Data at Mid Point
    CLRF	BUTTON_STATUS_1
    CLRF	BUTTON_STATUS_1	    ;Initialize With all Buttons Not Pressed
    CLRF	NEW_RX_DATA	    ;Initialize With No New Data
    CLRF	CONNECT_CHK_COUNT
    CLRF	I2C_COM_DELAY	    ;Initialize With Connection Time out and I2C Delay counts at 0
;***Oscillator Set up****
    BANKSEL	OSCCON
    MOVLW	H'78'
    MOVWF	OSCCON			;Set Internal Oscilator to 16Mhz

    BANKSEL	CLKRCON
    MOVLW	H'00'
    MOVWF	CLKRCON			;Disable Output CLock Reference
 
;*** SET OPTION_REG: ****
    BANKSEL	OPTION_REG
    CLRF	OPTION_REG		;Weak Pull Up Disabled, Interrupt Edge Selection, Timer Clock Set-Up
    
;*** SET CCPXCON REG: **
    BANKSEL	CCP1CON
    CLRF	CCP1CON			;DISABLE PWM & CCP
    
    BANKSEL	CCP2CON
    CLRF	CCP2CON
    
    BANKSEL	CCP3CON
    CLRF	CCP3CON
;*** TIMER 1 SETUP *****
    BANKSEL	T1CON
    CLRF	T1CON			;DISABLE TIMER 1
    
;*** TIMER 2 SETUP *****
    BANKSEL	T2CON
    MOVLW	H'00'			;Sets Prescaler and Postscaler to 1:1 (Wait to enable timer)
    MOVWF	T2CON
    BANKSEL	PR2		
    MOVLW	D'250'			;Set PR2 count 
    MOVWF	PR2	
    
;*** Alternate Pin Functions ****
    BANKSEL	APFCON1
    MOVLW	H'00'
    MOVWF	APFCON1			;Set Comm on Port C CCP on Port B
    
    BANKSEL	APFCON2
    MOVLW	H'01'
    MOVWF	APFCON2			;Set Save/Select on Port A CCP3 on Port B
    
;*** PORT A SETUP ****
    BANKSEL	ANSELA
    CLRF	ANSELA			;Disable Analog Input
    
    BANKSEL	WPUA
    CLRF	WPUA			;Disable Weak Pull Ups
    
    BANKSEL	ODCONA
    CLRF	ODCONA			;Disable Open Drain
    
    BANKSEL	SLRCONA
    CLRF	SLRCONA			;Disable Slew Rate Limit
    
    BANKSEL	INLVLA
    MOVLW	H'FF'
    MOVWF	INLVLA			;Set Input Threshold Voltage to CMOS Levels
    
    BANKSEL	LATA
    MOVLW	H'FF'
    MOVWF	LATA			;Clear Input Latches
    
    BANKSEL	TRISA
    MOVLW	H'00'
    MOVWF	TRISA 			;SET Port A as Output
    
    BANKSEL	PORTA
    MOVLW	H'FF'
    MOVWF	PORTA			;Initialize with Port A all High
    
;*** PORT B SETUP **** 
    BANKSEL	ANSELB
    CLRF	ANSELB			;Disable Analog Input
    
    BANKSEL	WPUB
    CLRF	WPUB			;Disable Weak Pull Ups
    
    BANKSEL	ODCONB
    CLRF	ODCONB			;Disable Open Drain
    
    BANKSEL	SLRCONB
    CLRF	SLRCONB			;Disable Slew Rate Limit
    
    BANKSEL	INLVLB
    MOVLW	H'FF'
    MOVWF	INLVLB			;Set Input Threshold Voltage to CMOS Levels
    
    BANKSEL	LATB
    CLRF	LATB			;Clear Input Latches
    
    BANKSEL	TRISB
    MOVLW	H'FF'
    MOVWF	TRISB 			;SET Port B as Inputs
    
;*** PORT C SETUP **** 
    BANKSEL	ANSELC
    CLRF	ANSELC			;Disable Analog Input
    
    BANKSEL	WPUC
    CLRF	WPUC			;Disable Weak Pull Ups
    
    BANKSEL	ODCONC
    CLRF	ODCONC			;Disable Open Drain
    
    BANKSEL	SLRCONC
    CLRF	SLRCONC			;Disable Slew Rate Limit
    
    BANKSEL	INLVLC
    MOVLW	H'FF'
    MOVWF	INLVLC			;Set Input Threshold Voltage to CMOS Levels
    
    BANKSEL	LATC
    CLRF	LATC			;Clear Input Latches
    
    BANKSEL	TRISC
    MOVLW	H'FF'
    MOVWF	TRISC 			;SET Port C as Output
    
    BANKSEL	PORTC
    BSF		PORTC,3
    BSF		PORTC,4
    
;*** PORT E SETUP ****
;28 Pin Package Does Not Have Port E
    
;*** Interrupt Set-Up : ****
    BANKSEL	INTCON
    MOVLW	H'40'			;Enable Peripheral Interrupts (Wait On Global)
    MOVWF	INTCON
    
    BANKSEL	PIE1
    MOVLW	H'22'
    MOVWF	PIE1			;Enable UART RX and Timer 2 Interrupts
 
    BANKSEL	PIE2
    MOVLW	H'00'
    MOVWF	PIE2			;Disable Interrupts
    
    BANKSEL	PIE3
    BCF		PIE3, 4			;Disable CCP3 Interrupt
    
    BANKSEL	PIE4
    MOVLW	H'00'
    MOVWF	PIE4			;Disable Interrupts
    
    ;*** UART Set Up: *****
    BANKSEL	TXSTA
    MOVLW	H'86'
    MOVWF	TXSTA			;Async, 8 Bit Tx, No Break Char, BRGH High (Wait On enable)
    
    BANKSEL	RCSTA
    MOVLW	H'80'
    MOVWF	RCSTA			;Serial Port Enable, 8 Bit Rx, Disable Adress Selection (Wait for Cont. Rx Enable)
    
    BANKSEL	BAUDCTL
    MOVLW	H'48'
    MOVWF	BAUDCTL			;Tx Non-Inverted, Baud Rate Gen 16 Bit, Auto Baud Detect Off
    
    BANKSEL	SPBRG
    MOVLW	D'34'
    MOVWF	SPBRG			
    
    BANKSEL	SPBRGH
    CLRF	SPBRGH			;Set Baud Rate to 115.2K  ( ((16MHz/115.2K)/4) - 1 = 34 )
    
    RETURN				;END OF SETUP ROUTINE