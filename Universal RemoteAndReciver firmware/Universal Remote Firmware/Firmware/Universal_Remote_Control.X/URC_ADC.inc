;****************************************************************************
;									    *
;	    File Name: URC_ADC.inc					    *
;	    Date: 11/24/24						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State Univerisy; RCET			    *
;	    Description:  ADC Channel Detection and Increment Functions     *
;			    Functions Only Called In ADC Interrupt	    *
;									    *
;****************************************************************************
    
    
ADC_CODE CODE
 
DETERMINE_ANCH	;*************Determine the Current Channel Based On ANCH_COUNT.  Saves Data and Sets Up Next Channel*************************
    BANKSEL	ANCH_COUNT
    MOVLW	H'00'
    SUBWF	ANCH_COUNT,0		;Subtract 0 from ANCH_COUNT Store in W
    BANKSEL	STATUS
    BTFSC	STATUS,2		;Skip if ANCH_COUNT is not 0
    GOTO	IS_ANCH0
    BANKSEL	ANCH_COUNT
    MOVLW	H'01'
    SUBWF	ANCH_COUNT,0		;Subtract 1 from ANCH_COUNT Store in W
    BANKSEL	STATUS
    BTFSC	STATUS,2		;Skip if ANCH_COUNT is not 1
    GOTO	IS_ANCH1
    BANKSEL	ANCH_COUNT
    MOVLW	H'02'
    SUBWF	ANCH_COUNT,0		;Subtract 2 from ANCH_COUNT Store in W
    BANKSEL	STATUS
    BTFSC	STATUS,2		;Skip if ANCH_COUNT is not 2
    GOTO	IS_ANCH2
    BANKSEL	ANCH_COUNT
    MOVLW	H'03'
    SUBWF	ANCH_COUNT,0		;Subtract 3 from ANCH_COUNT Store in W
    BANKSEL	STATUS
    BTFSC	STATUS,2		;Skip if ANCH_COUNT is not 3
    GOTO	IS_ANCH3
    BANKSEL	ANCH_COUNT
    MOVLW	H'04'
    SUBWF	ANCH_COUNT,0		;Subtract 4 from ANCH_COUNT Store in W
    BANKSEL	STATUS
    BTFSC	STATUS,2		;Skip if ANCH_COUNT is not 4
    GOTO	IS_ANCH4
    BANKSEL	ANCH_COUNT
    MOVLW	H'05'
    SUBWF	ANCH_COUNT,0		;Subtract 5 from ANCH_COUNT Store in W
    BANKSEL	STATUS
    BTFSC	STATUS,2		;Skip if ANCH_COUNT is not 5
    GOTO	IS_ANCH5
    BANKSEL	ANCH_COUNT
    CLRF	ANCH_COUNT		;Channel Unknown (Not 0 - 5) Clear Flag to Reset to CH0
    BANKSEL	ANCH0
    MOVFW	ANCH0
    BANKSEL	ADCON0
    MOVWF	ADCON0			;Select Analog Channel 0  Next and Enable ADC
    GOTO	DETERMINE_ANCH_END	;ANCH_COUNT IS Not 0-5 Clear Count Set Up Channel 0 and Exit
IS_ANCH0
    INCF	ANCH_COUNT		;Increment Channel Count For CH1 Nextt
    BANKSEL	ANCH1
    MOVFW	ANCH1
    BANKSEL	ADCON0
    MOVWF	ADCON0			;Select Analog Channel 1  Next and Enable ADC
    BANKSEL	ADRESH
    MOVFW	ADRESH
    BANKSEL	JOYSTICK_1_UD
    MOVWF	JOYSTICK_1_UD		;Save ADC Result Upper 8 Bits as Joystick 1 Up Down Data
    GOTO	DETERMINE_ANCH_END	;Exit Routine
IS_ANCH1
    INCF	ANCH_COUNT		;Increment Channel Count For CH2 Nextt
    BANKSEL	ANCH2
    MOVFW	ANCH2
    BANKSEL	ADCON0
    MOVWF	ADCON0			;Select Analog Channel 2  Next and Enable ADC
    BANKSEL	ADRESH
    MOVFW	ADRESH
    BANKSEL	JOYSTICK_1_LR
    MOVWF	JOYSTICK_1_LR		;Save ADC Result Upper 8 Bits as Joystick 1 Left Right Data
    GOTO	DETERMINE_ANCH_END	;Exit Routine
IS_ANCH2
    INCF	ANCH_COUNT		;Increment Channel Count For CH3 Nextt
    BANKSEL	ANCH3
    MOVFW	ANCH3
    BANKSEL	ADCON0
    MOVWF	ADCON0			;Select Analog Channel 3  Next and Enable ADC
    BANKSEL	ADRESH
    MOVFW	ADRESH
    BANKSEL	JOYSTICK_2_UD
    MOVWF	JOYSTICK_2_UD		;Save ADC Result Upper 8 Bits as Joystick 2 Up Down Data
    GOTO	DETERMINE_ANCH_END	;Exit Routine
IS_ANCH3
    INCF	ANCH_COUNT		;Increment Channel Count For CH4 Nextt
    BANKSEL	ANCH4
    MOVFW	ANCH4
    BANKSEL	ADCON0
    MOVWF	ADCON0			;Select Analog Channel 4  Next and Enable ADC
    BANKSEL	ADRESH
    MOVFW	ADRESH
    BANKSEL	JOYSTICK_2_LR
    MOVWF	JOYSTICK_2_LR		;Save ADC Result Upper 8 Bits as Joystick 2 Left Right Data
    GOTO	DETERMINE_ANCH_END	;Exit Routine
IS_ANCH4
    INCF	ANCH_COUNT		;Increment Channel Count For CH5 Nextt
    BANKSEL	ANCH5
    MOVFW	ANCH5
    BANKSEL	ADCON0
    MOVWF	ADCON0			;Select Analog Channel 5  Next and Enable ADC
    BANKSEL	ADRESH
    MOVFW	ADRESH
    BANKSEL	JOYSTICK_3_UD
    MOVWF	JOYSTICK_3_UD		;Save ADC Result Upper 8 Bits as Joystick 3 Up Down Data
    GOTO	DETERMINE_ANCH_END	;Exit Routine
IS_ANCH5
    CLRF	ANCH_COUNT		;Clear Channel Count For CH0 Nextt
    BANKSEL	ANCH0
    MOVFW	ANCH0
    BANKSEL	ADCON0
    MOVWF	ADCON0			;Select Analog Channel 0  Next and Enable ADC
    BANKSEL	ADRESH
    MOVFW	ADRESH
    BANKSEL	JOYSTICK_3_LR
    MOVWF	JOYSTICK_3_LR		;Save ADC Result Upper 8 Bits as Joystick 3 Left Right Data
DETERMINE_ANCH_END
    RETURN
