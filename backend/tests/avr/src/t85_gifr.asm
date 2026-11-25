; t85_gifr: an ATtiny85's INT0 flag is cleared by writing 1 to it in GIFR,
; the register it shares with the pin-change flag. With interrupts off the
; program waits for PB2 to rise (INTF0 sets), writes 1 to INTF0, and turns
; interrupts on: the handler must not run. PB1 goes high at the end; PB0
; goes high only if the handler ran. Then PB2 falls and rises again with
; interrupts on: PB4 goes high from the handler's second entry point.
  rjmp start
  rjmp isr           ; vector 1: INT0
start:
  ldi r16, 0x13
  out 0x17, r16      ; DDRB: PB0, PB1 and PB4 outputs
  ldi r16, 0x03
  out 0x35, r16      ; MCUCR: INT0 on the rising edge
  ldi r16, 0x40
  out 0x3b, r16      ; GIMSK: INT0
wait_high:
  sbis 0x16, 2
  rjmp wait_high
  in r17, 0x3a       ; GIFR: INTF0 is set
  ldi r16, 0x40
  out 0x3a, r16      ; a 1 written to INTF0
  sei
  nop
  nop
  sbi 0x18, 1        ; PB1: this far
  ldi r20, 1         ; from now on the handler is expected
loop:
  rjmp loop
isr:
  cpi r20, 1
  breq expected
  sbi 0x18, 0        ; PB0: the flag was not cleared
  reti
expected:
  sbi 0x18, 4        ; PB4
  reti
