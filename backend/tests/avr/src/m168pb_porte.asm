; m168pb_porte (plan-62-D): port E of the ATmega48PB/88PB/168PB (PE0-PE3):
; DDRE, PORTE and PINE writes, the PINE toggle, pins read back.
  ldi r16, 0x05
  out 0x0d, r16      ; DDRE: PE0 and PE2 outputs
  ldi r16, 0x0f
  out 0x0e, r16      ; PORTE: outputs high, pull-ups on PE1 and PE3
  in r17, 0x0c       ; PINE
  ldi r16, 0x03
  out 0x0c, r16      ; PINE toggles PORTE0 and PORTE1
  in r18, 0x0e
  sbi 0x0e, 0
  cbi 0x0e, 2
  sbi 0x0d, 3        ; PE3 output
  sbi 0x0c, 3        ; toggled through PINE
  ldi r16, 0xff
  out 0x04, r16      ; DDRB: PORTB shows PINE
loop:
  in r19, 0x0c
  out 0x05, r19
  inc r20
  out 0x0e, r20      ; PORTE counts
  rjmp loop
