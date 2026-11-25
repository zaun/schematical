; twi_rtc (plan-62-F): the TWI master against an I2C real-time clock at
; address 0x51 (PCF8563), with the port pull-ups on SDA (PC4) and SCL (PC5).
; Writes 0x25 to register 0x02 (seconds), sets the register address again
; with a repeated start, reads the register back and puts it on PORTB.
  ldi r16, 0x30
  out 0x08, r16      ; PORTC4, PORTC5: the pull-ups
  ldi r16, 0xff
  out 0x04, r16      ; DDRB: all outputs
  ldi r16, 2
  sts 0xb8, r16      ; TWBR: an SCL period of 20 cycles
  rcall tw_start
  ldi r16, 0xa2      ; SLA+W
  rcall tw_send
  ldi r16, 0x02      ; the register
  rcall tw_send
  ldi r16, 0x25      ; its value
  rcall tw_send
  rcall tw_start     ; repeated start
  ldi r16, 0xa2
  rcall tw_send
  ldi r16, 0x02
  rcall tw_send
  rcall tw_start     ; repeated start
  ldi r16, 0xa3      ; SLA+R
  rcall tw_send
  ldi r16, 0x84      ; read one byte, no acknowledge
  rcall tw_command
  lds r18, 0xbb      ; TWDR
  rcall tw_stop
  out 0x05, r18      ; PORTB
end:
  rjmp end
tw_start:
  ldi r16, 0xa4      ; TWINT, TWSTA, TWEN
  rjmp tw_command
tw_stop:
  ldi r16, 0x94      ; TWINT, TWSTO, TWEN
  rjmp tw_command
tw_send:
  sts 0xbb, r16      ; TWDR
  ldi r16, 0x84      ; TWINT, TWEN
tw_command:
  sts 0xbc, r16      ; TWCR
tw_wait:
  lds r17, 0xbc
  sbrs r17, 7        ; TWINT
  rjmp tw_wait
  ret
