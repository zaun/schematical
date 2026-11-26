; spi_eeprom (plan-62-F): the SPI master against a 25-series SPI EEPROM, mode
; 0, fosc/4. SS is PB2 as a port pin. WREN, WRITE A1 B2 C3 at 0x0010, RDSR
; until the write is done, READ the three bytes back; each goes to PORTD.
  ldi r16, 0x2c
  out 0x04, r16      ; DDRB: PB2 (SS), PB3 (MOSI), PB5 (SCK) outputs
  sbi 0x05, 2        ; SS high
  ldi r16, 0xff
  out 0x0a, r16      ; DDRD: all outputs
  ldi r16, 0x50
  out 0x2c, r16      ; SPCR: SPE, MSTR, mode 0, fosc/4
  cbi 0x05, 2
  ldi r16, 0x06      ; WREN
  rcall xfer
  sbi 0x05, 2
  cbi 0x05, 2
  ldi r16, 0x02      ; WRITE
  rcall xfer
  ldi r16, 0x00
  rcall xfer
  ldi r16, 0x10
  rcall xfer
  ldi r16, 0xa1
  rcall xfer
  ldi r16, 0xb2
  rcall xfer
  ldi r16, 0xc3
  rcall xfer
  sbi 0x05, 2
poll:
  cbi 0x05, 2
  ldi r16, 0x05      ; RDSR
  rcall xfer
  ldi r16, 0x00
  rcall xfer
  sbi 0x05, 2
  sbrc r18, 0        ; bit 0: write in progress
  rjmp poll
  cbi 0x05, 2
  ldi r16, 0x03      ; READ
  rcall xfer
  ldi r16, 0x00
  rcall xfer
  ldi r16, 0x10
  rcall xfer
  ldi r16, 0x00
  rcall xfer
  out 0x0b, r18      ; PORTD: 0xa1
  rcall xfer
  out 0x0b, r18      ; 0xb2
  rcall xfer
  out 0x0b, r18      ; 0xc3
  sbi 0x05, 2
end:
  rjmp end
xfer:
  out 0x2e, r16      ; SPDR
xfer1:
  in r17, 0x2d
  sbrs r17, 7        ; SPIF
  rjmp xfer1
  in r18, 0x2e
  ret
