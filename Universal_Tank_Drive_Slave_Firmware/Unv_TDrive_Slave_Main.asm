;****************************************************************************
;									    *
;	    File Name:	Unv_TDrive_Slave_Main.asm			    *
;	    Date: 1/28/25						    *
;	    File Version: 1						    *
;	    Author:    Zac Christensen					    *
;	    Company:   Idaho State University; RCET			    *
;	    Description: Slave Board for the Universal Control System	    *
;			    "Tank Drive" Board.				    *
;									    *
;****************************************************************************
;									    *
;	    Revision History:						    *
;     1.Added I2C Receive Interrupt Routine; 1/29/25			    *
;     2.Added "working" I2C Receive Routine; 2/10/25			    *
;     3.Added Timer 2 Interrupt Routines; 2/11/25			    *
;     4.Moved interrupt flag clearing into routines; 2/12/25		    *
;     5.Added Timer 1 Interrupts for I2C COM Timeout; 2/12/25		    *
;     6.Added COM Timeout Functionality;  2/13/25			    *
;     7.Added Set Motor State Routine; 2/13/25				    *
;     8.Added Determine Motor State and Joystick Thresholds; 2/18/25	    *
;     9.Slow I2C COM Timeout For More Peripherals; 3/3/25		    *
;     10.Add Comments and Update register names for Template; 4/30/25	    *
;     _.								    *
;									    *
;****************************************************************************
    
;****************************************************************************************
;****			     CURRENT_DRIVE_STATE Bit Map			      ***
;****************************************************************************************
;****  Current Drive State is Set up so that the lower Nibble(Bits 0-3) indicate the  ***
;***    right motor Function and the upper nibble(Bits 4-7) indicate the left motor   ***
;***    function.  Looking at the nibbles individually, a decimal 0=Stop, 1=Forward,  ***
;***	and 2=Reverse.								      ***
;****************************************************************************************
;***_________________________________________________________________________________ ***
;***| Hex Value	 |	Mode Name         |	       Motor Function		    | ***
;***|	 0x00	 |      All Stop          | Right Motor Stop, Left Motor Stop	    | ***
;***|	 0x11	 |    Drive Forward       | Right Motor Forward, Left Motor Forward | ***
;***|	 0x22	 |    Drive Reverse       | Right Motor Reverse, Left Motor Reverse | ***       
;***|	 0x10	 |  Turn Right Forward    | Right Motor Stop, Left Motor Forward    | ***    
;***|	 0x02	 |  Turn Right Reverse    | Right Motor Reverse, Left Motor Stop    | ***
;***|	 0x12	 | Zero-Point Turn Right  | Right Motor Reverse, Left Motor Forward | ***    
;***|	 0x01	 |  Turn Left Forward     | Right Motor Forward, Left Motor Stop    | ***    
;***|	 0x20	 |  Turn Left Reverse     | Right Motor Stop, Left Motor Reverse    | ***
;***|	 0x21	 | Zero-Point Turn Left   | Right Motor Forward, Left Motor Reverse | ***
;***|	 0x__	 | Unrecognized; All Stop | Right Motor Stop, Left Motor Stop	    | ***
;***_________________________________________________________________________________ ***
;****************************************************************************************
    
#INCLUDE <p16f1788.inc>					;Processor specific variable definitions
#INCLUDE <Unv_TDrive_Slave_PIC_SetUp.inc>		;URC TDrive Slave Board PIC Set Up
#INCLUDE <Unv_Slave_I2C_SetUp.inc>			;I2C Set Up and Read/Write Routines
LIST	 P=16f1788					;list directive to define processor
errorlevel -302,-207,-305,-206,-203			;suppress "not in bank 0" message,  Found label after column 1,
	
;******************************************    
;Configuration
;******************************************
    
; CONFIG1
; __config 0xC9E4
 __CONFIG _CONFIG1, _FOSC_INTOSC & _WDTE_OFF & _PWRTE_OFF & _MCLRE_ON & _CP_OFF & _CPD_OFF & _BOREN_OFF & _CLKOUTEN_OFF & _IESO_OFF & _FCMEN_OFF
; CONFIG2
; __config 0xDFFF
 __CONFIG _CONFIG2, _WRT_OFF & _VCAPEN_OFF & _PLLEN_OFF & _STVREN_ON & _BORV_LO & _LPBOR_OFF & _LVP_OFF

;******************************************		
;Interrupt Vectors
;******************************************
    ORG H'00'					
    GOTO SETUP				;RESET CONDITION GOTO SETUP
    ORG H'04'
    GOTO INTERRUPT			;Interrupt occur GOTO INTERRUPT
    
;******************************************
;Setup Routine
;******************************************
SETUP
    CALL	INITIALIZE
    CALL	I2C_SETUP_SLAVE
    
    BANKSEL	INTCON
    BSF		INTCON,7	    ;Enable Global Interrupts
    
    BANKSEL	T1CON
    BSF		T1CON,0		    ;Enable Timer 1 (I2C COM Timeout)
    
    BANKSEL	T2CON
    BSF		T2CON,2		    ;Enable Timer 2 (Interrupt Every 0.05mS)
    
    GOTO	MAIN
    
;******************************************
;Interrupt Service Routine
;******************************************
INTERRUPT    
	;Handles I2C receive and temporary data save functions
	BANKSEL	    PIR1
	BTFSC	    PIR1,3
	CALL	    I2C_RECEIVE	    ;Test for MSSP Interrupt
	
	;Handles loss of I2C connection
	BANKSEL	    PIR1
	BTFSC	    PIR1,0
	CALL	    I2C_COM_TIMEOUT ;Test for Timer 1 Interrupt
	
	;Handles Servo-Type signal output Timing
	BANKSEL	    PIR1
	BTFSC	    PIR1,1
	CALL	    TMR2_INTERRUPT  ;Test for Timer 2 Interrupt
	
    RETFIE
    
;******************************************
;Sub Routines
;******************************************
    
;*****************************************************************************************************
;*** TMR2_INTERRUPT Hanldles the Servo Signal Outputs.  Every 20mS All the outputs are set	   ***
;*** and their corresponding counters are reset to their on-time "status" count.		   ***
;*** When 20mS has not passed, each output counter is decremented.  If the count reaches 0	   ***
;*** the output is cleared.  Outputs 1-8 are on Port A; Output 9 is PortC,0; Output 10 is PortC,5. ***
;*** Right Motor is Output 1.  Left Motor is Output 2. Acutator Motor is Output 3.		   ***
;*****************************************************************************************************
TMR2_INTERRUPT
    BANKSEL	T20MS_COUNT_1
    MOVLW	H'00'
    SUBWF	T20MS_COUNT_1,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	FIRST_COUNT_ZERO
    DECF	T20MS_COUNT_1
    GOTO	T20MS_NOT_PASSED
FIRST_COUNT_ZERO
    BANKSEL	T20MS_COUNT_2
    DECFSZ	T20MS_COUNT_2
    GOTO	T20MS_NOT_PASSED
    MOVLW	D'200'
    MOVWF	T20MS_COUNT_2		;Reset 20mS Counter
    BANKSEL	T20MS_COUNT_1
    MOVWF	T20MS_COUNT_1
    BANKSEL	R_MOTOR_STATUS
    MOVFW	R_MOTOR_STATUS
    BANKSEL	R_MOTOR_COUNT
    MOVWF	R_MOTOR_COUNT		;Reset Right Motor Count
    BANKSEL	L_MOTOR_STATUS
    MOVFW	L_MOTOR_STATUS
    BANKSEL	L_MOTOR_COUNT
    MOVWF	L_MOTOR_COUNT		;Reset Left Motor Count
    BANKSEL	ACTUATOR_MOTOR_STATUS
    MOVFW	ACTUATOR_MOTOR_STATUS
    BANKSEL	ACTUATOR_MOTOR_COUNT
    MOVWF	ACTUATOR_MOTOR_COUNT    ;Reset Actuator Motor Count
    BANKSEL	PORTA
    BSF		PORTA,0
    BSF		PORTA,1		    
    BSF		PORTA,2			;Set Motor Outputs High(Right=Bit0; Left=Bit1; Head=Bit2)
    GOTO	TMR2_INTERRUPT_END
T20MS_NOT_PASSED
    BANKSEL	R_MOTOR_COUNT
    DECFSZ	R_MOTOR_COUNT
    GOTO	CHECK_L_MOTOR		;Right Motor On Time Not Reached Check Left
    BANKSEL	PORTA
    BCF		PORTA,0			;Right Motor On Time Reached Clear Output
CHECK_L_MOTOR
    BANKSEL	L_MOTOR_COUNT
    DECFSZ	L_MOTOR_COUNT	    
    GOTO	CHECK_ACTUATOR_MOTOR    ;Left Motor On Time Not Reached Check Head
    BANKSEL	PORTA
    BCF		PORTA,1			;Left Motor On time Reached Clear OutPut
CHECK_ACTUATOR_MOTOR
    BANKSEL	ACTUATOR_MOTOR_COUNT
    DECFSZ	ACTUATOR_MOTOR_COUNT	    
    GOTO	TMR2_INTERRUPT_END	;Left Motor On Time Not Reached Check Actuator
    BANKSEL	PORTA
    BCF		PORTA,2			;Actuator Motor On time Reached Clear OutPut
TMR2_INTERRUPT_END  
    BANKSEL	PIR1
    BCF		PIR1,1			;Clear Timer 2 Interrupt Flag
    RETURN
    
;**********************************************************************************************
;*** I2C_COM_TIMEOUT occurs during a timer 1 overflow. Every time a full I2C data packet is ***
;***  received timer 1 counters are cleared. This Function will occur if no I2C data is     ***
;***  received.  This Should be used to return outputs to safe state if I2C COM is lost.    ***
;***  This template sets all motors into a Stop.					    ***
;********************************************************************************************** 
I2C_COM_TIMEOUT
    ;******Occurs every 130mS*******
    BANKSEL	I2C_TIMEOUT_COUNT
    DECFSZ	I2C_TIMEOUT_COUNT		    ;Slow I2C COM Timeout Count 
    GOTO	I2C_COM_TIMEOUT_END
    MOVLW	H'02'
    MOVWF	I2C_TIMEOUT_COUNT		    ;Reset I2C COM Timeout Count
    BANKSEL	STOP_COUNT
    MOVFW	STOP_COUNT
    BANKSEL	ACTUATOR_MOTOR_STATUS
    MOVWF	ACTUATOR_MOTOR_STATUS
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS			    ;Timeout Occured.  Stop Motors
I2C_COM_TIMEOUT_END
    BANKSEL	PIR1
    BCF		PIR1,0				    ;Clear Timer 1 Interrupt Flag	
    RETURN
    
;****************************************************************************************************
;*** UPDATE_DRIVE_STATUS will run every time a full 4 byte I2C Packet is received.  This function ***
;*** will save the I2C Data out of temorary registsers, check joysticks against thresholdS, and	  ***
;*** update output signals accordingly.								  ***
;****************************************************************************************************    
UPDATE_DRIVE_STATUS
    CALL	SAVE_I2C_DATA		    ;Save temporary RX Data into registers
    CALL	DETERMINE_DRIVE_STATE	    ;Check Joystick Data and Determine Motor Function
    CALL	SET_DRIVE_STATE		    ;Check CURRENT_DRIVE_STATE and update motor output counts
    CALL	DETERMINE_ACTUATOR_STATE    ;Check Joystick Data and Set Actuator Motor Signal Counts
    BANKSEL	I2C_RX_COMPLETE
    CLRF	I2C_RX_COMPLETE		    ;Clear I2C data packet complete flag
    RETURN
   
;*************************************************************************************************
;*** SAVE_I2C_DATA is needed to convert the temporary I2C save registers into general          ***
;*** purpose registers.  This should correlate with the data sent from the master.             ***
;*** This Template assumes motors will be controlled from joystick 1 and 2.		       ***
;*************************************************************************************************
SAVE_I2C_DATA
    BANKSEL	I2C_RX_TEMP_1
    MOVFW	I2C_RX_TEMP_1
    BANKSEL	JOY1_UD
    MOVWF	JOY1_UD
    BANKSEL	I2C_RX_TEMP_2
    MOVFW	I2C_RX_TEMP_2
    BANKSEL	JOY1_LR	
    MOVWF	JOY1_LR
    BANKSEL	I2C_RX_TEMP_3
    MOVFW	I2C_RX_TEMP_3
    BANKSEL	JOY2_UD
    MOVWF	JOY2_UD
    BANKSEL	I2C_RX_TEMP_4
    MOVFW	I2C_RX_TEMP_4
    BANKSEL	JOY2_LR
    MOVWF	JOY2_LR
    RETURN
   
;******************************************************************************************************
;*** DETERMINE_DRIVE_STATE will run every time a full 4 byte I2C Packet is received.  This function ***
;***  Will compare joysticks to threshholds and set the register CURRENT_DRIVE_STATE to indicate    ***
;***   the desired drive mode.  Joystick 2 is compared for Right Motor.  Joystick 1 is compared for ***
;***   Left Motor.  See Comments above for Current Drive State Bit Map				    ***
;****************************************************************************************************** 
DETERMINE_DRIVE_STATE
    BANKSEL	CURRENT_DRIVE_STATE
    CLRF	CURRENT_DRIVE_STATE	    ;Clear any Previous State (put into stop mode)
;Check the joystick 2 for thresholds above or below.  if niether stop motor    
CHECK_RIGHT_MOTOR   
    BANKSEL	UPPER_THRESHOLD
    MOVFW	UPPER_THRESHOLD
    BANKSEL	JOY2_UD
    SUBWF	JOY2_UD,0		;Subtract Upper Threshold from Joystick Value (Stored in W)
    BANKSEL	STATUS			;JOY2_UD - UPPER_THRESHOLD
    BTFSC	STATUS,0
    GOTO	RIGHT_MOTOR_FORWARD	;Result is Positive Joystick is above Threshold
    BANKSEL	LOWER_THRESHOLD
    MOVFW	LOWER_THRESHOLD
    BANKSEL	JOY2_UD
    SUBWF	JOY2_UD,0		;Subtract Lower Threshold from Joystick Value (Stored in W)
    BANKSEL	STATUS			;JOY2_UD - LOWER_THRESHOLD
    BTFSS	STATUS,0
    GOTO	RIGHT_MOTOR_REVERSE	;Result is Negative Joystick is below Threshold
    GOTO	RIGHT_MOTOR_STOP	;Joystick is Neither above or below thresholds
;Check the joystick 1 for thresholds above or below.  if niether stop motor    
CHECK_LEFT_MOTOR    
    BANKSEL	UPPER_THRESHOLD
    MOVFW	UPPER_THRESHOLD
    BANKSEL	JOY1_UD
    SUBWF	JOY1_UD,0		;Subtract Upper Threshold from Joystick Value (Stored in W)
    BANKSEL	STATUS			;JOY1_UD - UPPER_THRESHOLD
    BTFSC	STATUS,0
    GOTO	LEFT_MOTOR_FORWARD	;Result is Positive Joystick is above Threshold
    BANKSEL	LOWER_THRESHOLD
    MOVFW	LOWER_THRESHOLD
    BANKSEL	JOY1_UD
    SUBWF	JOY1_UD,0		;Subtract Lower Threshold from Joystick Value (Stored in W)
    BANKSEL	STATUS			;JOY1_UD - LOWER_THRESHOLD
    BTFSS	STATUS,0
    GOTO	LEFT_MOTOR_REVERSE	;Result is Negative Joystick is below Threshold
    GOTO	LEFT_MOTOR_STOP		;Joystick is Neither above or below thresholds    
;Set Different States for Right Motor Before Checking Joystick 1 Threshold (CURRENT_DRIVE_STATE, Bits 0-3)
RIGHT_MOTOR_FORWARD
    BANKSEL	CURRENT_DRIVE_STATE
    BSF		CURRENT_DRIVE_STATE,0	    ;Set Drive State Right Motor Forward (0x_1)
    GOTO	CHECK_LEFT_MOTOR
RIGHT_MOTOR_REVERSE
    BANKSEL	CURRENT_DRIVE_STATE
    BSF		CURRENT_DRIVE_STATE,1	    ;Set Drive State Right Motor Reverse (0x_2)
    GOTO	CHECK_LEFT_MOTOR
RIGHT_MOTOR_STOP
    BANKSEL	CURRENT_DRIVE_STATE
    BCF		CURRENT_DRIVE_STATE,0
    BCF		CURRENT_DRIVE_STATE,1
    BCF		CURRENT_DRIVE_STATE,2
    BCF		CURRENT_DRIVE_STATE,3	    ;Set Drive State Right Motor Stop (0x_0)
    GOTO	CHECK_LEFT_MOTOR
;Set Different States for Left Motor (CURRENT_DRIVE_STATE, Bits 4-7)
LEFT_MOTOR_FORWARD
    BANKSEL	CURRENT_DRIVE_STATE
    BSF		CURRENT_DRIVE_STATE,4	    ;Set Drive State Left Motor Forward (0x1_)
    GOTO	DETERMINE_DRIVE_STATE_END
LEFT_MOTOR_REVERSE
    BANKSEL	CURRENT_DRIVE_STATE
    BSF		CURRENT_DRIVE_STATE,5	    ;Set Drive State Left Motor Reverse (0x2_)    
    GOTO	DETERMINE_DRIVE_STATE_END
LEFT_MOTOR_STOP
    BANKSEL	CURRENT_DRIVE_STATE
    BCF		CURRENT_DRIVE_STATE,4
    BCF		CURRENT_DRIVE_STATE,5
    BCF		CURRENT_DRIVE_STATE,6
    BCF		CURRENT_DRIVE_STATE,7	    ;Set Drive State Left Motor Stop (0x0_)
DETERMINE_DRIVE_STATE_END
    RETURN
   
;**********************************************************************************************************
;*** SET_DRIVE_STATE will run every time a full 4 byte I2C Packet is received.  This function will      ***
;***   Inspect the status of CURRENT_DRIVE_STATE to determine motor function.  It will then update      ***
;***    the outputs through setting Right and Left Motor Status Counts. See CURRENT_DRIVE_STATE bit map ***
;********************************************************************************************************** 
SET_DRIVE_STATE
    ;Check CURRENT_DRIVE_STATE to determine drive mode.  
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'00'
    SUBWF	CURRENT_DRIVE_STATE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	BOTH_STOP		    ;Current Mode is Stop
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'11'
    SUBWF	CURRENT_DRIVE_STATE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	BOTH_FORWARD		    ;Current Mode is Drive Forward
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'22'
    SUBWF	CURRENT_DRIVE_STATE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	BOTH_REVERSE		    ;Current Mode is Drive Reverse
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'10'
    SUBWF	CURRENT_DRIVE_STATE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	TURN_RIGHT_F		    ;Current Mode is Turn Right Forward
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'01'
    SUBWF	CURRENT_DRIVE_STATE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	TURN_LEFT_F		    ;Current Mode is Turn Left Forward
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'12'
    SUBWF	CURRENT_DRIVE_STATE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	Z_P_TURN_RIGHT		    ;Current Mode is Zero Point Turn Right
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'21'
    SUBWF	CURRENT_DRIVE_STATE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	Z_P_TURN_LEFT		    ;Current Mode is Zero Point Turn Left
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'02'
    SUBWF	CURRENT_DRIVE_STATE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	TURN_RIGHT_R		    ;Current Mode is Turn Right Reverse
    BANKSEL	CURRENT_DRIVE_STATE
    MOVLW	H'20'
    SUBWF	CURRENT_DRIVE_STATE,0
    BANKSEL	STATUS
    BTFSC	STATUS,2
    GOTO	TURN_LEFT_R		    ;Current Mode is Turn Left Reverse
    GOTO	BOTH_STOP		    ;If Current Mode Unknown Falls into Both Stop
;Mode determined.  Set Output Status Counts    
BOTH_STOP ;Right and Left Motors put into Stop.
    BANKSEL	STOP_COUNT
    MOVFW	STOP_COUNT
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS		    ;Stop Both Motors (1.5ms On Time)
    GOTO	SET_DRIVE_STATE_END
BOTH_FORWARD ;Right and Left Motors put into Forward.
    BANKSEL	FORWARD_COUNT
    MOVFW	FORWARD_COUNT
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS		    ;Forward Both Motors (2.5ms On Time)
    GOTO	SET_DRIVE_STATE_END
BOTH_REVERSE ;Right and Left Motors put into Reverse.
    BANKSEL	REVERSE_COUNT
    MOVFW	REVERSE_COUNT
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS		    ;Reverse Both Motors (0.5ms On Time)    
    GOTO	SET_DRIVE_STATE_END
Z_P_TURN_RIGHT ;Zero-Point Right Turn.  Right Motor Reverse.  Left Motor Forward.
    BANKSEL	FORWARD_COUNT
    MOVFW	FORWARD_COUNT
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS		    ;Forward Left Motor (2.5ms On Time)
    BANKSEL	REVERSE_COUNT
    MOVFW	REVERSE_COUNT
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS		    ;Reverse Right Motor (0.5mS On Time)
    GOTO	SET_DRIVE_STATE_END
Z_P_TURN_LEFT ;Zero-Point Left Turn.  Right Motor Forward.  Left Motor Reverse.
    BANKSEL	FORWARD_COUNT
    MOVFW	FORWARD_COUNT
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS		    ;Forward Right Motor (2.5mS On Time)
    BANKSEL	REVERSE_COUNT
    MOVFW	REVERSE_COUNT
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS		    ;Reverse Left Motor (0.5mS On Time)
    GOTO	SET_DRIVE_STATE_END
TURN_RIGHT_F ;Turn Right Through Forward Motion.  Right Motor Stop.  Left Motor Forward
    BANKSEL	STOP_COUNT
    MOVFW	STOP_COUNT
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS		    ;Stop Right Motor (1.5mS On Time)
    BANKSEL	FORWARD_COUNT
    MOVFW	FORWARD_COUNT
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS		    ;Forward Left Motor (2.5mS On Time)
    GOTO	SET_DRIVE_STATE_END
TURN_LEFT_F ;Turn Left Through Forward Motion.  Right Motor Forward.  Left Motor Stop  
    BANKSEL	STOP_COUNT
    MOVFW	STOP_COUNT
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS		    ;Stop Left Motor (1.5mS On Time)
    BANKSEL	FORWARD_COUNT
    MOVFW	FORWARD_COUNT
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS		    ;Forward Right Motor (2.5mS On Time)
    GOTO	SET_DRIVE_STATE_END
TURN_RIGHT_R ;Turn Right Through Reverse Motion.  Right Motor Reverse.  Left Motor Stop
    BANKSEL	STOP_COUNT
    MOVFW	STOP_COUNT
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS		    ;Stop Left Motor (1.5mS On Time)
    BANKSEL	REVERSE_COUNT
    MOVFW	REVERSE_COUNT
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS		    ;Reverse Right Motor (0.5mS On Time)
    GOTO	SET_DRIVE_STATE_END
TURN_LEFT_R ;Turn Left Through Reverse Motion.  Right Motor Stop.  Left Motor Reverse
    BANKSEL	STOP_COUNT
    MOVFW	STOP_COUNT
    BANKSEL	R_MOTOR_STATUS
    MOVWF	R_MOTOR_STATUS		    ;Stop Right Motor (1.5mS On Time)
    BANKSEL	REVERSE_COUNT
    MOVFW	REVERSE_COUNT
    BANKSEL	L_MOTOR_STATUS
    MOVWF	L_MOTOR_STATUS		    ;Reverse Left Motor (0.5mS On Time)
SET_DRIVE_STATE_END
    RETURN
   
;*********************************************************************************************************
;*** DETERMINE_ACUTATOR_STATE will run every time a full 4 byte I2C Packet is received.  This function ***
;***  Will compare joystick to threshholds and set the actuator output status count accoridingly.      ***
;***   This template uses joystick 2 left and right action as an example.  If Joystick 2 is above the  ***
;***   threshhold actuator motor is moves forward, below the threshold actuator motor moves reverse.   ***
;***   If it is neither above or below, actuator motor is set to stop.				       ***
;********************************************************************************************************* 
DETERMINE_ACTUATOR_STATE
    BANKSEL	UPPER_THRESHOLD
    MOVFW	UPPER_THRESHOLD
    BANKSEL	JOY2_LR
    SUBWF	JOY2_LR,0		;Subtract Upper Threshold from Joystick Value (Stored in W)
    BANKSEL	STATUS			;JOY2_LR - UPPER_THRESHOLD
    BTFSC	STATUS,0
    GOTO	ACTUATOR_MOTOR_FORWARD	;Result is Positive Joystick is above Threshold
    BANKSEL	LOWER_THRESHOLD
    MOVFW	LOWER_THRESHOLD
    BANKSEL	JOY2_LR
    SUBWF	JOY2_LR,0		;Subtract Lower Threshold from Joystick Value (Stored in W)
    BANKSEL	STATUS			;JOY2_LR - LOWER_THRESHOLD
    BTFSS	STATUS,0
    GOTO	ACTUATOR_MOTOR_REVERSE	;Result is Negative Joystick is below Threshold
    GOTO	ACTUATOR_MOTOR_STOP	;Joystick is Neither above or below thresholds
ACTUATOR_MOTOR_FORWARD
    BANKSEL	FORWARD_COUNT
    MOVFW	FORWARD_COUNT
    BANKSEL	ACTUATOR_MOTOR_STATUS
    MOVWF	ACTUATOR_MOTOR_STATUS
    GOTO	DETERMINE_ACTUATOR_STATE_END
ACTUATOR_MOTOR_REVERSE
    BANKSEL	REVERSE_COUNT
    MOVFW	REVERSE_COUNT
    BANKSEL	ACTUATOR_MOTOR_STATUS
    MOVWF	ACTUATOR_MOTOR_STATUS
    GOTO	DETERMINE_ACTUATOR_STATE_END
ACTUATOR_MOTOR_STOP
    BANKSEL	STOP_COUNT
    MOVFW	STOP_COUNT
    BANKSEL	ACTUATOR_MOTOR_STATUS
    MOVWF	ACTUATOR_MOTOR_STATUS
DETERMINE_ACTUATOR_STATE_END
    RETURN
    
;******************************************
;Main Code
;******************************************
MAIN
    ;************************************************************************************************
    ;*** UPDATE_DRIVE_STATUS will run every time a full 4 byte I2C Packet is received.  The flag  ***
    ;*** I2C_RX_COMPLETE Bit 0 is set within the I2C_RECEIVE function after the fourth data       ***
    ;*** byte is received.  See Unv_Slave_I2C_SetUp.inc					          ***
    ;************************************************************************************************
    BANKSEL	I2C_RX_COMPLETE
    BTFSC	I2C_RX_COMPLETE,0
    CALL	UPDATE_DRIVE_STATUS	    ;Full I2C Data Packet Received.  Save Temp Data, Check Joysticks, Update Outputs
    GOTO	MAIN

    END				;END PROGRAM DIRECTIVE *******************************
;*************************************************************************************************************
    