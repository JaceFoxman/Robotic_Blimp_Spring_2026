;-----------------------------------------------------------------------------------------------------------------------------------------------
;PROJECT NAME:	SETUP FOR 1788
;DATE:		10 APRIL 2025
;VERSION:		1
;CREATOR:		JUSTIN BELL
;COMPANY:	IDAHO STATE UNIVERSITY
;DESCRIPTION:	SETUP INCLUDE FILE FOR THE 1788
;		ADDRESSES ALL PORTS AND OTHER REGISTERS
;-----------------------------------------------------------------------------------------------------------------------------------------------

#include <xc.inc> 
    
PSECT code

SETUP88:
 ;--------------------------------------------------------OSCILLATOR---------------------------------------------------------------------
    BANKSEL	OSCSTAT
    BTFSC	OSCSTAT,6	    ; WAIT FOR PLL TO BE UP AND RUNNING
    GOTO	$-1
    BANKSEL	OSCCON
    CLRF	OSCCON
    MOVLW	0X78		    ; SET OSCILLATOR FOR 16MHZ AND LOOK AT CONFIG BITS
    MOVWF	OSCCON
    BCF		OSCCON,7	    ; DISABLE 4X PLL FOR 16MHZ CLOCK
    BANKSEL	OSCSTAT
    BTFSC	OSCSTAT,5	    ; WAIT FOR OSSCILATOR TO BE READY
    GOTO	$-1
 
;--------------------------------------------------------PORT A------------------------------------------------------------------------------
    BANKSEL	ADCON1
    MOVLW	0X00
    MOVWF	ADCON1
    BANKSEL	ANSELA
    MOVLW	0X00
    MOVWF	ANSELA
    BANKSEL	TRISA
    MOVLW	0X00
    MOVWF	TRISA
    BANKSEL	IOCAP
    MOVLW	0X00
    MOVWF	IOCAP
    BANKSEL	IOCAN
    MOVLW	0X00
    MOVWF	IOCAN
    BANKSEL	INLVLA
    MOVLW	0X00
    MOVWF	INLVLA
    BANKSEL	SLRCONA
    MOVLW	0X00
    MOVWF	SLRCONA
    BANKSEL	PORTA
    MOVLW	0x00
    MOVWF	PORTA	;Set PORTA LOW -Jason
    
;--------------------------------------------------------PORT B------------------------------------------------------------------------------
    BANKSEL	TRISB
    MOVLW	0X00	; Change to all outputs -Jason
    MOVWF	TRISB
    BANKSEL	ANSELB
    MOVLW	0X00
    MOVWF	ANSELB
    BANKSEL	WPUB
    MOVLW	0X00
    MOVWF	WPUB
    BANKSEL	ODCONB
    MOVLW	0X00
    MOVWF	ODCONB
    BANKSEL	SLRCONB
    MOVLW	0X00
    MOVWF	SLRCONB
    BANKSEL	IOCBP
    MOVLW	0X00
    MOVWF	IOCBP
    BANKSEL	IOCBN
    MOVLW	0X00
    MOVWF	IOCBN
    BANKSEL	IOCBF
    MOVLW	0X00
    MOVWF	IOCBF
    BANKSEL	PORTB
    MOVLW	0x00	
    MOVWF	PORTB	;Set PORTB low -Jason
		
;--------------------------------------------------------PORT C------------------------------------------------------------------------------
    BANKSEL	LATC
    MOVLW	0X00
    MOVWF	LATC
    BANKSEL	TRISC
    MOVLW	0X18	;SDA and SCL as inputs
    MOVWF	TRISC
    BANKSEL	WPUC
    MOVLW	0X00
    MOVWF	WPUC
    BANKSEL	ODCONC
    MOVLW	0X18
    MOVWF	ODCONC
    BANKSEL	SLRCONC
    MOVLW	0X00
    MOVWF	SLRCONC
    BANKSEL	INLVLC
    CLRF	INLVLC
    BANKSEL	PORTC
    MOVLW	0x00	
    MOVWF	PORTC	;Set PORTC LOW -Jason
		
;--------------------------------------------------------PORT E------------------------------------------------------------------------------
    BANKSEL	IOCEP
    MOVLW	0X00
    MOVWF	IOCEP
    BANKSEL	IOCEN
    MOVLW	0X00
    MOVWF	IOCEN
    
;--------------------------------------------------------OPTION REGISTER---------------------------------------------------------------
    BANKSEL	OPTION_REG
    MOVLW	0X00
    MOVWF	OPTION_REG
    
;--------------------------------------------------------INTERRUPT CONTROL-----------------------------------------------------------
    BANKSEL	INTCON
    MOVLW	0XC0	;Global and peripheral interrupts Enabled
    MOVWF	INTCON
    
;--------------------------------------------------------PERIPHERAL ONE-----------------------------------------------------------------
    BANKSEL	PIE1
    MOVLW	0X0B	;Enable SSP1IF, TMR2, and TMR1 Interrupt -Jason
    MOVWF	PIE1
    
;--------------------------------------------------------PERIPHERAL TWO----------------------------------------------------------------
    BANKSEL	PIE2
    MOVLW	0X00
    MOVWF	PIE2
    
;--------------------------------------------------------CCP1 CONTROL--------------------------------------------------------------------
;    BANKSEL CCP1CON
;    MOVLW	B'00000000'
;    MOVWF	CCP1CON
;    BANKSEL CCP2CON
;    MOVLW	B'00000000'
;    MOVWF	CCP2CON
;    BANKSEL CM2CON1
;    MOVLW	B'00000000'
;    MOVWF	CM2CON1
;--------------------------------------------------------TIMER ONE-----------------------------------------------------------------------
    BANKSEL	T1CON
    MOVLW	0x30
    MOVWF	T1CON			;Use instruction clock (Fosc/4); 1:8 prescaler (wait on enable)
    
    BANKSEL	T1GCON
    CLRF	T1GCON			;Gate always enabled
;--------------------------------------------------------TIMER TWO-----------------------------------------------------------------------
    BANKSEL PIR1
    MOVLW   0x00	;move (0000|0000) to w
    MOVWF   PIR1	;Set TMR2 flag to 0 at set up
    
    BANKSEL TMR2
    MOVLW   0x00	;move (0000|0000) to w
    MOVWF   TMR2	;Start TMR2 at 0x00 at startup
    
    BANKSEL T2CON
    MOVLW   0x00	;move (0000|0000) to w
    MOVWF   T2CON	;Keep TMR2 off until ready
    
    BANKSEL PR2
    MOVLW   0x64	;move (0110|0100) to w
    MOVWF   PR2		;Set count so that TMR2 compares itself decimal 100, 0x64
    
    BANKSEL T2CON
    MOVLW   0x05	;move (0000|0101) to w
    MOVWF   T2CON	;Set Prescaler to 1:4, Postscaler to 1:1, Enable TMR2
    
;--------------------------------------------------------EUART------------------------------------------------------------------------------
;    BANKSEL BAUD1CON
;    MOVLW	B'00000000'
;    MOVWF	BAUD1CON
;    BANKSEL SP1BRGL
;    MOVLW	B'00001111'
;    MOVWF	SP1BRGL
;    BANKSEL SPBRGH
;    CLRF SPBRGH
;    BANKSEL TXSTA
;    CLRF TXSTA
;    BANKSEL RCREG
;    CLRF RCREG
;    BANKSEL RCSTA
;    CLRF RCSTA
;    MOVLW	B'10110000'
;    MOVWF RCSTA
;    BANKSEL PIE1
;    BSF PIE1,RCIE	    ; ENABLE RECIEVE INTERRUPT FOR UART
    RETURN