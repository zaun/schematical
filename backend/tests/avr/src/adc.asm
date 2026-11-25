; adc (plan-62-C): a single conversion on each channel 0-7 (voltages by
; stimuli), the temperature, 1.1 V and 0 V inputs, ADLAR, the AREF, AVCC and
; internal references, each prescaler, the interrupt, ADSC without ADEN.
  jmp start
_LOC 0x54
  jmp isr_adc        ; 0x002a ADC
start:
; channels 0-7 against AVCC, clk/2
  ldi r20, 0
ch_avcc:
  ldi r16, 0x40      ; REFS0: AVCC
  or r16, r20
  sts 0x7c, r16      ; ADMUX
  ldi r16, 0xc0      ; ADEN, ADSC
  rcall convert
  inc r20
  cpi r20, 8
  brne ch_avcc
; the temperature sensor (8), the 1.1 V band gap (14), ground (15), an
; input that does not exist (9)
  ldi r16, 0x48
  sts 0x7c, r16
  ldi r16, 0xc0
  rcall convert
  ldi r16, 0x4e
  sts 0x7c, r16
  ldi r16, 0xc0
  rcall convert
  ldi r16, 0x4f
  sts 0x7c, r16
  ldi r16, 0xc0
  rcall convert
  ldi r16, 0x49
  sts 0x7c, r16
  ldi r16, 0xc0
  rcall convert
; left adjusted (ADLAR), channels 0-7
  ldi r20, 0
ch_adlar:
  ldi r16, 0x60
  or r16, r20
  sts 0x7c, r16
  ldi r16, 0xc0
  rcall convert
  inc r20
  cpi r20, 8
  brne ch_adlar
; the internal 1.1 V reference, the AREF pin, the reserved setting
  ldi r20, 0
ch_refs:
  ldi r16, 0xc0
  or r16, r20
  sts 0x7c, r16
  ldi r16, 0xc0
  rcall convert
  ldi r16, 0x00
  or r16, r20
  sts 0x7c, r16
  ldi r16, 0xc0
  rcall convert
  ldi r16, 0x80
  or r16, r20
  sts 0x7c, r16
  ldi r16, 0xc0
  rcall convert
  inc r20
  cpi r20, 8
  brne ch_refs
; each prescaler on channel 3
  ldi r16, 0x43
  sts 0x7c, r16
  ldi r20, 0
each_ps:
  ldi r16, 0xc0
  or r16, r20
  rcall convert
  inc r20
  cpi r20, 8
  brne each_ps
; ADEN off and on again: the first conversion is the long one
  ldi r16, 0x00
  sts 0x7a, r16
  ldi r16, 0xc0
  rcall convert
; ADSC without ADEN completes with 0
  ldi r16, 0x00
  sts 0x7a, r16
  ldi r16, 0x40
  rcall convert
; by the interrupt, channel 5
  ldi r16, 0x45
  sts 0x7c, r16
  sei
  ldi r16, 0xc8      ; ADEN, ADSC, ADIE
  sts 0x7a, r16
wait:
  cpi r24, 3         ; the handler starts two more
  brne wait
  cli
end:
  inc r23
  rjmp end
convert:
  sts 0x7a, r16      ; ADCSRA
convert1:
  lds r17, 0x7a
  sbrc r17, 6        ; ADSC
  rjmp convert1
  lds r18, 0x78      ; ADCL
  lds r19, 0x79      ; ADCH
  ret
isr_adc:
  lds r21, 0x78
  lds r22, 0x79
  inc r24
  cpi r24, 3
  breq isr_adc1
  ldi r25, 0xc8
  sts 0x7a, r25      ; the next conversion
  reti
isr_adc1:
  ldi r25, 0x80
  sts 0x7a, r25      ; ADIE off
  reti
