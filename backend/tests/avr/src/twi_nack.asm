; twi_nack (plan-62-F): the TWI master addresses 0x20, where nobody answers;
; the status (0x20: SLA+W sent, no acknowledge) goes to PORTB.
  ldi r16, 0x30
  out 0x08, r16      ; PORTC4, PORTC5: the pull-ups
  ldi r16, 0xff
  out 0x04, r16      ; DDRB: all outputs
  ldi r16, 2
  sts 0xb8, r16      ; TWBR
  ldi r16, 0xa4      ; start
  rcall tw_command
  ldi r16, 0x40      ; SLA+W for 0x20
  sts 0xbb, r16
  ldi r16, 0x84
  rcall tw_command
  lds r18, 0xb9      ; TWSR
  andi r18, 0xf8
  ldi r16, 0x94      ; stop
  rcall tw_command
  out 0x05, r18      ; PORTB
end:
  rjmp end
tw_command:
  sts 0xbc, r16      ; TWCR
tw_wait:
  lds r17, 0xbc
  sbrs r17, 7        ; TWINT
  rjmp tw_wait
  ret
