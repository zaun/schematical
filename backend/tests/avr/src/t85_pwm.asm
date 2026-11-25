; t85_pwm: an ATtiny85's Timer1 in PWM mode as the datasheet has it: the
; counter runs 0..OCR1C (99), so a period is 100 CPU cycles. OC1A (PB1) is
; high for OCR1A = 26 cycles with its inverse on /OC1A (PB0, COM1A = 01);
; OC1B (PB4) is high for OCR1B = 60 cycles (COM1B = 10). The program first
; turns the PLL on and waits for its lock, as firmware for the fast PWM does.
; The widths are even: the timer is brought up to date between instructions,
; and the idle loop's RJMP takes two cycles.
  ldi r16, 0x02
  out 0x27, r16      ; PLLCSR: PLLE
lock:
  in r16, 0x27
  sbrs r16, 0        ; PLOCK
  rjmp lock
  ldi r16, 0x13
  out 0x17, r16      ; DDRB: PB0, PB1 and PB4 outputs
  ldi r16, 99
  out 0x2d, r16      ; OCR1C: TOP
  ldi r16, 26
  out 0x2e, r16      ; OCR1A
  ldi r16, 60
  out 0x2b, r16      ; OCR1B
  ldi r16, 0x60
  out 0x2c, r16      ; GTCCR: PWM1B, COM1B1
  ldi r16, 0x51
  out 0x30, r16      ; TCCR1: PWM1A, COM1A0, CK
loop:
  rjmp loop
