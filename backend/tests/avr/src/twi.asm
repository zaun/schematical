; twi (plan-62-C): the TWI master: start, address + W with and without an
; acknowledge, data writes, a repeated start, address + R, reads with ack
; then nack, stop; by polling and by the interrupt; each prescaler. The
; scripted bus acknowledges address 0x50 only.
  jmp start
_LOC 0x60
  jmp isr_twi        ; 0x0030 TWI
start:
  ldi r16, 12
  sts 0xb8, r16      ; TWBR
; write two bytes to 0x50, repeated start, read two bytes, stop
  rcall tw_start
  ldi r16, 0xa0      ; SLA+W
  rcall tw_send
  ldi r16, 0x12
  rcall tw_send
  ldi r16, 0x34
  rcall tw_send
  rcall tw_start     ; repeated
  ldi r16, 0xa1      ; SLA+R
  rcall tw_send
  ldi r16, 0xc4      ; TWINT, TWEA, TWEN: read, acknowledge
  rcall tw_command
  lds r18, 0xbb      ; TWDR
  ldi r16, 0x84      ; read, no acknowledge
  rcall tw_command
  lds r18, 0xbb
  rcall tw_stop
; an address nobody answers
  rcall tw_start
  ldi r16, 0x60      ; SLA+W for 0x30
  rcall tw_send
  rcall tw_stop
  rcall tw_start
  ldi r16, 0x61      ; SLA+R for 0x30
  rcall tw_send
  rcall tw_stop
; each prescaler (TWPS in TWSR)
  ldi r16, 0x01
  sts 0xb9, r16
  rcall tw_start
  rcall tw_stop
  ldi r16, 0x02
  sts 0xb9, r16
  rcall tw_start
  rcall tw_stop
  ldi r16, 0x03
  sts 0xb9, r16
  rcall tw_start
  rcall tw_stop
  ldi r16, 0x00
  sts 0xb9, r16
; TWEN off: a command does nothing
  ldi r16, 0xa0
  sts 0xbc, r16
  lds r19, 0xb9
; by the interrupt: the handler notes the status and clears TWINT
  sei
  ldi r16, 0xa5      ; TWINT, TWSTA, TWEN, TWIE
  sts 0xbc, r16
  rcall tw_irq
  ldi r16, 0xa0
  sts 0xbb, r16
  ldi r16, 0x85      ; TWINT, TWEN, TWIE
  sts 0xbc, r16
  rcall tw_irq
  ldi r16, 0x77
  sts 0xbb, r16
  ldi r16, 0x85
  sts 0xbc, r16
  rcall tw_irq
  ldi r16, 0x95      ; TWINT, TWSTO, TWEN, TWIE
  sts 0xbc, r16
  rcall tw_irq
  cli
end:
  inc r23
  rjmp end
tw_start:
  ldi r16, 0xa4      ; TWINT, TWSTA, TWEN
  rjmp tw_command
tw_stop:
  ldi r16, 0x94      ; TWINT, TWSTO, TWEN
  rjmp tw_command
tw_send:
  sts 0xbb, r16      ; TWDR
  ldi r16, 0x84      ; TWINT, TWEN
tw_command:
  sts 0xbc, r16      ; TWCR
tw_wait:
  lds r17, 0xbc
  sbrs r17, 7        ; TWINT
  rjmp tw_wait
  lds r19, 0xb9      ; TWSR: the status
  ret
tw_irq:
  mov r22, r24
tw_irq1:
  cp r22, r24
  breq tw_irq1       ; until the handler ran
  ret
isr_twi:
  lds r25, 0xb9      ; the status
  inc r24
  ldi r26, 0x80
  sts 0xbc, r26      ; TWINT cleared, TWEN off: no new command
  reti
