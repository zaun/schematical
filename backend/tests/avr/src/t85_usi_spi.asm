; t85_usi_spi: an ATtiny85's USI in three-wire mode as an SPI master (mode
; 0), against a 25-series SPI EEPROM: DO is PB1, DI is PB0, USCK is PB2, and
; PB3 is the chip select. The clock is made by writing USITC (the shift
; register takes DI on USCK's rising edge, the counter counts the strobes).
; WREN, WRITE A1 B2 C3 at 0x0010, RDSR until the write is done, READ the
; three bytes back. PB4 goes high when all three read back right.
  ldi r16, 0x1e
  out 0x17, r16      ; DDRB: PB1 (DO), PB2 (USCK), PB3 (CS), PB4 outputs
  sbi 0x18, 3        ; CS high
  ldi r16, 0x18
  out 0x0d, r16      ; USICR: three-wire mode, so DO is driven before the first clock
  cbi 0x18, 3
  ldi r16, 0x06      ; WREN
  rcall xfer
  sbi 0x18, 3
  cbi 0x18, 3
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
  sbi 0x18, 3
poll:
  cbi 0x18, 3
  ldi r16, 0x05      ; RDSR
  rcall xfer
  ldi r16, 0x00
  rcall xfer
  sbi 0x18, 3
  sbrc r18, 0        ; bit 0: write in progress
  rjmp poll
  cbi 0x18, 3
  ldi r16, 0x03      ; READ
  rcall xfer
  ldi r16, 0x00
  rcall xfer
  ldi r16, 0x10
  rcall xfer
  ldi r16, 0x00
  rcall xfer
  cpi r18, 0xa1
  brne end
  rcall xfer
  cpi r18, 0xb2
  brne end
  rcall xfer
  in r19, 0x10       ; USIBR holds the byte too
  cpi r18, 0xc3
  brne end
  cpi r19, 0xc3
  brne end
  sbi 0x18, 3
  sbi 0x18, 4        ; PB4: all three read back
end:
  rjmp end
xfer:
  out 0x0f, r16      ; USIDR
  ldi r17, 0x40
  out 0x0e, r17      ; USISR: USIOIF cleared, the counter at 0
  ldi r17, 0x1b      ; USIWM0, USICS1, USICLK, USITC
xfer1:
  out 0x0d, r17      ; USICR: one clock edge
  sbis 0x0e, 6       ; USIOIF after 16 edges
  rjmp xfer1
  in r18, 0x0f
  ret
