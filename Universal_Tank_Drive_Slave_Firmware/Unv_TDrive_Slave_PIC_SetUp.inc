;****************************************************************************
;									    *
;	    File Name: Unv_TDrive_Slave_PIC_SetUp.INC			    *
;	    Date:   1/28/25						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State University; RCET			    *
;	    Description:  Firmware Setup for PIC16f1788	to be used as	    *
;			    Tank Drive Slave with Universal Control System. *
;									    *
;****************************************************************************
;									    *
;	    Revision History:						    *
;	1.Added register for saving controller status; 1/28/25		    *
;	2.Added MSSP interrupts for I2C COM; 1/30/25			    *
;	3.Added register for motor output signals; 2/11/25		    *
;	4.Added Timer 2 functionality and interrupts; 2/11/25		    *
;	5.Added Timer 1 functionality and interrupts; 2/12/25		    *
;	6.Added Current Drive State Registser; 2/13/25			    *
;	7.Added Joystick Upper and Lower Thresholds; 2/18/25		    *
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
I2C_TIMEOUT_COUNT	EQU	0X20	    ;Register to Slow I2C COM Timeout
			
JOY1_UD			EQU	0X22	    ;Register to Save Joystick 1 Up-Down Data
JOY1_LR			EQU	0X23	    ;Register to Save Joystick 1 Left-Right Data
JOY2_UD			EQU	0X24	    ;Register to Save Joystick 2 Up-Down Data
JOY2_LR			EQU	0X25	    ;Register to Save Joystick 2 Left-Right Data
T20MS_COUNT_2		EQU	0X26	    ;Register to Track Output Period (20mS)
T20MS_COUNT_1		EQU	0x27	    ;Register to Track Output Period (20mS)	    
R_MOTOR_COUNT		EQU	0X28	    ;Register to Track Current Right Motor Output On-Time(this one is decremented)
L_MOTOR_COUNT		EQU	0X29	    ;Register to Track Current Left Motor Output On-Time(this one is decremented)
R_MOTOR_STATUS		EQU	0X2A	    ;Register to Track Right Motor Mode (This status is used to reset R_MOTOR_COUNT)
L_MOTOR_STATUS		EQU	0X2B	    ;Register to Track Left Motor Mode (This status is used to reset L_MOTOR_COUNT)
FORWARD_COUNT		EQU	0X2C	    ;Register to Save 2.5mS On-Time Count
STOP_COUNT		EQU	0x2D	    ;Register to Save 1.5mS On-Time Count
REVERSE_COUNT		EQU	0x2E	    ;Register to Save 0.5mS On-Time Count
CURRENT_DRIVE_STATE	EQU	0X2F	    ;Register to Motor Output State Bits: 0=Stop, 1=Forward, 2=Reverse, 3=Turn Right, 4=Turn Left
UPPER_THRESHOLD		EQU	0X30	    ;Register used to compare Joystick Value to Determine Forward/Left
LOWER_THRESHOLD		EQU	0X31	    ;Register used to compare Joystick Value to Determine Back/Right
ACTUATOR_MOTOR_COUNT	EQU	0x32	    ;Register to Track Current Actuator Motor Output On-Time(this one is decremented)
ACTUATOR_MOTOR_STATUS	EQU	0x33	    ;Register to Track Actuator Motor Mode (This status is used to reset ACTUATOR_MOTOR_COUNT)
;******************************************		
;Start of Setup File Code
;******************************************
SETUP_CODE CODE
 
INITIALIZE
;*** SFR SETUP **********
    BANKSEL	I2C_TIMEOUT_COUNT
    MOVLW	H'02'
    MOVWF	I2C_TIMEOUT_COUNT
    
    BANKSEL	JOY1_UD
    MOVLW	D'128'
    MOVWF	JOY1_UD
    BANKSEL	JOY1_LR
    MOVLW	D'128'
    MOVWF	JOY1_LR
    BANKSEL	JOY2_UD
    MOVLW	D'128'
    MOVWF	JOY2_UD
    BANKSEL	JOY2_LR
    MOVLW	D'128'
    MOVWF	JOY2_LR			;Begin With all Joysticks at "Center" 
    
    BANKSEL	T20MS_COUNT_2
    MOVLW	D'200'			;200 Measured: 20.78mS
    MOVWF	T20MS_COUNT_2		;Save Count for 20mS (200*0.05mS=10mS)
    BANKSEL	T20MS_COUNT_1
    MOVLW	D'200'			;200 Measured: 20.78mS
    MOVWF	T20MS_COUNT_1		;Save Count for 20mS (200*0.05mS=10mS)
    BANKSEL	FORWARD_COUNT		
    MOVLW	D'40'			;25 Measured: 2.578mS
    MOVWF	FORWARD_COUNT		;Save Count for 2.5mS(25*0.05mS=2.5mS)
    BANKSEL	STOP_COUNT
    MOVLW	D'29'			;14 Measured: 1.458mS; 15 Measured: 1.538mS
    MOVWF	STOP_COUNT		;Save Count for 1.5mS(15*0.05mS=1.5mS)
    BANKSEL	REVERSE_COUNT
    MOVLW	D'20'			;5 Measured: 0.498mS
    MOVWF	REVERSE_COUNT		;Save Count for 0.5mS(5*0.05mS=0.5mS)
    BANKSEL	STOP_COUNT
    MOVFW	STOP_COUNT
    BANKSEL	ACTUATOR_MOTOR_STATUS
    MOVWF	ACTUATOR_MOTOR_STATUS
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS
    BANKSEL	ACTUATOR_MOTOR_COUNT
    MOVWF	ACTUATOR_MOTOR_COUNT
    BANKSEL	R_MOTOR_COUNT
    MOVWF	R_MOTOR_COUNT
    BANKSEL	L_MOTOR_COUNT
    MOVWF	L_MOTOR_COUNT		;Begin with Both Left, Right, and Actuator Motors in Stop (1.5mS)
    
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'00'
    MOVWF	CURRENT_DRIVE_STATE	;Begin in STOP mode
    
    BANKSEL	UPPER_THRESHOLD
    MOVLW	H'C8'
    MOVWF	UPPER_THRESHOLD
    BANKSEL	LOWER_THRESHOLD
    MOVLW	H'28'
    MOVWF	LOWER_THRESHOLD		;Set Upper and lower thresholds to determine joystick Status
    
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
    MOVLW	H'30'
    MOVWF	T1CON			;Use instruction clock (Fosc/4); 1:8 prescaler (wait on enable)
    
    BANKSEL	T1GCON
    CLRF	T1GCON			;Gate always enabled
    
;*** TIMER 2 SETUP *****
    BANKSEL	T2CON
    MOVLW	H'09'			;Sets Prescaler to 1:4; Post Scaler to 1:2(wait on enable)
    MOVWF	T2CON
    BANKSEL	PR2		
    MOVLW	D'25'			;Set PR2 count to 25 interrupt ever 0.05mS
    MOVWF	PR2			;(((16M/4)^-1 * 4) * 25) * 2 = 0.05mS
    
;*** Alternate Pin Functions ****
    BANKSEL	APFCON1
    MOVLW	H'41'
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
    CLRF	LATA			;Clear Input Latches
    
    BANKSEL	TRISA
    MOVLW	H'00'
    MOVWF	TRISA 			;SET Port A as Output
    
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
    MOVLW	H'00'
    MOVWF	TRISB 			;SET Port B as Output
    
    
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
    MOVLW	H'00'
    MOVWF	TRISC 			;SET Port C as Output
    
;*** PORT E SETUP ****
;28 Pin Package Does Not Have Port E
    
;*** Interrupt Set-Up : ****
    BANKSEL	INTCON
    MOVLW	H'40'			;Enable Peripheral Interrupts (Wait on Global)
    MOVWF	INTCON
    
    BANKSEL	PIE1
    MOVLW	H'0B'
    MOVWF	PIE1			;Enable MSSP, Timer 2, and Timer 1 Interrupts
 
    BANKSEL	PIE2
    MOVLW	H'00'
    MOVWF	PIE2			;Disable Interrupts
    
    BANKSEL	PIE3
    BCF		PIE3, 4			;Disable CCP3 Interrupt
    
    BANKSEL	PIE4
    MOVLW	H'00'
    MOVWF	PIE4			;Disable Interrupts
    
    RETURN				;END OF SETUP ROUTINE