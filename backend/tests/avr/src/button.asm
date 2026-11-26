; button (plan-62-E): PD2 is an input with its pull-up; PB0 follows it.
  sbi 0x04, 0        ; DDRB0: output
  sbi 0x0b, 2        ; PORTD2: the pull-up
loop:
  sbic 0x09, 2       ; PIND2
  rjmp high
  cbi 0x05, 0        ; PB0 low
  rjmp loop
high:
  sbi 0x05, 0        ; PB0 high
  rjmp loop
