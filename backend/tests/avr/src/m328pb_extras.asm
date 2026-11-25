; m328pb_extras (plan-62-D): what the ATmega328PB has beyond the 328P:
; timers 3 and 4 (CTC and fast PWM, OC3A on PD0 and OC4A on PD1, their
; interrupts), USART1, SPI1, TWI1, and PCINT3 on port E. Two words per
; vector: vector n is at byte 4n.
  jmp start
_LOC 0x6c
  jmp isr_pcint3     ; vector 27 PCINT3
  jmp isr_rx1        ; vector 28 USART1 RX
_LOC 0x78
  jmp isr_tx1        ; vector 30 USART1 TX
_LOC 0x84
  jmp isr_t3a        ; vector 33 TIMER3 COMPA
_LOC 0x8c
  jmp isr_t3ovf      ; vector 35 TIMER3 OVF
_LOC 0x9c
  jmp isr_spi1       ; vector 39 SPI1 STC
  jmp isr_twi1       ; vector 40 TWI1
_LOC 0xa8
  jmp isr_t4a        ; vector 42 TIMER4 COMPA
_LOC 0xb0
  jmp isr_t4ovf      ; vector 44 TIMER4 OVF
start:
  ldi r16, 0x03
  out 0x0a, r16      ; DDRD: PD0 (OC3A) and PD1 (OC4A) outputs
; PCINT3: PE0-PE3
  ldi r16, 0x0f
  sts 0x73, r16      ; PCMSK3
  ldi r16, 0x08
  sts 0x68, r16      ; PCICR: PCIE3
; timer 3: CTC with OCR3A, OC3A toggles, clk/1
  ldi r16, 0x00
  sts 0x99, r16
  ldi r16, 0x90
  sts 0x98, r16      ; OCR3A = 0x0090
  ldi r16, 0x03
  sts 0x71, r16      ; TIMSK3: overflow and compare A
  ldi r16, 0x40
  sts 0x90, r16      ; TCCR3A: OC3A toggles
  ldi r16, 0x09
  sts 0x91, r16      ; TCCR3B: CTC, clk/1
; timer 4: fast PWM 8 bit, OC4A non-inverting, clk/1
  ldi r16, 0x00
  sts 0xa9, r16
  ldi r16, 0x40
  sts 0xa8, r16      ; OCR4A = 0x0040
  ldi r16, 0x03
  sts 0x72, r16      ; TIMSK4
  ldi r16, 0x81
  sts 0xa0, r16      ; TCCR4A: OC4A clear on match, WGM40
  ldi r16, 0x09
  sts 0xa1, r16      ; TCCR4B: WGM42, clk/1
; USART1: 1 Mbaud, receive and transmit interrupts
  ldi r16, 0x00
  sts 0xcd, r16
  ldi r16, 0
  sts 0xcc, r16      ; UBRR1 = 0
  ldi r16, 0xd8
  sts 0xc9, r16      ; UCSR1B: RXCIE, TXCIE, RXEN, TXEN
  ldi r16, 0x31
  sts 0xce, r16      ; UDR1: '1'
; SPI1: master, interrupt
  ldi r16, 0xd0
  sts 0xac, r16      ; SPCR1: SPIE, SPE, MSTR
  ldi r16, 0x3c
  sts 0xae, r16      ; SPDR1
; TWI1: a start, by the interrupt
  ldi r16, 0xa5
  sts 0xdc, r16      ; TWCR1: TWINT, TWSTA, TWEN, TWIE
  sei
  ldi r31, 0
wait:
  dec r31
  brne wait
; timer 3 again, in normal mode, close to its overflow
  ldi r16, 0x01
  sts 0x91, r16      ; TCCR3B: normal, clk/1
  ldi r16, 0xff
  sts 0x95, r16
  ldi r16, 0x00
  sts 0x94, r16      ; TCNT3 = 0xff00
loop:
  inc r23
  rjmp loop
isr_pcint3:
  in r17, 0x0c       ; PINE
  inc r18
  reti
isr_rx1:
  lds r19, 0xce
  sts 0xce, r19      ; echo
  reti
isr_tx1:
  inc r20
  reti
isr_t3a:
  inc r21
  reti
isr_t3ovf:
  inc r22
  reti
isr_spi1:
  lds r24, 0xae      ; what came back
  inc r25
  reti
isr_twi1:
  lds r26, 0xd9      ; TWSR1: the status
  inc r27
  ldi r28, 0x80
  sts 0xdc, r28      ; TWINT cleared, TWEN off
  reti
isr_t4a:
  inc r29
  reti
isr_t4ovf:
  inc r30
  reti
