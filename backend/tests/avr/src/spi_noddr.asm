; spi_noddr (plan-62-F): the SPI master with the DDR bits of SCK and MOSI
; clear: as on the chip, it drives neither pin. The transfer still
; completes; PORTD then shows 0x5a.
  ldi r16, 0x04
  out 0x04, r16      ; DDRB: only PB2 (SS) is an output
  ldi r16, 0xff
  out 0x0a, r16      ; DDRD: all outputs
  ldi r16, 0x50
  out 0x2c, r16      ; SPCR: SPE, MSTR
  ldi r16, 0x99
  out 0x2e, r16      ; SPDR
wait:
  in r17, 0x2d
  sbrs r17, 7        ; SPIF
  rjmp wait
  ldi r16, 0x5a
  out 0x0b, r16
end:
  rjmp end
