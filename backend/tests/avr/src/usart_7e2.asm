; usart_7e2 (plan-62-F): one frame of 7 data bits, even parity and 2 stop
; bits at 9600 baud (internal 1 MHz clock): the byte 0x55.
  ldi r16, 0x02
  sts 0xc0, r16      ; UCSR0A: U2X
  ldi r16, 12
  sts 0xc4, r16      ; UBRR0L
  ldi r16, 0x2c
  sts 0xc2, r16      ; UCSR0C: even parity, 2 stop bits, 7 bits
  ldi r16, 0x08
  sts 0xc1, r16      ; UCSR0B: TXEN
  ldi r16, 0x55
  sts 0xc6, r16
end:
  rjmp end
