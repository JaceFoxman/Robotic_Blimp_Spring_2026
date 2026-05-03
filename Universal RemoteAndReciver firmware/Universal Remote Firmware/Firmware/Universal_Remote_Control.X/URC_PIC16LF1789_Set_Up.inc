;****************************************************************************
;									    *
;	    File Name: URC_PIC16LF1789_Set_Up.inc			    *
;	    Date: 11/21/24						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State Univerisy; RCET			    *
;	    Description:  Firmware Setup for PIC16f1789			    *
;									    *
;****************************************************************************
;									    *
;	    Revision History:						    *
;	1.Set Up Internal Oscillator; 11/21/24				    *
;	2.Set Up Port Inputs and Outputs according to PCB Layout; 11/21/24  *
;	3.Set Up USART COM; 11/21/24					    *
;	4.Added GPRs for Buttons and Joystick Data; 11/21/24		    *
;	5.Added GPRs for Transmission Data; 11/22/24			    *
;	6.Set Up ADC Module; 11/24/24					    *
;	7.Added GPRs for Analog Selection Channels; 11/24/24		    *
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
TIMERCOUNT	    EQU		0X20	    ;Register to Slow Timer 2 to 0.5s Interrupts
ROBOT_ADDR	    EQU		0X21	    ;Register for Saving the Robot Address
JOY_1_CMND	    EQU		0X22	    ;Register for Saving the First Joystick Data Packet Command
JOY_2_CMND	    EQU		0X23	    ;Register for Saving the Second Joystick Data Packet Command
BUTTONS_CMND	    EQU		0X24	    ;Register for Saving the Buttons Data Packet Command
;*******Button and Joysick Save Data Variables*********************************************************
JOYSTICK_1_UD	    EQU		0X25	    ;Save High Byte of Left Joystick Up Down Data
JOYSTICK_1_LR	    EQU		0X26	    ;Save High Byte of Left Joystick Left Right Data
JOYSTICK_2_UD	    EQU		0X27	    ;Save High Byte of Right Lower Joystick Up Down Data
JOYSTICK_2_LR	    EQU		0X28	    ;Save High Byte of Right Lower Joystick Left Right Data
JOYSTICK_3_UD	    EQU		0X29	    ;Save High Byte of Right Upper Joystick Up Down Data
JOYSTICK_3_LR	    EQU		0X2A	    ;Save High Byte of Right Upper Joystick Left Right Data
BUTTON_DATA_1	    EQU		0X2B	    ;Save Button Conditions for first 8 Buttons
BUTTON_DATA_2	    EQU		0X2C	    ;Save Button Condition for the 9th Button
;*******Transmission Variables********************************************************
TX_HANDSHAKE	    EQU		0X2D	    ;Register to Store the Handshake Character: $ (0x24)
TX_ADDR		    EQU		0X2E	    ;Register to Store the Robot Address Used in Tranmission
TX_CMND		    EQU		0X2F	    ;Register to Store the Command Used in Transmission
TX_DATA_1	    EQU		0X30	    ;Register to Store the First Data Byte in Transmission Packet
TX_DATA_2	    EQU		0X31	    ;Register to Store the Second Data Byte in Transmission Packet
TX_DATA_3	    EQU		0X32	    ;Register to Store the Third Data Byte in Transmission Packet
TX_COUNT	    EQU		0X33	    ;Register to Track Which Data Packet Should be Sent
;********ADC Channel Variables***************************************************************************
ANCH0		    EQU		0X34	    ;JOY1 UD\     
ANCH1		    EQU		0X35	    ;JOY1 LR \	   
ANCH2		    EQU		0X36	    ;JOY2 UD  \	
ANCH3		    EQU		0X37	    ;JOY2 LR   | Variables Used to Change the Analog Channel Selection While Saving Result Formatting
ANCH4		    EQU		0X38	    ;JOY3 UD  /	
ANCH5		    EQU		0X39	    ;JOY3 LR /	
ANCH_COUNT	    EQU		0X3A	    ;Keep Track of Which Analog Channel is Active
		    
;******************************************		
;Start of Setup File Code
;******************************************
SETUP_CODE CODE
 
INITIALIZE
;*** GPR SETUP **********
 ;*All General Purpose Registers In Bank 0***************
 BANKSEL	TIMERCOUNT
 MOVLW		D'31'
 MOVWF		TIMERCOUNT	    ;Timer 2 Set to 0.5s Code.  (16.32mS * 31 = 0.505S)
 
 CLRF		ROBOT_ADDR	    ;Clear Robot Address on Start UP
 
 MOVLW		H'4A'
 MOVWF		JOY_1_CMND	    ;Command Byte to Indicate the First set of Joystick Data Bytes; ASCII: J
 
 MOVLW		H'6A'
 MOVWF		JOY_2_CMND	    ;Command Byte to Indicate the Second Set of Joystick Data Bytes; ASCII: j
 
 MOVLW		H'42'
 MOVWF		BUTTONS_CMND	    ;Command Byte to Indicate the Buttons Data Bytes; ASCII: B
 
 MOVLW		H'24'
 MOVWF		TX_HANDSHAKE	    ;Store the Handshake Byte of ASCII $
 
 CLRF		TX_COUNT	    ;Initialize with the First Data Packet to be sent (Joystick 1)
 
 CLRF		JOYSTICK_1_UD
 
 CLRF		JOYSTICK_1_LR
 
 CLRF		JOYSTICK_2_UD
 
 CLRF		JOYSTICK_2_LR
 
 CLRF		JOYSTICK_3_UD
 
 CLRF		JOYSTICK_3_LR
 
 CLRF		BUTTON_DATA_1
 
 CLRF		BUTTON_DATA_2		;Initialize With Alll Data Bytes as 0
 
 CLRF		ANCH_COUNT		;Start With Analog Channel 0 (Joystick 1 UD)
 
 MOVLW		H'81'
 MOVWF		ANCH0			;Set Channel 0 with 10 Bit Resolution and Enable ADC (ADCON0)
 
 MOVLW		H'85'
 MOVWF		ANCH1			;Set Channel 1 with 10 Bit Resolution and Enable ADC (ADCON0)
 
 MOVLW		H'89'
 MOVWF		ANCH2			;Set Channel 2 with 10 Bit Resolution and Enable ADC (ADCON0)
 
 MOVLW		H'8D'
 MOVWF		ANCH3			;Set Channel 3 with 10 Bit Resolution and Enable ADC (ADCON0)
 
 MOVLW		H'91'
 MOVWF		ANCH4			;Set Channel 4 with 10 Bit Resolution and Enable ADC (ADCON0)
 
 MOVLW		H'95'
 MOVWF		ANCH5			;Set Channel 5 with 10 Bit Resolution and Enable ADC (ADCON0)
 
;*** SFR SETUP **********
 
;***Oscillator Set up****
    BANKSEL	OSCCON
    MOVLW	H'FB'
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
    MOVLW	H'7A'			;Sets Prescaler and Postscaler to 1:16 (Wait to enable timer)
    MOVWF	T2CON
    BANKSEL	PR2		
    MOVLW	D'255'			;Set PR2 count 
    MOVWF	PR2	
    
;*** Alternate Pin Functions ****
    BANKSEL	APFCON1
    MOVLW	H'F1'
    MOVWF	APFCON1			;Set Comm on Port C
    
    BANKSEL	APFCON2
    MOVLW	H'00'
    MOVWF	APFCON2			;Set Save/Select on Port A CCP3 on Port E
    
;*** PORT A SETUP ****
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
    CLRF	LATA			;Clear Input Latches
    
    BANKSEL	TRISA
    MOVLW	H'FF'
    MOVWF	TRISA 			;SET Port A as Input
    
    BANKSEL	ANSELA
    MOVLW	H'2F'
    MOVWF	ANSELA			;Enable Analog Input on RA0(CH0), RA1(CH1), RA2(CH2), RA3(CH3), and RA5(CH4)
    
;*** PORT B SETUP **** 
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
    MOVWF	TRISB 			;SET Port B as Input
    
    BANKSEL	ANSELB
    CLRF	ANSELB			;Disable Analog Input
    
;*** PORT C SETUP **** 
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
    MOVLW	H'BF'
    MOVWF	TRISC 			;SET Port C as Input (TX:RC6 as Output)
    
    BANKSEL	ANSELC
    CLRF	ANSELC			;Disable Analog Input
    
    ;*** PORT D SETUP **** 
    BANKSEL	WPUD
    CLRF	WPUD			;Disable Weak Pull Ups
    
    BANKSEL	ODCOND
    CLRF	ODCOND			;Disable Open Drain
    
    BANKSEL	SLRCOND
    CLRF	SLRCOND			;Disable Slew Rate Limit
    
    BANKSEL	INLVLD
    MOVLW	H'FF'
    MOVWF	INLVLD			;Set Input Threshold Voltage to CMOS Levels
    
    BANKSEL	LATD
    CLRF	LATD			;Clear Input Latches
    
    BANKSEL	TRISD
    MOVLW	H'00'
    MOVWF	TRISD 			;SET Port D as Output
    
    BANKSEL	ANSELD
    CLRF	ANSELD			;Disable Analog Input
    
;*** PORT E SETUP ****
    BANKSEL	WPUE
    CLRF	WPUE			;Disable Weak Pull Ups
    
    BANKSEL	ODCONE
    CLRF	ODCONE			;Disable Open Drain
    
    BANKSEL	SLRCONE
    CLRF	SLRCONE			;Disable Slew Rate Limit
    
    BANKSEL	INLVLE
    MOVLW	H'FF'
    MOVWF	INLVLE			;Set Input Threshold Voltage to CMOS Levels
    
    BANKSEL	LATE
    CLRF	LATE			;Clear Input Latches
    
    BANKSEL	TRISE
    MOVLW	H'0F'
    MOVWF	TRISE 			;SET Port E 0,1,2  as Input
    
    BANKSEL	ANSELE
    MOVLW	H'01'
    MOVWF	ANSELE			;Enable Analog Input on RE0(CH5)
    
;*** USART Set-Up  :  *****
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
    
;*** ADC Set-Up  :   ******
    BANKSEL	ADCON0
    MOVLW	H'81'
    MOVWF	ADCON0			;10 Bit Result, Start With Channel 0, and Enable ADC(Wait On Go)
    
    BANKSEL	ADCON1
    MOVLW	H'50'
    MOVWF	ADCON1			;Set Signed Magnitude Result, Fosc/16 Conversion Clock, and Pos/Neg References to VDD/VSS
    
    BANKSEL	ADCON2
    MOVLW	H'0F'
    MOVWF	ADCON2			;Disable Auto Converion Trigger and Set Negative Refernce to Whats Set in ADCON1
    
;*** Interrupt Set-Up : ****
    BANKSEL	INTCON
    MOVLW	H'40'			;Enable Peripheral Interrupts (Wait on Global Enable)
    MOVWF	INTCON
    
    BANKSEL	PIE1
    MOVLW	H'42'
    MOVWF	PIE1			;Enable Timer 2 and ADC Interrupts
 
    BANKSEL	PIE2
    MOVLW	H'00'
    MOVWF	PIE2			;Disable Interrupts
    
    BANKSEL	PIE3
    BCF		PIE3, 4			;Disable CCP3 Interrupt
    
    BANKSEL	PIE4
    MOVLW	H'00'
    MOVWF	PIE4			;Disable Interrupts
    
    
    RETURN				;END OF SETUP ROUTINE