; int0 (plan-62-E): a falling edge on PD2 (INT0, with its pull-up) toggles PB1.
  jmp start
  jmp isr            ; vector 1: INT0
start:
  sbi 0x04, 1        ; DDRB1: output
  sbi 0x0b, 2        ; PORTD2: the pull-up
  ldi r16, 0x02
  sts 0x69, r16      ; EICRA: INT0 on the falling edge
  sbi 0x1d, 0        ; EIMSK: INT0
  sei
loop:
  rjmp loop
isr:
  sbi 0x03, 1        ; PINB1: toggle PB1
  reti
