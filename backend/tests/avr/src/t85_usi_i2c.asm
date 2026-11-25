; t85_usi_i2c: an ATtiny85's USI in two-wire mode as an I2C master (the
; scheme of Atmel's AVR310), against an I2C real-time clock at address 0x51
; (PCF8563) with pull-up resistors on the bus: SDA is PB0, SCL is PB2.
; Writes 0x25 to register 0x02 (seconds), sets the register address again
; and reads the register back. PB4 goes high when it reads 0x25; PB3 goes
; high when a byte was not acknowledged.
  ldi r16, 0x05
  out 0x18, r16      ; PORTB: SDA and SCL released
  ldi r16, 0x1d
  out 0x17, r16      ; DDRB: SDA, SCL, PB3 and PB4 outputs
  ldi r16, 0xff
  out 0x0f, r16      ; USIDR: SDA released
  ldi r16, 0x2a
  out 0x0d, r16      ; USICR: two-wire, the pin's rising edge shifts, USITC counts
  ldi r16, 0xf0
  out 0x0e, r16      ; USISR: the flags cleared
  rcall i2c_start
  ldi r16, 0xa2      ; SLA+W
  rcall i2c_send
  ldi r16, 0x02      ; the register
  rcall i2c_send
  ldi r16, 0x25      ; its value
  rcall i2c_send
  rcall i2c_stop
  rcall i2c_start
  ldi r16, 0xa2
  rcall i2c_send
  ldi r16, 0x02
  rcall i2c_send
  rcall i2c_stop
  rcall i2c_start
  ldi r16, 0xa3      ; SLA+R
  rcall i2c_send
  cbi 0x17, 0        ; SDA an input
  ldi r17, 0xf0
  rcall transfer     ; 8 bits in
  mov r20, r18
  ldi r16, 0xff
  out 0x0f, r16      ; no acknowledge
  ldi r17, 0xfe
  rcall transfer
  rcall i2c_stop
  andi r20, 0x7f
  cpi r20, 0x25
  brne end
  sbi 0x18, 4        ; PB4: read back
end:
  rjmp end
; A start condition: SDA falls while SCL is high, then SCL falls.
i2c_start:
  sbi 0x18, 2        ; SCL released
  rcall delay
  cbi 0x18, 0        ; SDA low
  rcall delay
  cbi 0x18, 2        ; SCL low
  sbi 0x18, 0        ; SDA released
  ret
; A stop condition: SDA rises while SCL is high.
i2c_stop:
  cbi 0x18, 0        ; SDA low
  sbi 0x18, 2        ; SCL released
  rcall delay
  sbi 0x18, 0        ; SDA released
  rcall delay
  ret
; Sends r16 and reads the acknowledge bit; PB3 goes high when there is none.
i2c_send:
  out 0x0f, r16      ; USIDR
  ldi r17, 0xf0
  rcall transfer     ; 8 bits out
  cbi 0x17, 0        ; SDA an input
  ldi r17, 0xfe
  rcall transfer     ; 1 bit in
  sbrc r18, 0
  sbi 0x18, 3        ; PB3: not acknowledged
  ret
; Clocks the bits the counter value in r17 leaves (0xf0: 8, 0xfe: 1); the
; shift register afterwards in r18. Leaves SDA released and an output.
transfer:
  out 0x0e, r17      ; USISR: flags cleared, the counter set
  ldi r17, 0x2b      ; two-wire, USICS1, USICLK, USITC
transfer1:
  rcall delay
  out 0x0d, r17      ; SCL rises
  rcall delay
  out 0x0d, r17      ; SCL falls
  sbis 0x0e, 6       ; USIOIF
  rjmp transfer1
  rcall delay
  in r18, 0x0f       ; USIDR
  ldi r17, 0xff
  out 0x0f, r17      ; SDA released
  sbi 0x17, 0        ; SDA an output
  ret
delay:
  nop
  nop
  ret
