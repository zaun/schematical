; twi1_rtc (plan-62-F): twi_rtc on the ATmega328PB's second TWI (TWI1: SDA1
; on PE0, SCL1 on PE1, registers at 0xD8-0xDD). Writes 0x25 to register 0x02
; of the clock at 0x51, reads it back and puts it on PORTD.
  ldi r16, 0x03
  out 0x0e, r16      ; PORTE0, PORTE1: the pull-ups
  ldi r16, 0xff
  out 0x0a, r16      ; DDRD: all outputs
  ldi r16, 2
  sts 0xd8, r16      ; TWBR1
  rcall tw_start
  ldi r16, 0xa2      ; SLA+W
  rcall tw_send
  ldi r16, 0x02
  rcall tw_send
  ldi r16, 0x25
  rcall tw_send
  rcall tw_start     ; repeated start
  ldi r16, 0xa2
  rcall tw_send
  ldi r16, 0x02
  rcall tw_send
  rcall tw_start
  ldi r16, 0xa3      ; SLA+R
  rcall tw_send
  ldi r16, 0x84
  rcall tw_command
  lds r18, 0xdb      ; TWDR1
  rcall tw_stop
  out 0x0b, r18      ; PORTD
end:
  rjmp end
tw_start:
  ldi r16, 0xa4
  rjmp tw_command
tw_stop:
  ldi r16, 0x94
  rjmp tw_command
tw_send:
  sts 0xdb, r16      ; TWDR1
  ldi r16, 0x84
tw_command:
  sts 0xdc, r16      ; TWCR1
tw_wait:
  lds r17, 0xdc
  sbrs r17, 7        ; TWINT
  rjmp tw_wait
  ret
