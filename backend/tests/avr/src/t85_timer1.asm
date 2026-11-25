; t85_timer1: an ATtiny85's Timer1 (TCCR1 0x30, GTCCR 0x2c, TCNT1 0x2f, OCR1A
; 0x2e, OCR1B 0x2b, OCR1C 0x2d): the normal mode, CTC with OCR1C as TOP, PWM
; on A and on B, prescalers from CK to CK/16384, OC1A (PB1) and OC1B (PB4)
; toggling, clearing and setting, the overflow and both compare interrupts,
; TCNT1 read and written mid-count, force output compare, the prescaler
; reset, the flags cleared by writing 1, and Timer0 running beside it on the
; shared TIFR (0x38) and TIMSK (0x39).
  rjmp start         ; 0x000 RESET
_LOC 0x06
  rjmp isr_compa     ; 0x003 TIMER1 COMPA
  rjmp isr_ovf       ; 0x004 TIMER1 OVF
  rjmp isr_ovf0      ; 0x005 TIMER0 OVF
_LOC 0x12
  rjmp isr_compb     ; 0x009 TIMER1 COMPB
start:
  ldi r16, 0x12
  out 0x17, r16      ; DDRB: PB1 and PB4 outputs
  ldi r16, 0x30
  out 0x2e, r16      ; OCR1A
  ldi r16, 0x60
  out 0x2b, r16      ; OCR1B
  ldi r16, 0x64
  out 0x39, r16      ; TIMSK: OCIE1A, OCIE1B, TOIE1
  sei
; normal mode (TOP 0xFF), both outputs toggle on match, CK
  ldi r16, 0x10
  out 0x2c, r16      ; GTCCR: COM1B0
  ldi r16, 0x11
  out 0x30, r16      ; TCCR1: COM1A0, CK
  rcall wait
  in r17, 0x2f       ; TCNT1 read mid-count
  ldi r16, 0x08
  out 0x2f, r16      ; and written
  rcall wait
; CTC: TOP is OCR1C; A clears, B sets on match; CK/4
  ldi r16, 0x7f
  out 0x2d, r16      ; OCR1C
  ldi r16, 0x30
  out 0x2c, r16      ; COM1B1:0 = 11
  ldi r16, 0xa3
  out 0x30, r16      ; CTC1, COM1A1, CK/4
  rcall wait
  rcall wait
  rcall wait
; PWM on A (PWM1A), non-inverting, TOP OCR1C, CK
  ldi r16, 0x00
  out 0x2c, r16
  ldi r16, 0x61
  out 0x30, r16      ; PWM1A, COM1A1, CK
  rcall wait
  rcall wait
  ldi r16, 0x10
  out 0x2e, r16      ; OCR1A changed while the PWM runs
  rcall wait
  rcall wait
; PWM on A and B, A inverting (COM 11), B with COM 01, CK/2
  ldi r16, 0x50
  out 0x2c, r16      ; PWM1B, COM1B0
  ldi r16, 0x72
  out 0x30, r16      ; PWM1A, COM1A 11, CK/2
  rcall wait
  rcall wait
  rcall wait
  ldi r16, 0x05
  out 0x2f, r16      ; TCNT1 written in PWM mode
  rcall wait
; PWM with CTC1 set, CK/8
  ldi r16, 0xe4
  out 0x30, r16
  rcall wait
  rcall wait
  rcall wait
; Timer0 beside it: its overflow interrupt through the shared TIMSK
  ldi r16, 0x66
  out 0x39, r16      ; TIMSK: the three of Timer1 and TOIE0
  ldi r16, 0x01
  out 0x33, r16      ; TCCR0B: clk/1
  ldi r16, 0x00
  out 0x2c, r16
  ldi r16, 0x15
  out 0x30, r16      ; normal, A toggles, CK/16
  rcall longwait
; the slow prescalers: CK/64, CK/1024, CK/16384
  ldi r16, 0x17
  out 0x30, r16
  rcall longwait
  ldi r16, 0x1b
  out 0x30, r16
  rcall longwait
  ldi r16, 0x1f
  out 0x30, r16
  rcall longwait
; force output compare, and the prescaler reset
  ldi r16, 0x11
  out 0x30, r16
  ldi r16, 0x1c
  out 0x2c, r16      ; GTCCR: COM1B0, FOC1B, FOC1A
  ldi r16, 0x12
  out 0x2c, r16      ; COM1B0, PSR1
  in r17, 0x2c       ; the strobes read 0
; the flags clear by writing 1; stop both timers
  cli
  rcall wait
  in r17, 0x38       ; TIFR
  ldi r16, 0x66
  out 0x38, r16
  in r17, 0x38
  ldi r16, 0x00
  out 0x39, r16      ; TIMSK off
  out 0x30, r16      ; Timer1 stopped, its outputs released
  out 0x2c, r16
  out 0x33, r16      ; Timer0 stopped
  sei
end:
  inc r20
  in r21, 0x2f
  rjmp end
wait:
  ldi r18, 0
wait1:
  dec r18
  brne wait1
  ret
longwait:
  ldi r19, 12
longwait1:
  rcall wait
  dec r19
  brne longwait1
  ret
isr_compa:
  inc r24
  reti
isr_compb:
  inc r25
  reti
isr_ovf:
  inc r26
  in r27, 0x2f
  reti
isr_ovf0:
  inc r28
  reti
