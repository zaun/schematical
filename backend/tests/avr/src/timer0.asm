; timer0 (plan-62-B): the 8-bit timer 0 in normal, CTC, fast PWM (TOP 0xFF and
; OCR0A) and phase-correct modes; OC0A (PD6) and OC0B (PD5) set, clear and
; toggle; the overflow and both compare interrupts; a TCNT0 read and write
; mid-count; every prescaler; force output compare.
  jmp start
_LOC 0x38
  jmp isr_compa      ; 0x001c TIMER0 COMPA
  jmp isr_compb      ; 0x001e TIMER0 COMPB
  jmp isr_ovf        ; 0x0020 TIMER0 OVF
start:
  ldi r16, 0x60
  out 0x0a, r16      ; DDRD: PD6 and PD5 outputs
  ldi r16, 0x20
  out 0x27, r16      ; OCR0A
  ldi r16, 0x40
  out 0x28, r16      ; OCR0B
  ldi r16, 0x07
  sts 0x6e, r16      ; TIMSK0: overflow and both compares
  sei
; normal mode, both outputs toggle on match, clk/1
  ldi r16, 0x50
  out 0x24, r16      ; TCCR0A
  ldi r16, 0x01
  out 0x25, r16      ; TCCR0B
  rcall wait
  in r17, 0x26       ; TCNT0 read mid-count
  ldi r16, 0x10
  out 0x26, r16      ; TCNT0 written mid-count
  rcall wait
; CTC (TOP OCR0A): A clears, B sets on match, clk/8
  ldi r16, 0xb2
  out 0x24, r16
  ldi r16, 0x02
  out 0x25, r16
  rcall wait
  rcall wait
; fast PWM, TOP 0xFF: A non-inverting, B inverting, clk/1
  ldi r16, 0xb3
  out 0x24, r16
  ldi r16, 0x01
  out 0x25, r16
  rcall wait
  ldi r16, 0x80
  out 0x27, r16      ; OCR0A changes at BOTTOM
  ldi r16, 0x00
  out 0x28, r16      ; OCR0B 0
  rcall wait
  ldi r16, 0xff
  out 0x27, r16      ; OCR0A at TOP
  rcall wait
; fast PWM, TOP OCR0A: A toggles, B inverting, clk/64
  ldi r16, 0x20
  out 0x27, r16
  ldi r16, 0x10
  out 0x28, r16
  ldi r16, 0x73
  out 0x24, r16
  ldi r16, 0x0b
  out 0x25, r16
  rcall longwait
; phase correct, TOP 0xFF, clk/1
  ldi r16, 0xa1
  out 0x24, r16
  ldi r16, 0x01
  out 0x25, r16
  rcall wait
  rcall wait
  ldi r16, 0x05
  out 0x26, r16      ; TCNT0 written while counting down
  rcall wait
; phase correct, TOP OCR0A: A toggles, clk/8
  ldi r16, 0x61
  out 0x24, r16
  ldi r16, 0x0a
  out 0x25, r16
  rcall longwait
; clk/256 and clk/1024, normal mode, no outputs
  ldi r16, 0x00
  out 0x24, r16
  ldi r16, 0x04
  out 0x25, r16
  rcall longwait
  ldi r16, 0x05
  out 0x25, r16
  rcall longwait
; force output compare (normal mode only)
  ldi r16, 0x50
  out 0x24, r16
  ldi r16, 0xc1
  out 0x25, r16      ; FOC0A, FOC0B, clk/1
  ldi r16, 0x81
  out 0x25, r16      ; FOC0A again
  ldi r16, 0xb3
  out 0x24, r16      ; fast PWM: FOC does nothing
  ldi r16, 0xc1
  out 0x25, r16
; the flags clear by writing 1; stop the timer
  cli
  rcall wait
  in r17, 0x15       ; TIFR0
  ldi r16, 0x07
  out 0x15, r16
  in r17, 0x15
  ldi r16, 0x00
  sts 0x6e, r16      ; TIMSK0 off
  out 0x25, r16      ; stopped
  out 0x24, r16      ; outputs released
  sei
end:
  inc r20
  in r21, 0x26
  rjmp end
wait:
  ldi r18, 0
wait1:
  dec r18
  brne wait1
  ret
longwait:
  ldi r19, 20
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
  in r27, 0x26
  reti
