; timer2 (plan-62-B): timer 2 with its own prescaler set (clk/32 and clk/128
; exist only here), fast PWM on OC2A (PB3) and OC2B (PD3), and its
; interrupts.
  jmp start
_LOC 0x1c
  jmp isr_compa      ; 0x000e TIMER2 COMPA
  jmp isr_compb      ; 0x0010 TIMER2 COMPB
  jmp isr_ovf        ; 0x0012 TIMER2 OVF
start:
  ldi r16, 0x08
  out 0x04, r16      ; DDRB: PB3 output
  out 0x0a, r16      ; DDRD: PD3 output
  ldi r16, 0x30
  sts 0xb3, r16      ; OCR2A
  ldi r16, 0x90
  sts 0xb4, r16      ; OCR2B
  ldi r16, 0x07
  sts 0x70, r16      ; TIMSK2
  sei
; fast PWM, TOP 0xFF: A non-inverting, B inverting; clk/1, /8, /32, /64, /128
  ldi r16, 0xb3
  sts 0xb0, r16      ; TCCR2A
  ldi r17, 0x01
  rcall setclock
  ldi r17, 0x02
  rcall setclock
  ldi r17, 0x03
  rcall setclock
  ldi r17, 0x04
  rcall setclock
  ldi r17, 0x05
  rcall setclock
; CTC: A toggles; clk/256 and clk/1024
  ldi r16, 0x42
  sts 0xb0, r16
  ldi r17, 0x06
  rcall setclock
  ldi r17, 0x07
  rcall setclock
; phase correct, TOP OCR2A: A toggles; clk/32
  ldi r16, 0x61
  sts 0xb0, r16
  ldi r17, 0x0b
  rcall setclock
  lds r20, 0xb2      ; TCNT2
  ldi r16, 0x00
  sts 0xb1, r16      ; stopped
  sts 0xb0, r16
end:
  inc r23
  lds r20, 0xb2
  rjmp end
setclock:
  sts 0xb1, r17      ; TCCR2B
  ldi r19, 12
setclock1:
  rcall wait
  dec r19
  brne setclock1
  ret
wait:
  ldi r18, 0
wait1:
  dec r18
  brne wait1
  ret
isr_compa:
  inc r24
  reti
isr_compb:
  inc r25
  reti
isr_ovf:
  inc r26
  reti
