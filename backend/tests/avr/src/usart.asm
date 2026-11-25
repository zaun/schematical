; usart (plan-62-C): 9600 and 115200 baud settings, U2X, 5- to 9-bit frames,
; stop bits and parity, the UDRE, TXC and RXC interrupts, received bytes (by
; stimuli) read from UDR, TXEN toggled.
  jmp start
_LOC 0x48
  jmp isr_rx         ; 0x0024 USART RX
  jmp isr_udre       ; 0x0026 USART UDRE
  jmp isr_tx         ; 0x0028 USART TX
start:
  ldi r16, 0x00
  sts 0xc5, r16      ; UBRR0H
  ldi r16, 103
  sts 0xc4, r16      ; UBRR0L: 9600 baud at 16 MHz
  ldi r16, 0x18
  sts 0xc1, r16      ; UCSR0B: RXEN, TXEN
  ldi r16, 0x48
  rcall send         ; 'H' at 9600
; faster from here on: 115200 with U2X (UBRR 16)
  ldi r16, 0x02
  sts 0xc0, r16      ; UCSR0A: U2X
  ldi r16, 16
  sts 0xc4, r16
  ldi r16, 0x69
  rcall send         ; 'i'
; 1 Mbaud for the rest (UBRR 1 with U2X)
  ldi r16, 1
  sts 0xc4, r16
; frame formats: 5, 6, 7, 8 bits; 2 stop bits; even and odd parity
  ldi r16, 0x00
  rcall format
  ldi r16, 0x02
  rcall format
  ldi r16, 0x04
  rcall format
  ldi r16, 0x06
  rcall format
  ldi r16, 0x0e
  rcall format
  ldi r16, 0x26
  rcall format
  ldi r16, 0x36
  rcall format
; 9 bits: UCSZ2 in UCSR0B
  ldi r16, 0x1c
  sts 0xc1, r16
  ldi r16, 0x06
  rcall format
  ldi r16, 0x18
  sts 0xc1, r16
; a polled receive: wait for RXC, read UDR (the stimulus at cycle 60000)
recv:
  lds r17, 0xc0
  sbrs r17, 7
  rjmp recv
  lds r18, 0xc6
  lds r17, 0xc0      ; RXC is clear again
; TXC clears by writing 1
  ldi r16, 0x42
  sts 0xc0, r16
  lds r17, 0xc0
; by interrupts: RXCIE, TXCIE, UDRIE
  ldi r20, 3         ; bytes the UDRE interrupt still sends
  ldi r16, 0xf8
  sts 0xc1, r16
  sei
wait:
  inc r23
  cpi r24, 4         ; four bytes echoed by the receive interrupt
  brne wait
  cli
; TXEN off and on again marks UDR empty
  ldi r16, 0x10
  sts 0xc1, r16
  ldi r16, 0x18
  sts 0xc1, r16
  lds r17, 0xc0
; the receiver off: a byte that arrives now is lost
  ldi r16, 0x08
  sts 0xc1, r16
end:
  inc r23
  lds r17, 0xc0
  rjmp end
format:
  sts 0xc2, r16      ; UCSR0C
  ldi r16, 0xff
  rcall send
  ret
send:
  sts 0xc6, r16      ; UDR0
send1:
  lds r17, 0xc0
  sbrs r17, 5        ; UDRE
  rjmp send1
  ret
isr_rx:
  lds r21, 0xc6      ; the received byte
  sts 0xc6, r21      ; echoed
  inc r24
  reti
isr_udre:
  ldi r22, 0x55
  sts 0xc6, r22
  dec r20
  brne isr_udre1
  ldi r22, 0xd8
  sts 0xc1, r22      ; UDRIE off after three bytes
isr_udre1:
  reti
isr_tx:
  inc r25
  reti
