; t85_adc: an ATtiny85's ADC on its pins: ADC1 is PB2, ADC3 is PB3. The top
; bit of each conversion (left adjusted, against VCC) goes to PB0 (ADC1) and
; PB1 (ADC3); PB4 goes high when both are done.
  ldi r16, 0x13
  out 0x17, r16      ; DDRB: PB0, PB1 and PB4 outputs
  ldi r16, 0x21
  out 0x07, r16      ; ADMUX: ADLAR, ADC1
  rcall convert
  sbrc r18, 7
  sbi 0x18, 0
  ldi r16, 0x23
  out 0x07, r16      ; ADC3
  rcall convert
  sbrc r18, 7
  sbi 0x18, 1
  sbi 0x18, 4
end:
  rjmp end
convert:
  ldi r16, 0xc0      ; ADEN, ADSC
  out 0x06, r16
convert1:
  sbic 0x06, 6
  rjmp convert1
  in r18, 0x05       ; ADCH
  ret
