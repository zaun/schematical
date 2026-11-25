; t85_periph: an ATtiny85's ADC (ADMUX 0x07, ADCSRA 0x06, ADCL 0x04, ADCH
; 0x05), EEPROM (EECR 0x1c, EEDR 0x1d, EEARL 0x1e, EEARH 0x1f), system clock
; prescaler (CLKPR 0x26) and watchdog (WDTCR 0x21, MCUSR 0x34) at their
; ATtiny addresses, with the ADC, EEPROM ready and watchdog interrupts on
; their one-word vectors. The ADC voltages come by stimuli.
  rjmp start         ; 0x000 RESET
_LOC 0x0c
  rjmp isr_ee        ; 0x006 EE RDY
_LOC 0x10
  rjmp isr_adc       ; 0x008 ADC
_LOC 0x18
  rjmp isr_wdt       ; 0x00c WDT
start:
  in r20, 0x34       ; MCUSR
  inc r21            ; how often the program started
  sbrc r20, 3
  rjmp after_reset
; the single ended channels 0-3 against VCC
  ldi r20, 0
ch_vcc:
  out 0x07, r20      ; ADMUX
  rcall convert
  inc r20
  cpi r20, 4
  brne ch_vcc
; the differential inputs 4-11, the band gap (12), ground (13), the
; reserved input (14) and the temperature sensor (15)
ch_rest:
  out 0x07, r20
  rcall convert
  inc r20
  cpi r20, 16
  brne ch_rest
; left adjusted, the 1.1 V reference, the AREF pin
  ldi r16, 0x21
  out 0x07, r16      ; ADLAR, channel 1
  rcall convert
  ldi r16, 0x82
  out 0x07, r16      ; REFS1: 1.1 V, channel 2
  rcall convert
  ldi r16, 0x43
  out 0x07, r16      ; REFS0: AREF, channel 3
  rcall convert
; with the interrupt
  sei
  ldi r16, 0x01
  out 0x07, r16
  ldi r16, 0xcb      ; ADEN, ADSC, ADIE, clk/8
  out 0x06, r16
wait_adc:
  cpi r24, 1
  brne wait_adc
  cli
  ldi r16, 0x00
  out 0x06, r16
; the EEPROM: read erased, write, read back, the ready interrupt
  ldi r16, 0x01
  out 0x1f, r16      ; EEARH
  ldi r16, 0x23
  out 0x1e, r16      ; EEARL: address 0x0123
  sbi 0x1c, 0        ; EERE
  in r18, 0x1d       ; EEDR: 0xff
  ldi r16, 0x5a
  out 0x1d, r16
  sbi 0x1c, 2        ; EEMPE
  sbi 0x1c, 1        ; EEPE
busy:
  sbic 0x1c, 1
  rjmp busy
  ldi r16, 0x00
  out 0x1d, r16
  sbi 0x1c, 0
  in r18, 0x1d       ; 0x5a
  sei
  sbi 0x1c, 3        ; EERIE: ready now
wait_ee:
  cpi r25, 1
  brne wait_ee
  cbi 0x1c, 3
; the system clock divided by 256, then the watchdog: interrupt, then reset
  ldi r16, 0x80
  out 0x26, r16
  ldi r16, 0x08
  out 0x26, r16      ; CLKPR
  in r19, 0x26
  ldi r16, 0x18
  out 0x21, r16      ; WDTCR: WDCE, WDE
  ldi r16, 0x40
  out 0x21, r16      ; WDIE, 16 ms
idle:
  cpi r26, 2
  brne idle
  cli
  ldi r16, 0x18
  out 0x21, r16
  ldi r16, 0x08
  out 0x21, r16      ; WDE: the next timeout resets the chip
hang:
  inc r23
  rjmp hang
after_reset:
  ldi r16, 0x00
  out 0x34, r16      ; clear WDRF
  ldi r16, 0x18
  out 0x21, r16
  ldi r16, 0x00
  out 0x21, r16      ; watchdog off
  in r19, 0x21
end:
  inc r22
  rjmp end
convert:
  ldi r16, 0xc0      ; ADEN, ADSC, clk/2
  out 0x06, r16
convert1:
  sbic 0x06, 6
  rjmp convert1
  in r28, 0x04       ; ADCL
  in r29, 0x05       ; ADCH
  ret
isr_adc:
  inc r24
  in r28, 0x04
  in r29, 0x05
  reti
isr_ee:
  inc r25
  cbi 0x1c, 3
  reti
isr_wdt:
  inc r26
  reti
