; m48_vectors (plan-62-D): an ATmega48 (4 KB flash, 512 bytes of SRAM): one
; word per interrupt vector, so each vector is an RJMP. INT0 (vector 1), the
; timer 0 overflow (16) and USART RX (18) must land on their handlers. The
; stack starts at 0x2FF.
  rjmp start         ; 0x000 RESET
  rjmp isr_int0      ; 0x001 INT0
_LOC 0x20
  rjmp isr_ovf0      ; 0x010 TIMER0 OVF
_LOC 0x24
  rjmp isr_rx        ; 0x012 USART RX
start:
  ldi r16, 0xff
  out 0x04, r16      ; DDRB: all outputs
  in r20, 0x3d       ; SPL
  in r21, 0x3e       ; SPH: 0x02ff
  ldi r16, 0x03
  sts 0x69, r16      ; EICRA: INT0 on the rising edge
  ldi r16, 0x01
  out 0x1d, r16      ; EIMSK: INT0
  sts 0x6e, r16      ; TIMSK0: overflow
  out 0x25, r16      ; TCCR0B: clk/1
  ldi r16, 0x00
  sts 0xc5, r16
  ldi r16, 1
  sts 0xc4, r16      ; UBRR0 = 1
  ldi r16, 0x98
  sts 0xc1, r16      ; UCSR0B: RXCIE, RXEN, TXEN
  sei
loop:
  inc r23
  out 0x05, r24      ; PORTB shows the interrupt counts
  rjmp loop
isr_int0:
  inc r24
  reti
isr_ovf0:
  inc r25
  reti
isr_rx:
  lds r22, 0xc6      ; the byte
  sts 0xc6, r22      ; echoed
  inc r24
  reti
