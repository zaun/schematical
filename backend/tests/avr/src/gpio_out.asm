; gpio_out (plan-62-B): DDRx/PORTx/PINx writes, the PINx toggle, SBI/CBI on
; a port, pull-up state changes, reads of PINx on outputs and inputs.
  ldi r16, 0xff
  out 0x04, r16      ; DDRB: all outputs
  ldi r16, 0x55
  out 0x05, r16      ; PORTB
  in r17, 0x03       ; PINB reads back the outputs
  ldi r16, 0x0f
  out 0x03, r16      ; a write to PINB toggles PORTB bits 0-3
  in r18, 0x05
  sbi 0x0a, 3        ; DDRD3: output
  sbi 0x0b, 3        ; PORTD3 high
  cbi 0x0b, 3        ; and low
  sbi 0x0b, 5        ; PORTD5 on an input: the pull-up
  sbi 0x09, 5        ; PIND5 toggles PORTD5: pull-up off
  in r19, 0x09
  ldi r16, 0x00
  out 0x04, r16      ; DDRB: all inputs, pull-ups where PORTB is 1
  in r20, 0x03       ; PINB now reads the outside
  ldi r16, 0xf0
  out 0x07, r16      ; DDRC
  out 0x08, r16      ; PORTC
  sbi 0x06, 7        ; PINC7 toggles PORTC7
  sbi 0x06, 1        ; PINC1 toggles the pull-up of an input
loop:
  inc r21
  out 0x05, r21      ; PORTB counts
  out 0x04, r21      ; and so does DDRB
  in r22, 0x03
  in r23, 0x09
  in r24, 0x06
  rjmp loop
