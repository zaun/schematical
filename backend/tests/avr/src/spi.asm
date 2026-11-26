; spi (plan-62-C): every clock divider with and without SPI2X, a transfer by
; polling SPIF and by the interrupt, the write-collision flag, SPE off.
  jmp start
_LOC 0x44
  jmp isr_spi        ; 0x0022 SPI STC
start:
  ldi r20, 0x00      ; SPI2X off, then on
pass:
  out 0x2d, r20      ; SPSR
  ldi r16, 0x50
  out 0x2c, r16      ; SPCR: SPE, MSTR, fosc/4 (or /2)
  ldi r16, 0x11
  rcall xfer
  ldi r16, 0x51
  out 0x2c, r16      ; /16 (/8)
  ldi r16, 0x22
  rcall xfer
  ldi r16, 0x52
  out 0x2c, r16      ; /64 (/32)
  ldi r16, 0x33
  rcall xfer
  ldi r16, 0x53
  out 0x2c, r16      ; /128 (/64)
  ldi r16, 0x44
  rcall xfer
  inc r20
  cpi r20, 2
  brne pass
; modes and data order do not change the byte-level behavior
  ldi r16, 0x7c
  out 0x2c, r16      ; SPE, DORD, MSTR, CPOL, CPHA
  ldi r16, 0x81
  rcall xfer
; a write during a transfer: WCOL
  ldi r16, 0x53
  out 0x2c, r16
  ldi r16, 0xa0
  out 0x2e, r16      ; SPDR
  ldi r16, 0xa1
  out 0x2e, r16      ; collides
  in r21, 0x2d       ; WCOL set
xw2:
  in r17, 0x2d
  sbrs r17, 7
  rjmp xw2
  in r18, 0x2e
  ldi r16, 0xa2
  out 0x2e, r16      ; the next write clears WCOL
  in r21, 0x2d
xw3:
  in r17, 0x2d
  sbrs r17, 7
  rjmp xw3
  in r18, 0x2e
; by the interrupt
  ldi r16, 0xd0
  out 0x2c, r16      ; SPIE, SPE, MSTR
  sei
  ldi r16, 0x5a
  out 0x2e, r16
wait:
  cpi r24, 3         ; the handler sends two more bytes
  brne wait
  cli
; SPIF clears by writing 1 to SPSR (as avr8js has it)
  ldi r16, 0x50
  out 0x2c, r16
  ldi r16, 0x77
  out 0x2e, r16
xw4:
  in r17, 0x2d
  sbrs r17, 7
  rjmp xw4
  ldi r16, 0x80
  out 0x2d, r16
  in r17, 0x2d
; SPE off: SPDR is plain memory
  ldi r16, 0x10
  out 0x2c, r16
  ldi r16, 0x99
  out 0x2e, r16
  in r18, 0x2e
end:
  inc r23
  rjmp end
xfer:
  out 0x2e, r16
xfer1:
  in r17, 0x2d
  sbrs r17, 7        ; SPIF
  rjmp xfer1
  in r18, 0x2e
  ret
isr_spi:
  in r19, 0x2e
  inc r24
  cpi r24, 3
  breq isr_spi1
  out 0x2e, r19      ; send what came back
isr_spi1:
  reti
