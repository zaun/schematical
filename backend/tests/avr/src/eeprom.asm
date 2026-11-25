; eeprom (plan-62-C): a read of erased memory, write then read back, erase
; only, write only, the EEMPE timeout, a write while busy, the ready
; interrupt, an address past the end.
  jmp start
_LOC 0x58
  jmp isr_ee         ; 0x002c EE READY
start:
  ldi r16, 0x00
  out 0x22, r16      ; EEARH
  ldi r16, 0x10
  out 0x21, r16      ; EEARL: address 0x0010
  sbi 0x1f, 0        ; EERE
  in r18, 0x20       ; EEDR: erased, 0xff
; write 0x5a (erase and write), read it back
  ldi r16, 0x5a
  rcall write
  sbi 0x1f, 0
  in r18, 0x20
; write only (EEPM1): bits go from 1 to 0 only
  ldi r16, 0x0f
  out 0x20, r16
  ldi r16, 0x24      ; EEPM1, EEMPE
  out 0x1f, r16
  ldi r16, 0x22      ; EEPM1, EEPE
  out 0x1f, r16
  rcall busy
  sbi 0x1f, 0
  in r18, 0x20       ; 0x5a and 0x0f = 0x0a
; erase only (EEPM0)
  ldi r16, 0x14      ; EEPM0, EEMPE
  out 0x1f, r16
  ldi r16, 0x12      ; EEPM0, EEPE
  out 0x1f, r16
  rcall busy
  sbi 0x1f, 0
  in r18, 0x20       ; 0xff again
; EEPE more than four cycles after EEMPE does nothing
  ldi r16, 0x33
  out 0x20, r16
  sbi 0x1f, 2        ; EEMPE
  nop
  nop
  nop
  nop
  nop
  nop
  sbi 0x1f, 1        ; EEPE: too late
  in r19, 0x1f
  sbi 0x1f, 0
  in r18, 0x20       ; still 0xff
; a second write while one is in progress is ignored
  ldi r16, 0x77
  out 0x20, r16
  sbi 0x1f, 2
  sbi 0x1f, 1        ; address 0x10 gets 0x77
  ldi r16, 0x11
  out 0x21, r16      ; address 0x11
  ldi r16, 0x88
  out 0x20, r16
  sbi 0x1f, 2
  sbi 0x1f, 1        ; ignored: busy
  rcall busy
  sbi 0x1f, 0
  in r18, 0x20       ; address 0x11: 0xff
  ldi r16, 0x10
  out 0x21, r16
  sbi 0x1f, 0
  in r18, 0x20       ; address 0x10: 0x77
; an address past the end (1024 bytes): reads 0, a write is dropped
  ldi r16, 0x07
  out 0x22, r16
  sbi 0x1f, 0
  in r18, 0x20
  ldi r16, 0x00
  out 0x22, r16
; the ready interrupt: it fires while the EEPROM is ready
  ldi r16, 0x20
  out 0x21, r16
  ldi r16, 0xc3
  out 0x20, r16
  sei
  sbi 0x1f, 2
  sbi 0x1f, 1        ; a write to 0x20
  sbi 0x1f, 3        ; EERIE: the interrupt comes when the write is done
wait:
  cpi r24, 1
  brne wait
  cli
  sbi 0x1f, 0
  in r18, 0x20       ; 0xc3
end:
  inc r23
  rjmp end
write:
  out 0x20, r16
  sbi 0x1f, 2        ; EEMPE
  sbi 0x1f, 1        ; EEPE
busy:
  sbic 0x1f, 1       ; until EEPE clears
  rjmp busy
  ret
isr_ee:
  cbi 0x1f, 3        ; EERIE off, or it would fire for ever
  inc r24
  reti
