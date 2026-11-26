; pe0 (plan-62-G): toggles PE0, a pin only the PB devices have.
  sbi 0x0d, 0        ; DDRE0: output
loop:
  sbi 0x0c, 0        ; PINE0: toggle PE0
  rjmp loop
