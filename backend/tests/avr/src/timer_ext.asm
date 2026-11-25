; timer_ext (plan-62-B): timer 0 clocked from T0 (PD4) on the falling and then
; on the rising edge, and timer 1 from T1 (PD5), by pin stimuli. The counts
; go out on PORTB and PORTC so the trace shows them.
  jmp start
_LOC 0x2c
  jmp isr_compa1     ; 0x0016 TIMER1 COMPA
_LOC 0x38
  jmp isr_compa0     ; 0x001c TIMER0 COMPA
start:
  ldi r16, 0xff
  out 0x04, r16      ; DDRB: all outputs
  out 0x07, r16      ; DDRC: all outputs
  ldi r16, 0x03
  out 0x27, r16      ; OCR0A = 3
  ldi r16, 0x02
  sts 0x6e, r16      ; TIMSK0: compare A
  sts 0x6f, r16      ; TIMSK1: compare A
  ldi r16, 0x00
  sts 0x89, r16
  ldi r16, 0x05
  sts 0x88, r16      ; OCR1A = 5
  ldi r16, 0x02
  out 0x24, r16      ; timer 0 CTC
  ldi r16, 0x06
  out 0x25, r16      ; timer 0: T0, falling edge
  ldi r16, 0x0f
  sts 0x81, r16      ; timer 1: CTC, T1, rising edge
  sei
  ldi r19, 90
phase1:
  rcall show
  dec r19
  brne phase1
  ldi r16, 0x07
  out 0x25, r16      ; timer 0: T0, rising edge
  ldi r16, 0x0e
  sts 0x81, r16      ; timer 1: T1, falling edge
  ldi r19, 90
phase2:
  rcall show
  dec r19
  brne phase2
  ldi r16, 0x01
  out 0x25, r16      ; timer 0 back on the system clock
  ldi r16, 0x00
  sts 0x81, r16      ; timer 1 stopped
end:
  rcall show
  rjmp end
show:
  in r20, 0x26       ; TCNT0
  out 0x05, r20      ; on PORTB
  lds r21, 0x84      ; TCNT1L
  out 0x08, r21      ; on PORTC
  ret
isr_compa0:
  inc r24
  reti
isr_compa1:
  inc r25
  reti
