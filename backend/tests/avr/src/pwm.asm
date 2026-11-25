; pwm (plan-62-E): OC0A (PD6) in fast PWM, non-inverting, OCR0A 63: high for
; 64 of every 256 CPU cycles.
  sbi 0x0a, 6        ; DDRD6: output
  ldi r16, 63
  out 0x27, r16      ; OCR0A
  ldi r16, 0x83
  out 0x24, r16      ; TCCR0A: OC0A clears on match, fast PWM
  ldi r16, 0x01
  out 0x25, r16      ; TCCR0B: clk/1
loop:
  rjmp loop
