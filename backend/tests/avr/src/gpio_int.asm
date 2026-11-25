; gpio_int (plan-62-B): INT0 and INT1 in all four sense modes, the low-level
; interrupt held and released, EIFR and PCIFR cleared by writing 1, PCINT0-2
; through their PCMSK masks. The pins move by stimuli.
  jmp start          ; 0x0000 RESET
  jmp isr_int0       ; 0x0002 INT0
  jmp isr_int1       ; 0x0004 INT1
  jmp isr_pc0        ; 0x0006 PCINT0
  jmp isr_pc1        ; 0x0008 PCINT1
  jmp isr_pc2        ; 0x000a PCINT2
start:
  ldi r16, 0x0f
  sts 0x69, r16      ; EICRA: INT0 and INT1 on the rising edge
  ldi r16, 0x03
  out 0x1d, r16      ; EIMSK: both
  ldi r16, 0x07
  sts 0x68, r16      ; PCICR: all three
  ldi r16, 0x01
  sts 0x6b, r16      ; PCMSK0: PB0
  ldi r16, 0x02
  sts 0x6c, r16      ; PCMSK1: PC1
  ldi r16, 0x80
  sts 0x6d, r16      ; PCMSK2: PD7
  sei
phase1:
  inc r20
  cpi r20, 80
  brne phase1
  ldi r16, 0x0a
  sts 0x69, r16      ; both on the falling edge
phase2:
  inc r21
  cpi r21, 80
  brne phase2
  ldi r16, 0x05
  sts 0x69, r16      ; both on any change
phase3:
  inc r22
  cpi r22, 80
  brne phase3
  ldi r16, 0x00
  sts 0x69, r16      ; both on the low level
phase4:
  inc r23
  cpi r23, 80
  brne phase4
  cli
  ldi r16, 0x0e
  sts 0x69, r16      ; INT0 falling, INT1 rising, with interrupts off
phase5:
  inc r19
  cpi r19, 80
  brne phase5
  in r17, 0x1c       ; EIFR holds the flags
  in r18, 0x1b       ; PCIFR too
  ldi r16, 0x03
  out 0x1c, r16      ; cleared by writing 1
  ldi r16, 0x07
  out 0x1b, r16
  in r17, 0x1c
  in r18, 0x1b
  ldi r16, 0x00
  out 0x1d, r16      ; EIMSK off
  sts 0x6b, r16      ; PCMSK0 off
  sei
end:
  inc r19
  rjmp end
isr_int0:
  inc r24
  reti
isr_int1:
  inc r25
  reti
isr_pc0:
  inc r26
  reti
isr_pc1:
  inc r27
  reti
isr_pc2:
  inc r28
  reti
