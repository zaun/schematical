; t85_blink: an ATtiny25/45/85. PB0 toggles every 1000 CPU cycles, by the
; Timer1 compare A interrupt (CTC, TOP OCR1C = 249, CK/4). One word per
; vector, and small enough for the ATtiny25's 128 bytes of SRAM.
  rjmp start
_LOC 0x06
  rjmp isr           ; vector 3: TIMER1 COMPA
start:
  sbi 0x17, 0        ; DDRB0: output
  ldi r16, 249
  out 0x2d, r16      ; OCR1C: TOP
  out 0x2e, r16      ; OCR1A
  ldi r16, 0x40
  out 0x39, r16      ; TIMSK: OCIE1A
  ldi r16, 0x83
  out 0x30, r16      ; TCCR1: CTC1, CK/4
  sei
loop:
  rjmp loop
isr:
  push r16
  sbi 0x16, 0        ; PINB0: toggle PB0
  pop r16
  reti
