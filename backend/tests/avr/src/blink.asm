; blink (plan-62-E): PB5 toggles every 1000 CPU cycles, by the timer 1
; compare A interrupt (CTC, TOP 999, clk/1). For a chip with two-word vectors.
  jmp start
_LOC 0x2c
  jmp isr            ; vector 11: TIMER1 COMPA
start:
  sbi 0x04, 5        ; DDRB5: output
  ldi r16, 0x03
  sts 0x89, r16
  ldi r16, 0xe7
  sts 0x88, r16      ; OCR1A = 999
  ldi r16, 0x02
  sts 0x6f, r16      ; TIMSK1: OCIE1A
  ldi r16, 0x09
  sts 0x81, r16      ; TCCR1B: CTC, clk/1
  sei
loop:
  rjmp loop
isr:
  sbi 0x03, 5        ; PINB5: toggle PB5
  reti
