#include <p16F690.inc>

	errorlevel -302 
	__config (_INTRC_OSC_NOCLKOUT & _WDT_OFF & _PWRTE_OFF & _MCLRE_OFF & _CP_OFF & _BOR_OFF & _IESO_OFF & _FCMEN_OFF)
	
	cblock  0x20
		d1	; Define two file registers for the
		d2      ; delay loop
	endc

; Definitions for motorcontrol
#define ONOFF	PORTA, 3
#define	DIR_L	PORTA, 4
#define	DIR_R	PORTA, 5

; Ports that the motors are connected to
#define	SERVO_L	PORTC, 0
#define	SERVO_R	PORTC, 2

; Used to control the lift
#define	C_LIFT	PORTA, 1 
#define S_LIFT	PORTC, 1

	org 0

init:
	; Disable ADC
	banksel	ANSEL
	clrf	ANSEL
	banksel	ANSELH
	clrf	ANSELH

	; Use All PORTA as input
	banksel	TRISA
	movlw	0xff
	movwf	TRISA
	
	; Use all PORTB as output
	banksel	TRISB
	movlw	0x0
	movwf	TRISB

	; Use all PORTB as output
	banksel	TRISC
	movlw	0x0
	movwf	TRISC

	; Duuuurrr?
	bcf		STATUS,RP0

	; Clear all ports
	clrf	PORTA
	clrf	PORTB
	clrf	PORTC

main:
	; Time to drive?
	btfsc	ONOFF
	call	drive
	; Time to lift?
	btfss	C_LIFT
	call	lift
	; 20 ms delay
	call	delay_20
	goto	main			; wait here

drive:
	; Engine 1
	bsf		SERVO_L			; Output to servo
	btfss	DIR_L
	call	delay_1			; Anti-clockwise
	btfsc	DIR_L
	call	delay_2			; Clockwise
	bcf		SERVO_L			; Stop pulse to servo

	; Engine 2
	bsf		SERVO_R			; Output to servo
	btfss	DIR_R
	call	delay_1			; Anti-clockwise
	btfsc	DIR_R
	call	delay_2			; Clockwise
	bcf		SERVO_R			; Stop pulse to servo
	return

lift:
	; Servo connected to the lift
	bsf		S_LIFT			; Output to servo
	call	delay_1			; Anti-clockwise
	bcf		S_LIFT			; Stop pulse to servo
	return

delay_1:
	; Delay 1ms
	movlw	2
	movwf	d1
	movlw	75
	movwf	d2
delay_1_inner:
	decfsz	d2, 1
	goto	delay_1_inner
	decfsz	d1, 1
	goto	delay_1_inner
	return

delay_2:
	; Delay 2ms
	movlw	3
	movwf	d1
	movlw	151
	movwf	d2
delay_2_inner:
	decfsz	d2, 1
	goto	delay_2_inner
	decfsz	d1, 1
	goto	delay_2_inner
	nop
	return

delay_20:
	; Delay 20ms
	movlw	26
	movwf	d1
	movlw	248
	movwf	d2
delay_20_inner:
	decfsz	d2, 1
	goto	delay_20_inner
	decfsz	d1, 1
	goto	delay_20_inner
	nop
	return

	end