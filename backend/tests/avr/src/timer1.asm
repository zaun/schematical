; timer1 (plan-62-B): the 16-bit timer 1 in every waveform mode: normal, CTC
; with OCR1A and with ICR1 as TOP, fast and phase-correct PWM at 8, 9 and 10
; bits and with ICR1 / OCR1A as TOP, phase-and-frequency-correct; the TEMP
; latch on 16-bit reads and writes; force output compare. OC1A is PB1 and
; OC1B is PB2.
  jmp start
_LOC 0x28
  jmp isr_capt       ; 0x0014 TIMER1 CAPT
  jmp isr_compa      ; 0x0016 TIMER1 COMPA
  jmp isr_compb      ; 0x0018 TIMER1 COMPB
  jmp isr_ovf        ; 0x001a TIMER1 OVF
start:
  ldi r16, 0x06
  out 0x04, r16      ; DDRB: PB1 and PB2 outputs
  ldi r16, 0x07
  sts 0x6f, r16      ; TIMSK1: overflow and both compares
  ldi r16, 0x01
  sts 0x89, r16      ; OCR1AH (through TEMP)
  ldi r16, 0x00
  sts 0x88, r16      ; OCR1A = 0x0100
  ldi r16, 0x02
  sts 0x8b, r16
  ldi r16, 0x00
  sts 0x8a, r16      ; OCR1B = 0x0200
  ldi r16, 0x01
  sts 0x87, r16
  ldi r16, 0x80
  sts 0x86, r16      ; ICR1 = 0x0180
  sei
; mode 0, normal: both outputs toggle, clk/1
  ldi r16, 0x50
  ldi r17, 0x01
  rcall setmode
  lds r20, 0x84      ; TCNT1L latches TCNT1H into TEMP
  lds r21, 0x85
  ldi r16, 0xff
  sts 0x85, r16      ; TCNT1H through TEMP
  ldi r16, 0x00
  sts 0x84, r16      ; TCNT1 = 0xff00: overflow soon
  rcall wait
; mode 4, CTC with OCR1A
  ldi r16, 0x50
  ldi r17, 0x09
  rcall setmode
; mode 12, CTC with ICR1
  ldi r16, 0x50
  ldi r17, 0x19
  rcall setmode
; modes 5, 6, 7: fast PWM 8, 9, 10 bits: A non-inverting, B inverting
  ldi r16, 0xb1
  ldi r17, 0x09
  rcall setmode
  ldi r16, 0xb2
  ldi r17, 0x09
  rcall setmode
  ldi r16, 0xb3
  ldi r17, 0x09
  rcall setmode
; modes 1, 2, 3: phase correct 8, 9, 10 bits
  ldi r16, 0xa1
  ldi r17, 0x01
  rcall setmode
  ldi r16, 0xa2
  ldi r17, 0x01
  rcall setmode
  ldi r16, 0xa3
  ldi r17, 0x01
  rcall setmode
; mode 14, fast PWM with ICR1; mode 15, with OCR1A (A toggles)
  ldi r16, 0xb2
  ldi r17, 0x19
  rcall setmode
  ldi r16, 0x63
  ldi r17, 0x19
  rcall setmode
; mode 10, phase correct with ICR1; mode 11, with OCR1A (A toggles)
  ldi r16, 0xa2
  ldi r17, 0x11
  rcall setmode
  ldi r16, 0x63
  ldi r17, 0x11
  rcall setmode
; mode 8, phase and frequency correct with ICR1; mode 9, with OCR1A
  ldi r16, 0xa0
  ldi r17, 0x11
  rcall setmode
  ldi r16, 0x61
  ldi r17, 0x11
  rcall setmode
; an OCR write in a PWM mode waits for its update point
  ldi r16, 0x00
  sts 0x89, r16
  ldi r16, 0x30
  sts 0x88, r16      ; OCR1A = 0x0030
  rcall wait
; mode 13 (reserved), then clk/8 and clk/64 in normal mode
  ldi r16, 0x51
  ldi r17, 0x19
  rcall setmode
  ldi r16, 0x50
  ldi r17, 0x02
  rcall setmode
  ldi r16, 0x50
  ldi r17, 0x03
  rcall setmode
; force output compare through TCCR1C (normal mode)
  ldi r16, 0xc0
  sts 0x82, r16
  ldi r16, 0x80
  sts 0x82, r16
; the flags clear by writing 1; stop
  cli
  rcall wait
  in r22, 0x16       ; TIFR1
  ldi r16, 0x27
  out 0x16, r16
  in r22, 0x16
  ldi r16, 0x00
  sts 0x6f, r16
  sts 0x81, r16
  sts 0x80, r16
  sei
end:
  inc r23
  lds r20, 0x84
  lds r21, 0x85
  rjmp end
setmode:
  sts 0x80, r16      ; TCCR1A
  sts 0x81, r17      ; TCCR1B
  rcall wait
  rcall wait
  rcall wait
  rcall wait
  ret
wait:
  ldi r18, 0
wait1:
  dec r18
  brne wait1
  ret
isr_capt:
  inc r27
  reti
isr_compa:
  inc r24
  reti
isr_compb:
  inc r25
  reti
isr_ovf:
  inc r26
  reti
