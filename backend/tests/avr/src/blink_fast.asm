; blink_fast (plan-62-E): blink after setting CLKPR to 0: on the internal
; oscillator the chip then runs at 8 MHz instead of 1 MHz. PB5 toggles every
; 1000 CPU cycles.
  jmp start
_LOC 0x2c
  jmp isr            ; vector 11: TIMER1 COMPA
start:
  ldi r16, 0x80
  ldi r17, 0x00
  sts 0x61, r16      ; CLKPR: change enable
  sts 0x61, r17      ; divide by 1
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
