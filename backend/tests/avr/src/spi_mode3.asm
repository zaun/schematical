; spi_mode3 (plan-62-F): the SPI master in mode 3 (CPOL and CPHA set: SCK
; idles high), LSB-first off, fosc/16, against a 25-series EEPROM: WREN,
; then RDSR; the status (WEL set: 0x02) goes to PORTD.
  ldi r16, 0x2c
  out 0x04, r16      ; DDRB: PB2 (SS), PB3 (MOSI), PB5 (SCK) outputs
  sbi 0x05, 2        ; SS high
  ldi r16, 0xff
  out 0x0a, r16      ; DDRD: all outputs
  ldi r16, 0x5d
  out 0x2c, r16      ; SPCR: SPE, MSTR, CPOL, CPHA, fosc/16
  nop
  nop
  cbi 0x05, 2
  ldi r16, 0x06      ; WREN
  rcall xfer
  sbi 0x05, 2
  cbi 0x05, 2
  ldi r16, 0x05      ; RDSR
  rcall xfer
  ldi r16, 0x00
  rcall xfer
  sbi 0x05, 2
  out 0x0b, r18      ; PORTD: the status
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
