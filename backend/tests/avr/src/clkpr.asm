; clkpr (plan-62-B): the CLKPR change-enable sequence, a late write that is
; ignored, and each prescaler value 0-15.
  ldi r16, 0x80
  ldi r17, 0x03
  sts 0x61, r16      ; CLKPCE
  sts 0x61, r17      ; within 4 cycles: divide by 8
  lds r18, 0x61
  sts 0x61, r16      ; CLKPCE again
  nop
  nop
  nop
  nop
  nop
  nop
  sts 0x61, r17      ; too late: this is a write with no CLKPCE before it
  lds r19, 0x61
  sts 0x61, r17      ; and again
  lds r19, 0x61
  ldi r20, 0x00
each:
  sts 0x61, r16
  sts 0x61, r20
  lds r21, 0x61
  inc r20
  cpi r20, 16
  brne each
  sts 0x61, r16      ; CLKPCE twice, then the value
  sts 0x61, r16
  sts 0x61, r17
  lds r22, 0x61
done:
  inc r23
  rjmp done
