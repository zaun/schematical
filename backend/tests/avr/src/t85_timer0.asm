; t85_timer0: an ATtiny85's Timer0 (TCCR0A 0x2a, TCCR0B 0x33, TCNT0 0x32,
; OCR0A 0x29, OCR0B 0x28) with its flags and enables in the TIFR (0x38) and
; TIMSK (0x39) it shares with Timer1: normal, CTC and fast PWM modes, OC0A
; (PB0) and OC0B (PB1), the three interrupts, the external clock on T0 (PB2,
; moved by stimuli), the flags cleared by writing 1.
  rjmp start         ; 0x000 RESET
_LOC 0x0a
  rjmp isr_ovf       ; 0x005 TIMER0 OVF
_LOC 0x14
  rjmp isr_compa     ; 0x00a TIMER0 COMPA
  rjmp isr_compb     ; 0x00b TIMER0 COMPB
start:
  ldi r16, 0x03
  out 0x17, r16      ; DDRB: PB0 and PB1 outputs
  ldi r16, 0x20
  out 0x29, r16      ; OCR0A
  ldi r16, 0x40
  out 0x28, r16      ; OCR0B
  ldi r16, 0x1a
  out 0x39, r16      ; TIMSK: OCIE0A, OCIE0B, TOIE0
  sei
; normal mode, both outputs toggle on match, clk/1
  ldi r16, 0x50
  out 0x2a, r16
  ldi r16, 0x01
  out 0x33, r16
  rcall wait
  in r17, 0x32       ; TCNT0 read mid-count
  ldi r16, 0x10
  out 0x32, r16      ; and written
  rcall wait
; CTC (TOP OCR0A): A clears, B sets on match, clk/8
  ldi r16, 0xb2
  out 0x2a, r16
  ldi r16, 0x02
  out 0x33, r16
  rcall wait
  rcall wait
; fast PWM, TOP 0xFF: A non-inverting, B inverting, clk/1
  ldi r16, 0xb3
  out 0x2a, r16
  ldi r16, 0x01
  out 0x33, r16
  rcall wait
  ldi r16, 0x80
  out 0x29, r16
  rcall wait
; phase correct, TOP 0xFF, clk/1
  ldi r16, 0xa1
  out 0x2a, r16
  rcall wait
  rcall wait
; the external clock: T0's rising edge, normal mode, A toggles
  ldi r16, 0x04
  out 0x29, r16      ; OCR0A: 4 edges
  ldi r16, 0x42
  out 0x2a, r16      ; CTC, A toggles
  ldi r16, 0x07
  out 0x33, r16
  rcall longwait
  ldi r16, 0x06
  out 0x33, r16      ; the falling edge
  rcall longwait
; the flags clear by writing 1; stop the timer
  cli
  ldi r16, 0x01
  out 0x33, r16
  ldi r16, 0x00
  out 0x2a, r16
  rcall wait
  in r17, 0x38       ; TIFR
  ldi r16, 0x1a
  out 0x38, r16
  in r17, 0x38
  ldi r16, 0x00
  out 0x39, r16      ; TIMSK off
  out 0x33, r16      ; stopped
  sei
end:
  inc r20
  in r21, 0x32
  rjmp end
wait:
  ldi r18, 0
wait1:
  dec r18
  brne wait1
  ret
longwait:
  ldi r19, 6
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
  in r27, 0x32
  reti
