; t25_small: an ATtiny25 (2 KB flash, 128 bytes of SRAM at 0x60-0xDF): the
; data space is smaller than an ATmega's register space. The stack starts
; at 0x00DF; calls, pushes and a Timer1 overflow interrupt use it, and the
; program writes the first and the last byte of the SRAM.
  rjmp start         ; 0x000 RESET
_LOC 0x08
  rjmp isr_ovf       ; 0x004 TIMER1 OVF
start:
  in r20, 0x3d       ; SPL: 0xdf
  in r21, 0x3e       ; SPH: 0
  ldi r16, 0xa5
  sts 0x0060, r16    ; the first byte of the SRAM
  sts 0x00df, r16    ; and the last (the stack will overwrite it)
  lds r17, 0x0060
  ldi r16, 0x3f
  out 0x17, r16      ; DDRB: all six outputs
  ldi r16, 0x04
  out 0x39, r16      ; TIMSK: TOIE1
  ldi r16, 0x02
  out 0x30, r16      ; TCCR1: CK/2
  sei
loop:
  push r22
  rcall deeper
  pop r22
  inc r22
  out 0x18, r24      ; PORTB shows the overflow count
  rjmp loop
deeper:
  push r23
  rcall deepest
  pop r23
  ret
deepest:
  inc r23
  ret
isr_ovf:
  inc r24
  reti
