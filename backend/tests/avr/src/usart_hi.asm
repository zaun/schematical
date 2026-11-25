; usart_hi (plan-62-F): 9600 baud 8N1 on the internal 1 MHz clock (U2X, UBRR
; 12: 104 cycles per bit). Sends "HI", then echoes every received byte + 1.
  ldi r16, 0x02
  sts 0xc0, r16      ; UCSR0A: U2X
  ldi r16, 12
  sts 0xc4, r16      ; UBRR0L
  ldi r16, 0x18
  sts 0xc1, r16      ; UCSR0B: RXEN, TXEN
  ldi r16, 0x48
  rcall send         ; 'H'
  ldi r16, 0x49
  rcall send         ; 'I'
echo:
  lds r17, 0xc0
  sbrs r17, 7        ; RXC
  rjmp echo
  lds r16, 0xc6      ; UDR0
  inc r16
  rcall send
  rjmp echo
send:
  sts 0xc6, r16
send1:
  lds r17, 0xc0
  sbrs r17, 5        ; UDRE
  rjmp send1
  ret
