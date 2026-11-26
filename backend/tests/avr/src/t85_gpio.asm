; t85_gpio: an ATtiny85's port B (PINB 0x16, DDRB 0x17, PORTB 0x18): outputs,
; pull-ups, a PINB write that toggles; INT0 (PB2) in all four sense modes
; (MCUCR) and the pin-change interrupt through PCMSK, both enabled in GIMSK
; and flagged in GIFR. One word per vector. The pins move by stimuli.
  rjmp start         ; 0x000 RESET
  rjmp isr_int0      ; 0x001 INT0
  rjmp isr_pcint     ; 0x002 PCINT0
start:
  in r20, 0x3d       ; SPL
  in r21, 0x3e       ; SPH: the stack starts at 0x025f
  ldi r16, 0x03
  out 0x17, r16      ; DDRB: PB0 and PB1 outputs
  ldi r16, 0x11
  out 0x18, r16      ; PORTB: PB0 high, the pull-up of PB4
  ldi r16, 0x02
  out 0x16, r16      ; PINB written: PB1 toggles
  sbi 0x18, 0
  cbi 0x18, 0
  in r17, 0x16       ; PINB
  ldi r16, 0x03
  out 0x35, r16      ; MCUCR: INT0 on the rising edge
  ldi r16, 0x18
  out 0x15, r16      ; PCMSK: PB3 and PB4
  ldi r16, 0x60
  out 0x3b, r16      ; GIMSK: INT0 and PCIE
  sei
phase1:
  inc r20
  cpi r20, 80
  brne phase1
  ldi r16, 0x02
  out 0x35, r16      ; the falling edge
phase2:
  inc r21
  cpi r21, 80
  brne phase2
  ldi r16, 0x01
  out 0x35, r16      ; any change
phase3:
  inc r22
  cpi r22, 80
  brne phase3
  ldi r16, 0x00
  out 0x35, r16      ; the low level
phase4:
  inc r23
  cpi r23, 80
  brne phase4
  cli
  ldi r16, 0x03
  out 0x35, r16      ; rising again, with interrupts off
phase5:
  inc r19
  cpi r19, 80
  brne phase5
  in r17, 0x3a       ; GIFR holds both flags
  ldi r16, 0x60
  out 0x3a, r16      ; a 1 written to each
  in r18, 0x3a
  ldi r16, 0x20
  out 0x3b, r16      ; GIMSK: the pin change alone
  ldi r16, 0x01
  out 0x15, r16      ; PCMSK: PB0, an output, which the program moves
  sei
  sbi 0x18, 0
  cbi 0x18, 0
  ldi r16, 0x00
  out 0x3b, r16      ; GIMSK off
end:
  inc r19
  in r17, 0x16
  rjmp end
isr_int0:
  inc r24
  reti
isr_pcint:
  inc r25
  in r26, 0x16
  reti
