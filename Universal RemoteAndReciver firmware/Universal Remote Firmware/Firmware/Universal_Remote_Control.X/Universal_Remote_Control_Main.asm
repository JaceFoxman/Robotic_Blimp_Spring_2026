;****************************************************************************
;									    *
;	    File Name: Universal_Remote_Control_Main.asm		    *
;	    Date: 11/21/24						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State Univerisy; RCET			    *
;	    Description:Firmware For Universal Remote Controller	    *
;									    *
;****************************************************************************
;									    *
;	    Revision History:						    *
;     1.Added Timer 2 Interrupt Delay to Occur Every 0.5S; 11/21/24	    *
;     2.Added UART COM Functions include File; 11/22/24			    *
;     3.Added Button Check and Update Button Data; 11/24/24		    *
;     4.Added ADC Interrupt and Single Channel Read; 11/24/24		    *
;     5.Added ADC For all Other Channels; 11/24/24			    *
;									    *
;****************************************************************************
    
#INCLUDE <p16lf1789.inc>				; processor specific variable definitions
#INCLUDE <URC_PIC16LF1789_Set_Up.inc>			;Set Up Proccesor Functions
#INCLUDE <URC_UART.INC>					;Include File That Contains Transmission Functions
#INCLUDE <URC_ADC.INC>					;Include File That Contains ADC Channel Select Functions
;LIST	 P=16LF1789					;list directive to define processor
;errorlevel -302,-207,-305,-206,-203			;suppress "not in bank 0" message,  Found label after column 1,
    
;******************************************    
;Configuration
;******************************************
    
; CONFIG1
; __config 0xC9E4
 __CONFIG _CONFIG1, _FOSC_INTOSC & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_OFF & _CLKOUTEN_OFF & _IESO_OFF & _FCMEN_OFF
; CONFIG2
; __config 0xDDFF
 __CONFIG _CONFIG2, _WRT_OFF & _PLLEN_ON & _STVREN_OFF & _BORV_LO & _LPBOR_OFF & _LVP_OFF

;******************************************		
;Interrupt Vectors
;******************************************
    ORG H'00'					
    GOTO SETUP				    ;RESET CONDITION GOTO SETUP
    ORG H'04'
    GOTO INTERRUPT			    ;Interrupt occur GOTO INTERRUPT
    
;******************************************
;Setup Routine
;******************************************
SETUP
    CALL    INITIALIZE
    
;*****Not Currently Using USART Receive***************************
;    BANKSEL	RCSTA
;    BSF	RCSTA,4			    ;Enable Continuous Receive
    
    BANKSEL	TXSTA
    BSF		TXSTA,5			    ;Enable Transmit
	
    BANKSEL	T2CON
    BSF		T2CON,2			    ;Enable Timer 2

    BANKSEL	INTCON
    BSF		INTCON,7		    ;Enable Global Interrupts
    
    BANKSEL	ADCON0
    BSF		ADCON0,1		    ;Start Conversion
    
    CALL	SET_ROBOT_ADDR		    ;Load Dip Switch Value as Robot Address

    GOTO    MAIN
    
;******************************************
;Interrupt Service Routine
;******************************************
INTERRUPT
    BANKSEL	PIR1
    BTFSC	PIR1,1			    ;Test Timer 2 Interrupt Flag
    CALL	TMR2_INTERRUPT		    ;Occurs every ~16mS
    
    BANKSEL	PIR1
    BTFSC	PIR1,6			    ;Test ADC Interrupt Flag
    CALL	ADC_INTERRUPT		    ;ADC has Finish Conversion
    RETFIE
    
;******************************************
;Sub Routines
;******************************************
TMR2_INTERRUPT
;    BANKSEL	TIMERCOUNT
;    DECFSZ	TIMERCOUNT
;    GOTO	TMR2_INTERRUPT_END	    ;1/2s Has Not Yet Occured Exit Interrupt
;    
;    MOVLW	D'31'
;    MOVWF	TIMERCOUNT		    ;Reset Timer Count to 31 (16.32ms * 31 = 0.505S)
    ;*******This Code Should Run Every 0.5S************************************* 
    CALL	SET_ROBOT_ADDR		    ;Update Transmit Address Before TX
    CALL	DETERMINE_PACKET	    ;Determine Which Packet Should Be Sent and Load Corrisponding Data  
    CALL	SEND_DATA_PACKET	    ;Transmit Data Packet
TMR2_INTERRUPT_END
    BANKSEL	PIR1
    BCF		PIR1,1			    ;Clear Timer 2 Interrupt Flag
    RETURN
    
ADC_INTERRUPT ;Save ADC RESULT and Begin Converison on Next Channel
   BANKSEL	ADCON0
   BCF		ADCON0,0		    ;Disable ADC to Prepare for New Channel Input
   CALL		DETERMINE_ANCH		    ;Determine Current Channel, Save Channel Data, and Start Next Channel
ADC_INTERRUPT_END
    BANKSEL	ADCON0	    
    BSF		ADCON0,1		    ;Start Next Conversion
    BANKSEL	PIR1
    BCF		PIR1,6			    ;Clear ADC Interrupt Flag
    RETURN

    
SET_ROBOT_ADDR	;This Sets the Robot Address Register to The Value of Port B
    BANKSEL	PORTB
    MOVFW	PORTB
    BANKSEL	TX_ADDR
    MOVWF	TX_ADDR
    RETURN
    
UPDATE_BAR_DISPLAY ;Update the LED Bar Display on PORT D
    ;****Move Port B Value to LED Bar Display
    BANKSEL	PORTB
    MOVFW	PORTB
    BANKSEL	PORTD
    MOVWF	PORTD
    ;****************************************
    
;    ;****Move JOYSTICK_1_UD Value to LED Bar Display
;    BANKSEL	JOYSTICK_1_UD
;    MOVFW	JOYSTICK_1_UD
;    BANKSEL	PORTD
;    MOVWF	PORTD
;    ;****************************************
    
;    ;****Move JOYSTICK_1_LR Value to LED Bar Display
;    BANKSEL	JOYSTICK_1_LR
;    MOVFW	JOYSTICK_1_LR
;    BANKSEL	PORTD
;    MOVWF	PORTD
;    ;****************************************
    
;    ;****Move JOYSTICK_2_UD Value to LED Bar Display
;    BANKSEL	JOYSTICK_2_UD
;    MOVFW	JOYSTICK_2_UD
;    BANKSEL	PORTD
;    MOVWF	PORTD
;    ;****************************************
    
;    ;****Move JOYSTICK_2_LR Value to LED Bar Display
;    BANKSEL	JOYSTICK_2_LR
;    MOVFW	JOYSTICK_2_LR
;    BANKSEL	PORTD
;    MOVWF	PORTD
;    ;****************************************
    
;    ;****Move JOYSTICK_3_UD Value to LED Bar Display
;    BANKSEL	JOYSTICK_3_UD
;    MOVFW	JOYSTICK_3_UD
;    BANKSEL	PORTD
;    MOVWF	PORTD
;    ;****************************************
    
;    ;****Move JOYSTICK_3_LR Value to LED Bar Display
;    BANKSEL	JOYSTICK_3_LR
;    MOVFW	JOYSTICK_3_LR
;    BANKSEL	PORTD
;    MOVWF	PORTD
;    ;****************************************
    
    RETURN
;******************************************
;Main Code
;******************************************
MAIN  
    CALL	UPDATE_BAR_DISPLAY
    ;************************************************************************************
    ;****Check Buttons and Update Transmission Flags*************************************
    ;****NOTE:  All Port Registers and All Button Registers in the Same Bank[0]**********
    ;************************************************************************************
    BANKSEL	PORTA
    BTFSC	PORTA,4
    BSF		BUTTON_DATA_1,0		    ;Test Button and Set Button 1 Flag (Down Face Button)
    
    BANKSEL	PORTE
    BTFSC	PORTE,1
    BSF		BUTTON_DATA_1,1		    ;Test Button and Set Button 2 Flag (Right Face Button)
    
    BANKSEL	PORTE
    BTFSC	PORTE,2
    BSF		BUTTON_DATA_1,2		    ;Test Button and Set Button 3 Flag (Left Face Button)
    
    BANKSEL	PORTC
    BTFSC	PORTC,0
    BSF		BUTTON_DATA_1,3		    ;Test Button and Set Button 4 Flag (Up Face Button)
    
    BANKSEL	PORTA
    BTFSC	PORTA,6
    BSF		BUTTON_DATA_1,4		    ;Test Button and Set Left Bumper Flag
    
    BANKSEL	PORTA
    BTFSC	PORTA,7
    BSF		BUTTON_DATA_1,5		    ;Test Button and Set Right Bumper Flag
    
    BANKSEL	PORTC
    BTFSC	PORTC,1
    BSF		BUTTON_DATA_1,6		    ;Test Button and Set Left Joystick(1) Flag
    
    BANKSEL	PORTC
    BTFSC	PORTC,2
    BSF		BUTTON_DATA_1,7		    ;Test Button and Set Right Lower Joystick(2) Flag
    
    BANKSEL	PORTC
    BTFSC	PORTC,5
    BSF		BUTTON_DATA_2,0		    ;Test Button and Set Right Upper Joystick(3) Flag
    ;************************************************************************************
    ;**************************All Buttons Checked***************************************
    ;************************************************************************************ 
    GOTO	MAIN
    END				;END PROGRAM DIRECTIVE *******************************
;*************************************************************************************************************
    