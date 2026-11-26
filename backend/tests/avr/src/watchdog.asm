; watchdog (plan-62-C): the WDTCSR change-enable sequence, WDR holding the
; watchdog off, the interrupt mode, then the reset mode: the program
; restarts and reads WDRF. The system clock is divided by 256 first so a
; timeout is 1000 cycles.
  jmp start
_LOC 0x18
  jmp isr_wdt        ; 0x000c WDT
start:
  in r20, 0x34       ; MCUSR: WDRF after a watchdog reset
  inc r21            ; how often the program started
  ldi r16, 0x80
  sts 0x61, r16
  ldi r16, 0x08
  sts 0x61, r16      ; CLKPR: divide by 256
  sbrc r20, 3
  rjmp after_reset
; a prescaler write without the change-enable sequence is ignored
  ldi r16, 0x07
  sts 0x60, r16
  lds r19, 0x60
; the interrupt mode
  ldi r16, 0x18
  sts 0x60, r16      ; WDCE, WDE
  ldi r16, 0x40
  sts 0x60, r16      ; WDIE, 16 ms
  sei
  ldi r22, 200
kick:                ; WDR every few cycles: no timeout
  wdr
  dec r22
  brne kick
idle:                ; no WDR: the interrupt fires every 1000 cycles
  cpi r24, 2
  brne idle
  cli
; interrupt and reset together: the first timeout interrupts and clears WDIE
  ldi r16, 0x18
  sts 0x60, r16
  ldi r16, 0x48
  sts 0x60, r16      ; WDIE, WDE
  sei
idle2:
  cpi r24, 3
  brne idle2
  lds r19, 0x60      ; WDIE is clear, WDE set
; the reset mode with a longer timeout (32 ms: 2000 cycles), held off by WDR
  ldi r16, 0x18
  sts 0x60, r16
  ldi r16, 0x09
  sts 0x60, r16      ; WDE, WDP0
  ldi r22, 0
kick2:
  wdr
  dec r22
  brne kick2
hang:                ; no WDR: the watchdog resets the chip
  inc r23
  rjmp hang
after_reset:
  ldi r16, 0x00
  out 0x34, r16      ; clear WDRF
  ldi r16, 0x18
  sts 0x60, r16
  ldi r16, 0x00
  sts 0x60, r16      ; watchdog off
  lds r19, 0x60
end:
  inc r25
  rjmp end
isr_wdt:
  inc r24
  reti
