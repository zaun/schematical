; adc_port (plan-62-E): converts ADC channel 0 (PC0) over and over, left
; adjusted against AVCC, and puts the high 8 bits on PORTB.
  ldi r16, 0xff
  out 0x04, r16      ; DDRB: all outputs
loop:
  ldi r16, 0x60
  sts 0x7c, r16      ; ADMUX: AVCC, ADLAR, channel 0
  ldi r16, 0xc0
  sts 0x7a, r16      ; ADCSRA: ADEN, ADSC
wait:
  lds r17, 0x7a
  sbrc r17, 6        ; ADSC
  rjmp wait
  lds r18, 0x79      ; ADCH
  out 0x05, r18      ; PORTB
  rjmp loop
