---
{
  "format": "schematical-datasheet",
  "version": 1,
  "id": "atmega328p",
  "title": "ATmega48A/PA/88A/PA/168A/PA/328/P 8-bit AVR microcontrollers with 4/8/16/32 KB in-system programmable flash",
  "class": "microcontroller",
  "source": {
    "pdf": "mcu/atmega328p.pdf",
    "vendor": "Microchip Technology",
    "document": "DS40002061B",
    "date": "2020-08",
    "pages": 653
  },
  "devices": [],
  "parts": [],
  "pending": "AVR microcontrollers are not in the catalog yet",
  "converted": "2026-11-07"
}
---

# ATmega48A/PA/88A/PA/168A/PA/328/P 8-bit AVR microcontrollers with 4/8/16/32 KB in-system programmable flash

## Description

Eight low-power CMOS 8-bit AVR microcontrollers that differ only in
memory sizes, boot loader support and interrupt vector size: 4, 8, 16 or
32 Kbytes of in-system self-programmable flash, 256, 512, 512 or 1024
bytes of EEPROM and 512, 1024, 1024 or 2048 bytes of SRAM (ATmega48A/PA,
88A/PA, 168A/PA, 328/P). The AVR RISC core executes most of its 131
instructions in one clock cycle from 32 general purpose registers, up to
20 MIPS at 20 MHz, with a 2-cycle hardware multiplier.

On chip: 23 general purpose I/O lines, two 8-bit and one 16-bit
timer/counter with compare modes (Timer/Counter2 can run asynchronously
from a 32 kHz crystal as a real-time counter), six PWM channels, a USART
(also usable as SPI master), an SPI port, a byte-oriented 2-wire serial
interface (I²C compatible), a 6-channel 10-bit ADC (8 channels in the
32-pin packages) with temperature sensor, an analog comparator, a
watchdog timer with its own oscillator, power-on reset, programmable
brown-out detection, a calibrated internal RC oscillator, external and
pin change interrupts, debugWIRE on-chip debug, and software
selectable sleep modes (Idle, ADC Noise Reduction, Power-save,
Power-down, Standby, plus Extended Standby).

Except in ATmega48A/PA, the flash has a separate boot loader section
with its own lock bits and true read-while-write self-programming.
Program and data memories can be programmed in-system over SPI, by a
parallel programmer, or by the on-chip boot program. Flash endurance
10,000 and EEPROM endurance 100,000 write/erase cycles.

## Pinout

```table id=pin-table caption="Pin assignments (top view numbering)"
| Pin | 32 TQFP (32A), 32 VQFN/MLF (32M1-A) | 28 VQFN/MLF (28M1) | 28 PDIP (28P3) |
|---|--:|--:|--:|
| PD3 | 1 | 1 | 5 |
| PD4 | 2 | 2 | 6 |
| GND | 3, 5, 21 | 4, 18 | 8, 22 |
| VCC | 4, 6 | 3 | 7 |
| PB6 | 7 | 5 | 9 |
| PB7 | 8 | 6 | 10 |
| PD5 | 9 | 7 | 11 |
| PD6 | 10 | 8 | 12 |
| PD7 | 11 | 9 | 13 |
| PB0 | 12 | 10 | 14 |
| PB1 | 13 | 11 | 15 |
| PB2 | 14 | 12 | 16 |
| PB3 | 15 | 13 | 17 |
| PB4 | 16 | 14 | 18 |
| PB5 | 17 | 15 | 19 |
| AVCC | 18 | 16 | 20 |
| ADC6 | 19 | | |
| AREF | 20 | 17 | 21 |
| ADC7 | 22 | | |
| PC0 | 23 | 19 | 23 |
| PC1 | 24 | 20 | 24 |
| PC2 | 25 | 21 | 25 |
| PC3 | 26 | 22 | 26 |
| PC4 | 27 | 23 | 27 |
| PC5 | 28 | 24 | 28 |
| PC6 | 29 | 25 | 1 |
| PD0 | 30 | 26 | 2 |
| PD1 | 31 | 27 | 3 |
| PD2 | 32 | 28 | 4 |
| Exposed pad [1] | 32M1-A only | yes | |

[1] Bottom pad of the VQFN/MLF packages; solder it to ground.
```

## Pin functions

```pins
VCC: Digital supply voltage
GND: Ground
PB0: Port B bit 0; PCINT0, CLKO (system clock output), ICP1 (Timer/Counter1 input capture)
PB1: Port B bit 1; PCINT1, OC1A
PB2: Port B bit 2; PCINT2, SS (SPI slave select), OC1B
PB3: Port B bit 3; PCINT3, MOSI, OC2A
PB4: Port B bit 4; PCINT4, MISO
PB5: Port B bit 5; PCINT5, SCK
PB6: Port B bit 6; PCINT6. Depending on the clock fuses, XTAL1: input to the inverting oscillator amplifier and to the internal clock circuit; TOSC1 when the internal RC oscillator clocks the chip and AS2 (ASSR) = 1
PB7: Port B bit 7; PCINT7. Depending on the clock fuses, XTAL2: output of the inverting oscillator amplifier; TOSC2 when the internal RC oscillator clocks the chip and AS2 = 1
PC0, PC1, PC2, PC3: Port C bits 0–3; ADC0–ADC3, PCINT8–PCINT11. Supplied from AVCC.
PC4: Port C bit 4; ADC4, SDA (2-wire data), PCINT12
PC5: Port C bit 5; ADC5, SCL (2-wire clock), PCINT13
PC6: RESET input, active low, while the RSTDISBL fuse is unprogrammed: a low level longer than the minimum pulse length resets the part, even without a clock; shorter pulses are not guaranteed to. With RSTDISBL programmed, I/O pin PC6 (electrical characteristics differ from the other Port C pins); PCINT14.
PD0: Port D bit 0; RXD (USART input), PCINT16
PD1: Port D bit 1; TXD (USART output), PCINT17
PD2: Port D bit 2; INT0, PCINT18
PD3: Port D bit 3; INT1, OC2B, PCINT19
PD4: Port D bit 4; XCK (USART external clock), T0 (Timer/Counter0 external clock), PCINT20
PD5: Port D bit 5; T1 (Timer/Counter1 external clock), OC0B, PCINT21
PD6: Port D bit 6; AIN0 (comparator positive input), OC0A, PCINT22
PD7: Port D bit 7; AIN1 (comparator negative input), PCINT23
AVCC: Supply for the ADC, PC3:0 and ADC7:6 (PC6:4 use VCC). Connect to VCC even if the ADC is unused; through a low-pass filter when it is used.
AREF: Analog reference for the ADC
ADC6, ADC7: Analog-only ADC inputs (32-pin packages only), powered from the analog supply
```

Ports B, C (PC5:0) and D are bidirectional I/O ports with a pull-up
resistor selectable per bit and symmetrical drive with high sink and
source capability. An input with its pull-up on sources current when
pulled low externally. All port pins are tri-stated while a reset
condition is active, even without a clock. Alternate functions are
detailed under [I/O ports](#io-ports).

## Functional description

Block diagram: the AVR CPU with flash, SRAM and EEPROM on an 8-bit data
bus with the watchdog timer and oscillator, power supervision (POR/BOD
and reset), oscillator circuits and clock generation, debugWIRE and
programming logic, Timer/Counters 0, 1 and 2, the USART, SPI, TWI,
analog comparator, A/D converter with internal bandgap reference, and
ports B (8), C (7) and D (8).

```table id=devices caption="Device comparison"
| Device | Flash | EEPROM | SRAM | Interrupt vector size | Boot loader section, read-while-write |
|---|---|---|---|---|---|
| ATmega48A, ATmega48PA | 4 Kbytes | 256 bytes | 512 bytes | 1 instruction word | No; SPM executes from the whole flash |
| ATmega88A, ATmega88PA | 8 Kbytes | 512 bytes | 1 Kbytes | 1 instruction word | Yes; SPM executes only from the boot section |
| ATmega168A, ATmega168PA | 16 Kbytes | 512 bytes | 1 Kbytes | 2 instruction words | Yes; SPM executes only from the boot section |
| ATmega328, ATmega328P | 32 Kbytes | 1 Kbytes | 2 Kbytes | 2 instruction words | Yes; SPM executes only from the boot section |
```

Sleep modes: Idle stops the CPU while SRAM, timer/counters, USART, TWI,
SPI and the interrupt system keep running; Power-down keeps register
contents but stops the oscillator and all other functions until the
next interrupt or hardware reset; Power-save keeps the asynchronous
timer running; ADC Noise Reduction stops the CPU and all I/O modules
except the asynchronous timer and the ADC; Standby keeps the crystal or
resonator oscillator running for fast start-up.

Data retention: projected failure rate much less than 1 PPM over 20
years at 85 °C or 100 years at 25 °C.

Registers in the extended I/O space (0x60–0xFF) can't be reached with
IN, OUT, SBIS, SBIC, CBI and SBI; use LDS/STS with SBRS, SBRC, SBR and
CBR. Capacitive touch sensing (QTouch, QMatrix) is available as a
Microchip software library.

### Errata

This data sheet lists no errata. They are in Microchip's separate
document *ATmega48A/PA/88A/PA/168A/PA/328/P Silicon Errata and Data
Sheet Clarification* (DS80000855).

## Programming model

Harvard architecture with separate program and data buses and a
one-level pipeline: the next instruction is fetched while one executes,
so most instructions complete in one clk~CPU~ cycle (up to 1 MIPS per
MHz; no internal clock division). In one cycle the ALU reads two
register operands, executes and writes the result back. The ALU does
arithmetic, logic and bit functions between registers or between a
register and a constant, plus signed/unsigned and fractional
multiplication. Most instructions are one 16-bit word; every program
memory address holds a 16- or 32-bit instruction.

### SREG – Status register, 0x3F (0x5F)

Updated after every ALU operation. SREG is not saved on interrupt entry
nor restored on return; software must do it.

```table id=sreg caption="SREG bits"
| Bit | Name | Description |
|---|---|---|
| 7 | I | Global interrupt enable. 0 = all interrupts disabled regardless of their enable bits. Cleared by hardware when an interrupt is taken, set by RETI; set and cleared by SEI and CLI. |
| 6 | T | Bit copy storage: source (BST) or destination (BLD) of the bit copy instructions |
| 5 | H | Half carry flag; useful in BCD arithmetic |
| 4 | S | Sign bit, S = N ⊕ V |
| 3 | V | Two's complement overflow flag |
| 2 | N | Negative flag |
| 1 | Z | Zero flag |
| 0 | C | Carry flag |
```

```wavedrom id=sreg-reg caption="SREG"
{"reg": [
  {"name": "C", "bits": 1}, {"name": "Z", "bits": 1}, {"name": "N", "bits": 1}, {"name": "V", "bits": 1},
  {"name": "S", "bits": 1}, {"name": "H", "bits": 1}, {"name": "T", "bits": 1}, {"name": "I", "bits": 1}
 ]}
```

### General purpose register file

32 × 8-bit registers R0–R31, mapped to data addresses 0x00–0x1F (not
physical SRAM, but X, Y and Z can point at any of them). The file
supports one 8-bit output operand and one 8-bit result, two 8-bit
operands and one 8-bit result, two 8-bit operands and one 16-bit
result, and one 16-bit operand and one 16-bit result. Most instructions
on the file reach all registers and take one cycle.

R26–R31 pair up as the 16-bit indirect address pointers for the data
space, used with fixed displacement, automatic increment and automatic
decrement; Z also points into program memory (LPM, SPM, IJMP, ICALL).

```table id=xyz caption="Pointer registers"
| Pointer | High byte | Low byte |
|---|---|---|
| X | XH = R27 (0x1B) | XL = R26 (0x1A) |
| Y | YH = R29 (0x1D) | YL = R28 (0x1C) |
| Z | ZH = R31 (0x1F) | ZL = R30 (0x1E) |
```

### SPH and SPL – Stack pointer, 0x3E (0x5E) and 0x3D (0x5D)

The stack lives in data SRAM and grows down; SP points to the top of
the stack. Define the stack before any call or interrupt; SP must point
above the start of SRAM (0x100).

```table id=sp caption="SPH:SPL bits"
| Bit | Name | Description |
|---|---|---|
| 15:8 | SP[15:8] | Stack pointer high byte (SPH) |
| 7:0 | SP[7:0] | Stack pointer low byte (SPL) |
```

RAMEND, the last SRAM address and SP's reset value: 0x02FF (ATmega48A/PA),
0x04FF (ATmega88A/PA, ATmega168A/PA), 0x08FF (ATmega328/P).

```table id=stack-ops caption="Stack pointer instructions"
| Instruction | Stack pointer | Description |
|---|---|---|
| PUSH | Decremented by 1 | Data pushed onto the stack |
| CALL, ICALL, RCALL | Decremented by 2 | Return address pushed with a subroutine call or interrupt |
| POP | Incremented by 1 | Data popped from the stack |
| RET, RETI | Incremented by 2 | Return address popped on return from subroutine or interrupt |
```

### Reset and interrupt handling

The reset and each interrupt have their own program vector at the
bottom of program memory (see [Memory map](#memory-map)); a lower
vector address has higher priority (RESET first, then INT0). Each
interrupt has an enable bit that works only with SREG's I bit set.
Interrupts can be disabled automatically, depending on the program
counter, when boot lock bits BLB02 or BLB12 are programmed. Except in
ATmega48A/PA, IVSEL (MCUCR) moves the interrupt vectors, and the
BOOTRST fuse the reset vector, to the start of the boot flash section.

- Taking an interrupt clears I; software may set I to allow nested
  interrupts. RETI sets I.
- Flag interrupts: the event sets an interrupt flag; vectoring clears it
  in hardware, and writing 1 to the flag also clears it. A flag set
  while its enable (or I) is cleared stays pending until enabled or
  cleared; pending interrupts are then served by priority.
- Level interrupts trigger while the condition lasts and need no flag;
  a condition that disappears before the interrupt is enabled is lost.
- After an interrupt returns, at least one main-program instruction
  runs before any pending interrupt is served.
- CLI takes effect at once: no interrupt executes after it, even one
  occurring in the same cycle (use it around timed sequences such as
  the EEPROM write).
- The instruction after SEI always executes before a pending interrupt
  (so SEI followed by SLEEP enters sleep before the interrupt is
  served).

Response time: at least four clock cycles, during which the program
counter is pushed; then the vector (normally a 3-cycle jump) executes.
An interrupt arriving during a multi-cycle instruction waits for it to
complete. In sleep, the response time grows by four cycles plus the
wake-up start-up time. Return (RETI) takes four cycles: PC popped, SP
incremented by two, I set.

## Instruction set

```table id=addr-modes caption="Addressing modes"
| Mode | Syntax | Operand | Effective address |
|---|---|---|---|
| Register direct, one register | Rd | Rd | Register d (0–31) |
| Register direct, two registers | Rd, Rr | Rd, Rr | Registers d and r; result in Rd |
| Immediate | Rd, K | 8-bit constant K | — (Rd is R16–R31 for LDI, SUBI, SBCI, CPI, ANDI, ORI, SBR, CBR, SER) |
| I/O direct | P | I/O address 0–63 | I/O register P (data address P + 0x20); SBI, CBI, SBIC, SBIS reach 0–31 only |
| Data direct | k | 16-bit address k | k, anywhere in the data space (LDS, STS) |
| Data indirect | X, Y, Z | Pointer register | X, Y or Z |
| Data indirect with displacement | Y+q, Z+q | Pointer, q = 0–63 | Y + q or Z + q |
| Data indirect with pre-decrement | -X, -Y, -Z | Pointer | Pointer decremented, then used |
| Data indirect with post-increment | X+, Y+, Z+ | Pointer | Pointer used, then incremented |
| Program memory constant | Z, Z+ | Z | Byte Z of program memory (LPM); SPM writes R1:R0 at Z |
| Relative | k | Offset k | PC + k + 1 (RJMP, RCALL, branches) |
| Indirect program | — | Z | PC ← Z (IJMP, ICALL) |
| Direct program | k | Address k | PC ← k (JMP, CALL) |
```

```table id=isa caption="Instruction set summary"
| Mnemonic | Operands | Operation | Flags | Words | Cycles |
|---|---|---|---|--:|--:|
| Arithmetic and logic | < | < | < | < | < |
| ADD | Rd, Rr | Add two registers: Rd ← Rd + Rr | Z,C,N,V,H | 1 | 1 |
| ADC | Rd, Rr | Add with carry: Rd ← Rd + Rr + C | Z,C,N,V,H | 1 | 1 |
| ADIW | Rdl, K | Add immediate to word: Rdh:Rdl ← Rdh:Rdl + K | Z,C,N,V,S | 1 | 2 |
| SUB | Rd, Rr | Subtract two registers: Rd ← Rd − Rr | Z,C,N,V,H | 1 | 1 |
| SUBI | Rd, K | Subtract constant: Rd ← Rd − K | Z,C,N,V,H | 1 | 1 |
| SBC | Rd, Rr | Subtract with carry: Rd ← Rd − Rr − C | Z,C,N,V,H | 1 | 1 |
| SBCI | Rd, K | Subtract constant with carry: Rd ← Rd − K − C | Z,C,N,V,H | 1 | 1 |
| SBIW | Rdl, K | Subtract immediate from word: Rdh:Rdl ← Rdh:Rdl − K | Z,C,N,V,S | 1 | 2 |
| AND | Rd, Rr | Logical AND: Rd ← Rd • Rr | Z,N,V | 1 | 1 |
| ANDI | Rd, K | Logical AND with constant: Rd ← Rd • K | Z,N,V | 1 | 1 |
| OR | Rd, Rr | Logical OR: Rd ← Rd ∨ Rr | Z,N,V | 1 | 1 |
| ORI | Rd, K | Logical OR with constant: Rd ← Rd ∨ K | Z,N,V | 1 | 1 |
| EOR | Rd, Rr | Exclusive OR: Rd ← Rd ⊕ Rr | Z,N,V | 1 | 1 |
| COM | Rd | One's complement: Rd ← 0xFF − Rd | Z,C,N,V | 1 | 1 |
| NEG | Rd | Two's complement: Rd ← 0x00 − Rd | Z,C,N,V,H | 1 | 1 |
| SBR | Rd, K | Set bit(s): Rd ← Rd ∨ K | Z,N,V | 1 | 1 |
| CBR | Rd, K | Clear bit(s): Rd ← Rd • (0xFF − K) | Z,N,V | 1 | 1 |
| INC | Rd | Increment: Rd ← Rd + 1 | Z,N,V | 1 | 1 |
| DEC | Rd | Decrement: Rd ← Rd − 1 | Z,N,V | 1 | 1 |
| TST | Rd | Test for zero or minus: Rd ← Rd • Rd | Z,N,V | 1 | 1 |
| CLR | Rd | Clear register: Rd ← Rd ⊕ Rd | Z,N,V | 1 | 1 |
| SER | Rd | Set register: Rd ← 0xFF | None | 1 | 1 |
| MUL | Rd, Rr | Multiply unsigned: R1:R0 ← Rd × Rr | Z,C | 1 | 2 |
| MULS | Rd, Rr | Multiply signed: R1:R0 ← Rd × Rr | Z,C | 1 | 2 |
| MULSU | Rd, Rr | Multiply signed with unsigned: R1:R0 ← Rd × Rr | Z,C | 1 | 2 |
| FMUL | Rd, Rr | Fractional multiply unsigned: R1:R0 ← (Rd × Rr) << 1 | Z,C | 1 | 2 |
| FMULS | Rd, Rr | Fractional multiply signed: R1:R0 ← (Rd × Rr) << 1 | Z,C | 1 | 2 |
| FMULSU | Rd, Rr | Fractional multiply signed with unsigned: R1:R0 ← (Rd × Rr) << 1 | Z,C | 1 | 2 |
| Branch | < | < | < | < | < |
| RJMP | k | Relative jump: PC ← PC + k + 1 | None | 1 | 2 |
| IJMP | | Indirect jump to (Z): PC ← Z | None | 1 | 2 |
| JMP [1] | k | Direct jump: PC ← k | None | 2 | 3 |
| RCALL | k | Relative subroutine call: PC ← PC + k + 1 | None | 1 | 3 |
| ICALL | | Indirect call to (Z): PC ← Z | None | 1 | 3 |
| CALL [1] | k | Direct subroutine call: PC ← k | None | 2 | 4 |
| RET | | Subroutine return: PC ← STACK | None | 1 | 4 |
| RETI | | Interrupt return: PC ← STACK | I | 1 | 4 |
| CPSE | Rd, Rr | Compare, skip if equal: if (Rd = Rr) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| CP | Rd, Rr | Compare: Rd − Rr | Z,N,V,C,H | 1 | 1 |
| CPC | Rd, Rr | Compare with carry: Rd − Rr − C | Z,N,V,C,H | 1 | 1 |
| CPI | Rd, K | Compare with immediate: Rd − K | Z,N,V,C,H | 1 | 1 |
| SBRC | Rr, b | Skip if bit in register cleared: if (Rr(b) = 0) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| SBRS | Rr, b | Skip if bit in register set: if (Rr(b) = 1) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| SBIC | P, b | Skip if bit in I/O register cleared: if (P(b) = 0) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| SBIS | P, b | Skip if bit in I/O register set: if (P(b) = 1) PC ← PC + 2 or 3 | None | 1 | 1/2/3 |
| BRBS | s, k | Branch if status flag set: if (SREG(s) = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRBC | s, k | Branch if status flag cleared: if (SREG(s) = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BREQ | k | Branch if equal: if (Z = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRNE | k | Branch if not equal: if (Z = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRCS | k | Branch if carry set: if (C = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRCC | k | Branch if carry cleared: if (C = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRSH | k | Branch if same or higher: if (C = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRLO | k | Branch if lower: if (C = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRMI | k | Branch if minus: if (N = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRPL | k | Branch if plus: if (N = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRGE | k | Branch if greater or equal, signed: if (N ⊕ V = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRLT | k | Branch if less than, signed: if (N ⊕ V = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRHS | k | Branch if half carry set: if (H = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRHC | k | Branch if half carry cleared: if (H = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRTS | k | Branch if T set: if (T = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRTC | k | Branch if T cleared: if (T = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRVS | k | Branch if overflow set: if (V = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRVC | k | Branch if overflow cleared: if (V = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRIE | k | Branch if interrupts enabled: if (I = 1) PC ← PC + k + 1 | None | 1 | 1/2 |
| BRID | k | Branch if interrupts disabled: if (I = 0) PC ← PC + k + 1 | None | 1 | 1/2 |
| Bit and bit-test | < | < | < | < | < |
| SBI | P, b | Set bit in I/O register: I/O(P,b) ← 1 | None | 1 | 2 |
| CBI | P, b | Clear bit in I/O register: I/O(P,b) ← 0 | None | 1 | 2 |
| LSL | Rd | Logical shift left: Rd(n+1) ← Rd(n), Rd(0) ← 0 | Z,C,N,V | 1 | 1 |
| LSR | Rd | Logical shift right: Rd(n) ← Rd(n+1), Rd(7) ← 0 | Z,C,N,V | 1 | 1 |
| ROL | Rd | Rotate left through carry: Rd(0) ← C, Rd(n+1) ← Rd(n), C ← Rd(7) | Z,C,N,V | 1 | 1 |
| ROR | Rd | Rotate right through carry: Rd(7) ← C, Rd(n) ← Rd(n+1), C ← Rd(0) | Z,C,N,V | 1 | 1 |
| ASR | Rd | Arithmetic shift right: Rd(n) ← Rd(n+1), n = 0…6 | Z,C,N,V | 1 | 1 |
| SWAP | Rd | Swap nibbles: Rd(3…0) ↔ Rd(7…4) | None | 1 | 1 |
| BSET | s | Flag set: SREG(s) ← 1 | SREG(s) | 1 | 1 |
| BCLR | s | Flag clear: SREG(s) ← 0 | SREG(s) | 1 | 1 |
| BST | Rr, b | Bit store from register to T: T ← Rr(b) | T | 1 | 1 |
| BLD | Rd, b | Bit load from T to register: Rd(b) ← T | None | 1 | 1 |
| SEC, CLC | | Set, clear carry: C ← 1, C ← 0 | C | 1 | 1 |
| SEN, CLN | | Set, clear negative flag: N ← 1, N ← 0 | N | 1 | 1 |
| SEZ, CLZ | | Set, clear zero flag: Z ← 1, Z ← 0 | Z | 1 | 1 |
| SEI, CLI | | Global interrupt enable, disable: I ← 1, I ← 0 | I | 1 | 1 |
| SES, CLS | | Set, clear signed test flag: S ← 1, S ← 0 | S | 1 | 1 |
| SEV, CLV | | Set, clear two's complement overflow: V ← 1, V ← 0 | V | 1 | 1 |
| SET, CLT | | Set, clear T: T ← 1, T ← 0 | T | 1 | 1 |
| SEH, CLH | | Set, clear half carry: H ← 1, H ← 0 | H | 1 | 1 |
| Data transfer | < | < | < | < | < |
| MOV | Rd, Rr | Move between registers: Rd ← Rr | None | 1 | 1 |
| MOVW | Rd, Rr | Copy register word: Rd+1:Rd ← Rr+1:Rr | None | 1 | 1 |
| LDI | Rd, K | Load immediate: Rd ← K | None | 1 | 1 |
| LD | Rd, X / Y / Z | Load indirect: Rd ← (X), (Y), (Z) | None | 1 | 2 |
| LD | Rd, X+ / Y+ / Z+ | Load indirect, post-increment: Rd ← (X), X ← X + 1 (same for Y, Z) | None | 1 | 2 |
| LD | Rd, -X / -Y / -Z | Load indirect, pre-decrement: X ← X − 1, Rd ← (X) (same for Y, Z) | None | 1 | 2 |
| LDD | Rd, Y+q / Z+q | Load indirect with displacement: Rd ← (Y + q), (Z + q) | None | 1 | 2 |
| LDS | Rd, k | Load direct from data space: Rd ← (k) | None | 2 | 2 |
| ST | X / Y / Z, Rr | Store indirect: (X), (Y), (Z) ← Rr | None | 1 | 2 |
| ST | X+ / Y+ / Z+, Rr | Store indirect, post-increment: (X) ← Rr, X ← X + 1 (same for Y, Z) | None | 1 | 2 |
| ST | -X / -Y / -Z, Rr | Store indirect, pre-decrement: X ← X − 1, (X) ← Rr (same for Y, Z) | None | 1 | 2 |
| STD | Y+q / Z+q, Rr | Store indirect with displacement: (Y + q), (Z + q) ← Rr | None | 1 | 2 |
| STS | k, Rr | Store direct to data space: (k) ← Rr | None | 2 | 2 |
| LPM | | Load program memory: R0 ← (Z) | None | 1 | 3 |
| LPM | Rd, Z | Load program memory: Rd ← (Z) | None | 1 | 3 |
| LPM | Rd, Z+ | Load program memory, post-increment: Rd ← (Z), Z ← Z + 1 | None | 1 | 3 |
| SPM | | Store program memory: (Z) ← R1:R0 | None | 1 | [2] |
| IN | Rd, P | In port: Rd ← P | None | 1 | 1 |
| OUT | P, Rr | Out port: P ← Rr | None | 1 | 1 |
| PUSH | Rr | Push register: STACK ← Rr | None | 1 | 2 |
| POP | Rd | Pop register: Rd ← STACK | None | 1 | 2 |
| MCU control | < | < | < | < | < |
| NOP | | No operation | None | 1 | 1 |
| SLEEP | | Sleep (see [power management](#power-management-and-sleep)) | None | 1 | 1 |
| WDR | | Watchdog reset (see the watchdog timer) | None | 1 | 1 |
| BREAK | | Break; for on-chip debug only | None | 1 | [3] |

[1] Available only in ATmega168PA and ATmega328P, per the source's note; see Conversion notes.
[2] Not given; depends on the SPM operation (see self-programming).
[3] N/A.
```

## Memory map

Two linear memory spaces, program (flash) and data, plus the EEPROM as a
separate data space (see [EEPROM](#eeprom)).

### Program memory

Flash organized as 16-bit words (all instructions are 16 or 32 bits),
endurance at least 10,000 write/erase cycles. Constant tables may sit
anywhere in program memory (LPM). Except in ATmega48A/PA the flash is
split into an application section and a boot loader section at the top,
each with its own lock bits; the boot section size is set by the BOOTSZ
fuses (see self-programming and memory programming).

```table id=flash-map caption="Program memory (word addresses)"
| Device | Flash | Organization | PC width | Address range | Sections |
|---|---|---|---|---|---|
| ATmega48A/PA | 4 Kbytes | 2K × 16 | 11 bits | 0x000–0x7FF | Application only |
| ATmega88A/PA | 8 Kbytes | 4K × 16 | 12 bits | 0x0000–0x0FFF | Application, then boot flash at the top |
| ATmega168A/PA | 16 Kbytes | 8K × 16 | 13 bits | 0x0000–0x1FFF | Application, then boot flash at the top |
| ATmega328/P | 32 Kbytes | 16K × 16 | 14 bits | 0x0000–0x3FFF | Application, then boot flash at the top |
```

### Data memory

```table id=data-map caption="Data memory"
| Address | Contents | Access |
|---|---|---|
| 0x0000–0x001F | 32 general purpose registers R0–R31 | All data addressing modes |
| 0x0020–0x005F | 64 I/O registers (I/O addresses 0x00–0x3F) | IN/OUT with the I/O address; LD/ST with the data address (I/O + 0x20); SBI, CBI, SBIS, SBIC for I/O 0x00–0x1F |
| 0x0060–0x00FF | 160 extended I/O registers | LD/LDS/LDD and ST/STS/STD only |
| 0x0100–RAMEND | Internal SRAM: 512 (ATmega48A/PA), 1024 (88A/PA, 168A/PA) or 2048 (328/P) bytes; RAMEND = 0x02FF, 0x04FF, 0x04FF, 0x08FF | All data addressing modes |
```

Direct addressing reaches the whole data space; indirect with
displacement reaches 63 locations above Y or Z. Internal SRAM accesses
take two clk~CPU~ cycles (address computed in the first, data read or
written in the second).

### I/O memory

All I/O registers are listed in [Registers and commands](#registers);
they are described with their peripherals. Write reserved bits as zero;
never write reserved I/O addresses. Some status flags are cleared by
writing 1 to them; SBI and CBI change only the addressed bit, so they
are safe on registers with such flags (I/O 0x00–0x1F only).

General purpose I/O registers GPIOR0 0x1E (0x3E), GPIOR1 0x2A (0x4A)
and GPIOR2 0x2B (0x4B): 8-bit R/W, reset 0x00, free for global variables
and status flags; GPIOR0 is bit-accessible with SBI, CBI, SBIS, SBIC.

### Reset and interrupt vectors

Each vector is one instruction word in ATmega48A/PA and ATmega88A/PA and
two words in ATmega168A/PA and ATmega328/P. A vector that is never
enabled may hold ordinary code, as may the vector area when the reset
vector and the interrupt vectors are in different sections.

```table id=vectors caption="Reset and interrupt vectors"
| Vector | Source | Definition | Address, ATmega48A/PA, 88A/PA | Address, ATmega168A/PA, 328/P |
|--:|---|---|---|---|
| 1 | RESET | External pin, power-on reset, brown-out reset and watchdog system reset [1] | 0x000 | 0x0000 |
| 2 | INT0 | External interrupt request 0 | 0x001 | 0x0002 |
| 3 | INT1 | External interrupt request 1 | 0x002 | 0x0004 |
| 4 | PCINT0 | Pin change interrupt request 0 | 0x003 | 0x0006 |
| 5 | PCINT1 | Pin change interrupt request 1 | 0x004 | 0x0008 |
| 6 | PCINT2 | Pin change interrupt request 2 | 0x005 | 0x000A |
| 7 | WDT | Watchdog time-out interrupt | 0x006 | 0x000C |
| 8 | TIMER2_COMPA | Timer/Counter2 compare match A | 0x007 | 0x000E |
| 9 | TIMER2_COMPB | Timer/Counter2 compare match B | 0x008 | 0x0010 |
| 10 | TIMER2_OVF | Timer/Counter2 overflow | 0x009 | 0x0012 |
| 11 | TIMER1_CAPT | Timer/Counter1 capture event | 0x00A | 0x0014 |
| 12 | TIMER1_COMPA | Timer/Counter1 compare match A | 0x00B | 0x0016 |
| 13 | TIMER1_COMPB | Timer/Counter1 compare match B | 0x00C | 0x0018 |
| 14 | TIMER1_OVF | Timer/Counter1 overflow | 0x00D | 0x001A |
| 15 | TIMER0_COMPA | Timer/Counter0 compare match A | 0x00E | 0x001C |
| 16 | TIMER0_COMPB | Timer/Counter0 compare match B | 0x00F | 0x001E |
| 17 | TIMER0_OVF | Timer/Counter0 overflow | 0x010 | 0x0020 |
| 18 | SPI_STC | SPI serial transfer complete | 0x011 | 0x0022 |
| 19 | USART_RX | USART Rx complete | 0x012 | 0x0024 |
| 20 | USART_UDRE | USART data register empty | 0x013 | 0x0026 |
| 21 | USART_TX | USART Tx complete | 0x014 | 0x0028 |
| 22 | ADC | ADC conversion complete | 0x015 | 0x002A |
| 23 | EE_READY | EEPROM ready | 0x016 | 0x002C |
| 24 | ANALOG_COMP | Analog comparator | 0x017 | 0x002E |
| 25 | TWI | 2-wire serial interface | 0x018 | 0x0030 |
| 26 | SPM_READY | Store program memory ready | 0x019 | 0x0032 |

[1] Except in ATmega48A/PA: with the BOOTRST fuse programmed the device jumps to the boot reset address at reset, and with IVSEL (MCUCR) set the interrupt vectors move to the start of the boot flash section (address = table address + boot section start).
```

```table id=vector-place caption="Reset and interrupt vector placement, ATmega88A/PA, 168A/PA, 328/P [1]"
| BOOTRST | IVSEL | Reset address | Interrupt vectors start, ATmega88A/PA | Interrupt vectors start, ATmega168A/PA, 328/P |
|:-:|:-:|---|---|---|
| 1 | 0 | 0x000 | 0x001 | 0x002 |
| 1 | 1 | 0x000 | Boot reset address + 0x001 | Boot reset address + 0x0002 |
| 0 | 0 | Boot reset address | 0x001 | 0x002 |
| 0 | 1 | Boot reset address | Boot reset address + 0x001 | Boot reset address + 0x0002 |

[1] BOOTRST 1 = unprogrammed, 0 = programmed. The boot reset address follows from the BOOTSZ fuses (see [the boot size table](#boot-sizes)). IVSEL and its timed change sequence (IVCE) are described under [interrupts](#interrupts).
```

### Lock bits

The ATmega48A/48PA have two lock bits, the ATmega88/168/328 six. Unprogrammed = 1,
programmed = 0; only chip erase sets them back to 1. On the ATmega48A/48PA (no boot section)
SPM works on the whole flash if SELFPRGEN is programmed, and is disabled otherwise.

```table id=prog-lockbyte caption="Lock bit byte"
| Bit | Name | Description | Default |
|---|---|---|---|
| 5 | BLB12 [1] | Boot lock bit | 1 |
| 4 | BLB11 [1] | Boot lock bit | 1 |
| 3 | BLB02 [1] | Boot lock bit | 1 |
| 2 | BLB01 [1] | Boot lock bit | 1 |
| 1 | LB2 | Lock bit | 1 |
| 0 | LB1 | Lock bit | 1 |

[1] ATmega88A/88PA/168A/168PA/328/328P only.
```

```table id=prog-lbmodes caption="Lock bit protection modes [1]"
| LB mode | LB2 | LB1 | Protection |
|--:|:-:|:-:|---|
| 1 | 1 | 1 | No memory lock features |
| 2 | 1 | 0 | Further programming of flash and EEPROM disabled in parallel and serial programming. Fuse bits locked in both modes. |
| 3 | 0 | 0 | Further programming and verification of flash and EEPROM disabled in parallel and serial programming. Boot lock bits and fuse bits locked in both modes. |

[1] Program the fuse bits and boot lock bits before LB1 and LB2.
```

The boot lock bit modes (BLB0, BLB1) are in [Boot loader](#boot-blb0).

### Fuse bits

Three fuse bytes; a programmed fuse reads 0. Chip erase does not change the fuses. Programming
LB1 locks the fuses, so program the fuses first.

```table id=prog-fusemap caption="Fuse byte layout"
| Byte | Bit | ATmega48A/48PA | ATmega88A/88PA/168A/168PA | ATmega328/328P |
|---|--:|---|---|---|
| Extended | 2 | – | BOOTSZ1 | BODLEVEL2 |
| ^ | 1 | – | BOOTSZ0 | BODLEVEL1 |
| ^ | 0 | SELFPRGEN | BOOTRST | BODLEVEL0 |
| High | 7 | RSTDISBL | RSTDISBL | RSTDISBL |
| ^ | 6 | DWEN | DWEN | DWEN |
| ^ | 5 | SPIEN | SPIEN | SPIEN |
| ^ | 4 | WDTON | WDTON | WDTON |
| ^ | 3 | EESAVE | EESAVE | EESAVE |
| ^ | 2 | BODLEVEL2 | BODLEVEL2 | BOOTSZ1 |
| ^ | 1 | BODLEVEL1 | BODLEVEL1 | BOOTSZ0 |
| ^ | 0 | BODLEVEL0 | BODLEVEL0 | BOOTRST |
| Low | 7 | CKDIV8 | CKDIV8 | CKDIV8 |
| ^ | 6 | CKOUT | CKOUT | CKOUT |
| ^ | 5 | SUT1 | SUT1 | SUT1 |
| ^ | 4 | SUT0 | SUT0 | SUT0 |
| ^ | 3:0 | CKSEL3:0 | CKSEL3:0 | CKSEL3:0 |
```

Unused fuse bits (extended byte bits 7:3, and the ones marked –) read 1.

```table id=prog-fuses caption="Fuse functions and defaults (1 = unprogrammed, 0 = programmed)"
| Fuse | Function | Default |
|---|---|---|
| SELFPRGEN | Self-programming enable (ATmega48A/48PA) | 1 |
| BOOTSZ1:0 | Boot size, see the [boot size table](#boot-sizes) | 00 (maximum boot size) |
| BOOTRST | Reset vector select, see the [boot reset table](#boot-bootrst) | 1 |
| BODLEVEL2:0 | Brown-out detector trigger level (decoding with the BOD characteristics) | 111 |
| RSTDISBL | External reset disable (!{RESET} becomes I/O pin PC6) | 1 |
| DWEN | debugWIRE enable | 1 |
| SPIEN | Serial program and data downloading enable; not accessible in serial programming mode | 0 (SPI programming enabled) |
| WDTON | Watchdog timer always on | 1 |
| EESAVE | EEPROM preserved through chip erase | 1 (EEPROM not preserved) |
| CKDIV8 | Divide clock by 8 (system clock prescaler) | 0 |
| CKOUT | Clock output on PORTB0 | 1 |
| SUT1:0 | Start-up time | 10 (maximum start-up time for the default clock source) |
| CKSEL3:0 | Clock source | 0010 (internal 8 MHz RC oscillator) |
```

Fuse values are latched on entering programming mode, and changes take effect only after
leaving it, except EESAVE, which takes effect as soon as it is programmed. Fuses are also
latched at power-up in normal mode.

### Signature and calibration bytes

A three-byte signature in a separate address space identifies the device; it can be read in
serial and parallel mode, also when the device is locked. The calibration byte for the
internal RC oscillator sits in the high byte of signature address 0x000 and is copied into
OSCCAL during reset.

```table id=prog-sig caption="Signature bytes"
| Device | 0x000 | 0x001 | 0x002 |
|---|---|---|---|
| ATmega48A | 0x1E | 0x92 | 0x05 |
| ATmega48PA | 0x1E | 0x92 | 0x0A |
| ATmega88A | 0x1E | 0x93 | 0x0A |
| ATmega88PA | 0x1E | 0x93 | 0x0F |
| ATmega168A | 0x1E | 0x94 | 0x06 |
| ATmega168PA | 0x1E | 0x94 | 0x0B |
| ATmega328 | 0x1E | 0x95 | 0x14 |
| ATmega328P | 0x1E | 0x95 | 0x0F |
```


## Peripherals

Register headings give the I/O address and, in parentheses, the data-space address (`0x24 (0x44)`); extended I/O registers have only a data-space address. The [register summary](#registers) lists every register. Reserved bits are left out of the bit tables: write them 0 and ignore them on read.

### EEPROM

256 (ATmega48A/PA), 512 (ATmega88A/PA, ATmega168A/PA) or 1024 bytes
(ATmega328/P) of data EEPROM in a separate data space, read and written
byte by byte; endurance at least 100,000 write/erase cycles. The access
registers are in the I/O space. Reading halts the CPU for four clock
cycles before the next instruction; writing halts it for two. Writes are
timed by the calibrated RC oscillator (26,368 cycles, typically 3.3 ms);
EEPE tells software when the next byte can be written. Programming the
EEPROM through SPI or parallel programming is described under
[memory programming](#memory-programming).

EEPROM data can be corrupted when V~CC~ is too low, either because the
write itself needs a minimum voltage or because the CPU executes
instructions incorrectly. Keep !{RESET} active during low supply: enable
the brown-out detector, or use an external low-V~CC~ reset circuit if no
BOD level fits. A write in progress at reset completes if the supply is
sufficient.

#### EEARH and EEARL – EEPROM address register, 0x22 (0x42) and 0x21 (0x41)

```table id=ee-eear caption="EEAR bits"
| Bit | Name | Description |
|---|---|---|
| 9:8 | EEAR[9:8] | Address high bits. Unused in ATmega48A/PA: always write 0. |
| 7:0 | EEAR[7:0] | EEPROM address, linear from 0 to 255/511/511/1023. Undefined after reset: write it before accessing the EEPROM. |
```

#### EEDR – EEPROM data register, 0x20 (0x40)

Data to write to, or data read from, the EEPROM address in EEAR. Reset 0.

#### EECR – EEPROM control register, 0x1F (0x3F)

```table id=ee-eecr caption="EECR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 5:4 | EEPM[1:0] | R/W | X | Programming mode triggered by writing EEPE (table below). Writes ignored while EEPE is set. Reset to 00 unless the EEPROM is busy programming. |
| 3 | EERIE | R/W | 0 | EEPROM ready interrupt enable (with SREG I). The interrupt is constant while EEPE is 0; not generated during an EEPROM write or SPM. |
| 2 | EEMPE | R/W | 0 | Master write enable: setting EEPE within four clock cycles after EEMPE is written to 1 writes the EEPROM; otherwise EEPE has no effect. Cleared by hardware after four cycles. |
| 1 | EEPE | R/W | X | Write enable (strobe). Cleared by hardware when the write time has elapsed. The CPU halts two cycles after EEPE is set. |
| 0 | EERE | R/W | 0 | Read enable: with the address in EEAR, writing 1 reads the byte into EEDR at once; the CPU halts four cycles. Neither a read nor an EEAR change is possible while a write is in progress: poll EEPE first. |
```

```table id=ee-mode caption="EEPROM programming modes"
| EEPM1 | EEPM0 | Typ programming time | Operation |
|:-:|:-:|---|---|
| 0 | 0 | 3.4 ms | Erase and write in one (atomic) operation |
| 0 | 1 | 1.8 ms | Erase only |
| 1 | 0 | 1.8 ms | Write only |
| 1 | 1 | – | Reserved |
```

Write procedure (steps 3 and 4 in either order):

1. Wait until EEPE is 0.
2. Wait until SPMEN in SPMCSR is 0 (only needed if a boot loader can write
   the flash; the EEPROM can't be written during a CPU flash write).
3. Write the address to EEAR (optional).
4. Write the data to EEDR (optional).
5. Write 1 to EEMPE while writing 0 to EEPE.
6. Within four clock cycles, write 1 to EEPE.

An interrupt between steps 5 and 6 makes the write fail (EEMPE times
out), and an interrupt routine that accesses the EEPROM corrupts an
interrupted access by changing EEAR or EEDR: keep the global interrupt
flag cleared during all steps.

### Clock system

The selected clock source feeds the AVR clock control unit through the
system clock prescaler; the unit distributes these clock domains, each
of which sleep modes can halt (see [power management and
sleep](#power-management-and-sleep)):

- **clk~CPU~**: the AVR core (register file, SREG, stack). Halting it
  stops the core.
- **clk~I/O~**: most I/O modules (timer/counters, SPI, USART) and the
  external interrupts. TWI address recognition works in all sleep modes
  with clk~I/O~ halted.
- **clk~FLASH~**: the flash interface; usually active with clk~CPU~.
- **clk~ASY~**: lets Timer/Counter2 run from an external clock or a
  32 kHz crystal, as a real-time counter even in sleep.
- **clk~ADC~**: a separate ADC domain, so the CPU and I/O clocks can be
  halted to reduce noise during a conversion.

A level-triggered interrupt used to wake from power-down must be held
until the start-up time ends; if it goes away earlier the MCU still
wakes, but no interrupt is generated.

#### Clock sources

The CKSEL fuses select the source ("1" means unprogrammed, "0"
programmed in every fuse table).

```table id=clk-sources caption="Device clocking options"
| Clock source | CKSEL[3:0] |
|---|---|
| Low power crystal oscillator | 1111–1000 |
| Full swing crystal oscillator | 0111–0110 |
| Low frequency crystal oscillator | 0101–0100 |
| Internal 128 kHz RC oscillator | 0011 |
| Calibrated internal RC oscillator | 0010 |
| External clock | 0000 |
| Reserved | 0001 |
```

Parts ship with the internal 8.0 MHz RC oscillator and CKDIV8
programmed (1.0 MHz system clock), maximum start-up time and the
time-out enabled: CKSEL = 0010, SUT = 10, CKDIV8 = 0.

**Start-up sequence.** After all other reset sources release, an
internal reset is held for a time-out t~TOUT~, timed by the watchdog
oscillator, so V~CC~ can reach a sufficient level; the SUT and CKSEL
fuses set its length (table below). The delay does not monitor the
voltage: choose one longer than the V~CC~ rise time, or use the BOD
(then the time-out can be disabled; disabling it without a BOD is not
recommended). A ripple counter then holds reset for the oscillator's
start-up cycle count (6 cycles for an external clock up to 32K cycles
for a low-frequency crystal). Waking from power-down or power-save
includes only the start-up time, not the time-out.

```table id=clk-tout caption="Watchdog oscillator cycles in the time-out"
| Typ time-out, V~CC~ = 5.0 V | Typ time-out, V~CC~ = 3.0 V | Cycles |
|---|---|--:|
| 0 ms | 0 ms | 0 |
| 4.1 ms | 4.3 ms | 512 |
| 65 ms | 69 ms | 8K (8,192) |
```

#### Low power crystal oscillator

XTAL1 and XTAL2 are the input and output of an inverting amplifier for a
quartz crystal or ceramic resonator, with capacitors C1 and C2 from each
pin to GND. This oscillator has a reduced swing on XTAL2: lowest power,
but it can't drive other clock inputs and is more susceptible to noise
(use the full swing oscillator then). C1 and C2 are always equal; for
resonators use the manufacturer's values. CKSEL[3:1] selects the
frequency range; CKSEL0 and SUT[1:0] the start-up time.

```table id=clk-lpxo caption="Low power crystal oscillator operating modes [1]"
| Frequency range (MHz) | C1, C2 for crystals (pF) | CKSEL[3:1] [2] |
|---|---|---|
| 0.4–0.9 | – | 100 [3] |
| 0.9–3.0 | 12–22 | 101 |
| 3.0–8.0 | 12–22 | 110 |
| 8.0–16.0 | 12–22 | 111 |

[1] If the crystal frequency exceeds the device's specification (depends on V~CC~), program CKDIV8 to divide it by 8; the divided clock must meet the specification.
[2] Recommended setting for the frequency range.
[3] Ceramic resonators only, not crystals.
```

```table id=clk-xo-sut caption="Start-up times, low power and full swing crystal oscillators"
| Oscillator source / power conditions | From power-down and power-save | Additional delay from reset (V~CC~ = 5.0 V) | CKSEL0 | SUT[1:0] |
|---|---|---|:-:|:-:|
| Ceramic resonator, fast rising power | 258 CK | 14 CK + 4.1 ms [1] | 0 | 00 |
| Ceramic resonator, slowly rising power | 258 CK | 14 CK + 65 ms [1] | 0 | 01 |
| Ceramic resonator, BOD enabled | 1K CK | 14 CK [2] | 0 | 10 |
| Ceramic resonator, fast rising power | 1K CK | 14 CK + 4.1 ms [2] | 0 | 11 |
| Ceramic resonator, slowly rising power | 1K CK | 14 CK + 65 ms [2] | 1 | 00 |
| Crystal oscillator, BOD enabled | 16K CK | 14 CK | 1 | 01 |
| Crystal oscillator, fast rising power | 16K CK | 14 CK + 4.1 ms | 1 | 10 |
| Crystal oscillator, slowly rising power | 16K CK | 14 CK + 65 ms | 1 | 11 |

[1] Only when not operating close to the device's maximum frequency, and only if frequency stability at start-up doesn't matter. Not suitable for crystals.
[2] For ceramic resonators; ensures frequency stability at start-up. Also usable with crystals when not close to the maximum frequency and if stability at start-up doesn't matter.
```

#### Full swing crystal oscillator

Same connection, with rail-to-rail swing on XTAL2: suitable for driving
other clock inputs and for noisy environments, at higher current. Works
only at V~CC~ = 2.7–5.5 V. 0.4–20 MHz, C1 = C2 = 12–22 pF for crystals,
CKSEL[3:1] = 011 (program CKDIV8 if the frequency exceeds the device's
specification). Start-up times as in the table above.

#### Low frequency crystal oscillator

Optimized for a 32.768 kHz watch crystal, for very low power. The
crystal vendor specifies load capacitance C~L~ and ESR.

```table id=clk-lfxo-esr caption="Maximum ESR for a 32.768 kHz crystal [1]"
| Crystal C~L~ (pF) | Max ESR (kΩ) |
|--:|--:|
| 6.5 | 75 |
| 9.0 | 65 |
| 12.5 | 30 |

[1] Typical value, based on characterization.
```

Internal load capacitance C~i~ at each pin, as system oscillator and as
timer oscillator alike: 18 pF on XTAL1/TOSC1, 8 pF on XTAL2/TOSC2. The
capacitance needed at each pin (external C~e~ plus C~i~), with C~S~ the
stray capacitance of one pin:

```equation
C~e~ + C~i~ = 2 × C~L~ − C~S~
```

Crystals with C~L~ above 6 pF need external capacitors.

```table id=clk-lfxo-sut caption="Low frequency crystal oscillator: additional delay from reset"
| SUT[1:0] | Additional delay from reset (V~CC~ = 5.0 V) | Recommended usage |
|:-:|---|---|
| 00 | 4 CK | Fast rising power or BOD enabled |
| 01 | 4 CK + 4.1 ms | Slowly rising power |
| 10 | 4 CK + 65 ms | Stable frequency at start-up |
| 11 | – | Reserved |
```

```table id=clk-lfxo-cksel caption="Low frequency crystal oscillator: start-up time from power-down and power-save"
| CKSEL[3:0] | Start-up time | Recommended usage |
|:-:|---|---|
| 0100 [1] | 1K CK | |
| 0101 | 32K CK | Stable frequency at start-up |

[1] Only if frequency stability at start-up doesn't matter.
```

#### Calibrated internal RC oscillator

About 8.0 MHz with no external components (CKSEL = 0010, 7.3–8.1 MHz;
shipped selected, with CKDIV8 programmed, which divides by 8 if 8 MHz
exceeds the device's specification at the V~CC~ used). Reset loads the
factory calibration value into OSCCAL; software can recalibrate it more
accurately (see the accuracy table below). The watchdog oscillator still
times the watchdog and the reset time-out.

```table id=clk-rc-sut caption="Start-up times, calibrated internal RC oscillator and 128 kHz oscillator"
| Power conditions | From power-down and power-save | Additional delay from reset, RC oscillator (V~CC~ = 5.0 V) | Additional delay from reset, 128 kHz oscillator | SUT[1:0] |
|---|---|---|---|:-:|
| BOD enabled | 6 CK | 14 CK [1] | 14 CK [1] | 00 |
| Fast rising power | 6 CK | 14 CK + 4.1 ms | 14 CK + 4 ms | 01 |
| Slowly rising power | 6 CK | 14 CK + 65 ms [2] | 14 CK + 64 ms | 10 |
| Reserved | | | | 11 |

[1] With the RSTDISBL fuse programmed, this becomes 14 CK + 4.1 ms so programming mode can still be entered.
[2] Shipped setting (RC oscillator).
```

#### 128 kHz internal oscillator

A very low power, low accuracy 128 kHz clock (nominal at 3 V, 25 °C),
selected with CKSEL = 0011. Start-up times in the table above.

#### External clock

An external clock on XTAL1 (XTAL2/PB7 free), CKSEL = 0000, 0–20 MHz.
Start-up: 6 CK from power-down and power-save; additional delay from
reset 14 CK (SUT = 00, BOD enabled), 14 CK + 4.1 ms (01, fast rising
power), 14 CK + 65 ms (10, slowly rising power); SUT = 11 is reserved.
The frequency must not change by more than 2 % from one cycle to the
next, or behavior is unpredictable; hold the MCU in reset during larger
changes, or change the frequency at run time with the system clock
prescaler.

#### Clock output buffer

With the CKOUT fuse programmed, the system clock (after the prescaler)
is output on CLKO, also during reset, overriding the pin's I/O function.
Any clock source can be used.

#### Timer/counter oscillator

The low frequency crystal oscillator doubles as the Timer/Counter2
oscillator on TOSC1/TOSC2, which share the XTAL1/XTAL2 pins. The system
clock must be at least four times the oscillator frequency, so the timer
oscillator can only be used with the calibrated internal RC oscillator as
system clock. An external clock can be applied to TOSC1 with EXCLK in
ASSR (see [Timer/Counter2](#timercounter2)).

#### System clock prescaler

CLKPR divides the selected clock for clk~I/O~, clk~ADC~, clk~CPU~ and
clk~FLASH~, with any clock source, to save power. Switching is
glitch-free, and no intermediate frequency is higher than either the old
or the new one. The prescaler's ripple counter runs at the undivided
clock and can't be read, so the switch time is not exact: the new
frequency is active between T1 + T2 and T1 + 2 × T2 after CLKPS is
written (T1 the old clock period, T2 the new), with 2 active clock edges
in that interval.

Changing CLKPS, with interrupts disabled:

1. Write CLKPCE = 1 and all other CLKPR bits 0.
2. Within four cycles, write the new CLKPS value with CLKPCE = 0.

#### OSCCAL – Oscillator calibration register, (0x66)

```table id=clk-osccal caption="OSCCAL bits"
| Bit | Name | Description |
|---|---|---|
| 7 | CAL7 | Range: 0 = low frequency range, 1 = high. The ranges overlap: 0x7F gives a higher frequency than 0x80. |
| 6:0 | CAL[6:0] | Tunes within the range: 0x00 lowest, 0x7F highest frequency |
```

Reset loads the factory calibration value. Calibration outside the user
calibration range below is not ensured. The oscillator times EEPROM and
flash writes: don't calibrate above 8.8 MHz when writing them, or the
write may fail.

#### CLKPR – Clock prescale register, (0x61)

```table id=clk-clkpr caption="CLKPR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | CLKPCE | R/W | 0 | Prescaler change enable. Updated only when the other bits are written 0 at the same time. Cleared by hardware four cycles after it is written or when CLKPS is written; rewriting it in that window neither extends nor clears the time-out. |
| 3:0 | CLKPS[3:0] | R/W | 0000 or 0011 | Division factor between the clock source and the system clock (table below). Reset to 0000 with CKDIV8 unprogrammed, 0011 (÷8) with CKDIV8 programmed. Any value can be written regardless of CKDIV8; software must keep the clock within the device's maximum frequency. |
```

```table id=clk-clkps caption="Clock prescaler select"
| CLKPS[3:0] | Division factor |
|:-:|--:|
| 0000 | 1 |
| 0001 | 2 |
| 0010 | 4 |
| 0011 | 8 |
| 0100 | 16 |
| 0101 | 32 |
| 0110 | 64 |
| 0111 | 128 |
| 1000 | 256 |
| 1001–1111 | Reserved |
```

#### Clock characteristics

```table id=clk-rc-acc caption="Calibration accuracy of the internal RC oscillator"
| Calibration | Frequency | V~CC~ | Temperature | Accuracy |
|---|---|---|---|---|
| Factory | 8.0 MHz | 3 V | 25 °C | ±10 % |
| User | 7.3–8.1 MHz | 1.8–5.5 V | -40 °C to 85 °C | ±1 % |
```

```wavedrom id=clk-ext caption="External clock drive waveform"
{"signal": [
  {"name": "XTAL1", "wave": "0.1..0..1..0", "node": "..A..B..C..D"}
 ],
 "edge": ["A<->B tCHCX", "B<->C tCLCX", "A<->C tCLCL"],
 "config": {"hscale": 2}}
```

```table id=clk-ext-drive caption="External clock drive"
| Symbol | Parameter | V~CC~ = 1.8–5.5 V | < | V~CC~ = 2.7–5.5 V | < | V~CC~ = 4.5–5.5 V | < | Unit |
| ^ | ^ | Min | Max | Min | Max | Min | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|---|
| 1/t~CLCL~ | Oscillator frequency | 0 | 4 | 0 | 10 | 0 | 20 | MHz |
| t~CLCL~ | Clock period | 250 | | 100 | | 50 | | ns |
| t~CHCX~ | High time | 100 | | 40 | | 20 | | ns |
| t~CLCX~ | Low time | 100 | | 40 | | 20 | | ns |
| t~CLCH~ | Rise time | | 2.0 | | 1.6 | | 0.5 | µs |
| t~CHCL~ | Fall time | | 2.0 | | 1.6 | | 0.5 | µs |
| Δt~CLCL~ | Change in period from one clock cycle to the next | | 2 | | 2 | | 2 | % |
```

High and low times are measured at V~IH1~ and V~IL1~.

### Power management and sleep

Sleep modes shut down unused modules. Setting SE in SMCR and executing
SLEEP enters the mode SM[2:0] selects. An enabled interrupt wakes the
MCU: it is halted four cycles plus the start-up time, runs the interrupt
routine, and resumes after the SLEEP instruction. Register file and SRAM
are unaltered. A reset during sleep wakes the MCU and runs from the reset
vector. An enabled BOD keeps monitoring V~CC~ during sleep unless
disabled by software (below).

```table id=pm-modes caption="Active clock domains, oscillators and wake-up sources in the sleep modes (X = active / enabled / can wake)"
| Sleep mode | clk~CPU~ | clk~FLASH~ | clk~IO~ | clk~ADC~ | clk~ASY~ | Main clock source | Timer oscillator | INT1, INT0, pin change | TWI address match | Timer2 | SPM/EEPROM ready | ADC | WDT | Other I/O | Software BOD disable [1] |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| Idle | | | X | X | X | X | X [2] | X | X | X | X | X | X | X | |
| ADC noise reduction | | | | X | X | X | X [2] | X [4] | X | X [2] | X | X | X | | |
| Power-down | | | | | | | | X [4] | X | | | | X | | X |
| Power-save | | | | | X | | X [2] | X [4] | X | X | | | X | | X |
| Standby [3] | | | | | | X | | X [4] | X | | | | X | | X |
| Extended standby | | | | | X [2] | X | X [2] | X [4] | X | X | | | X | | X |

[1] ATmega48PA/88PA/168PA/328P only.
[2] If Timer/Counter2 runs in asynchronous mode.
[3] Only recommended with an external crystal or resonator as clock source.
[4] INT0 and INT1 wake these modes only as level interrupts (see [external interrupts](#external-interrupts)).
```

#### BOD disable

(ATmega48PA/88PA/168PA/328P only.) With the BOD enabled by the BODLEVEL
fuses, software can turn it off during power-down, power-save, standby
and extended standby, for the same sleep current as with the BOD
disabled by fuses. Setting BODS in MCUCR turns the BOD off right after
entering sleep; it is re-enabled automatically on wake-up, which then
takes about 60 µs so the BOD works before code continues. Timed
sequence:

1. Write BODS = 1 and BODSE = 1.
2. Within four clock cycles, write BODS = 1 and BODSE = 0.
3. BODS is active three clock cycles after it is set and clears itself
   three cycles later: execute SLEEP while it is active.

#### Modes

- **Idle** (SM = 000): stops clk~CPU~ and clk~FLASH~; SPI, USART, analog
  comparator, ADC, TWI, timer/counters, watchdog and interrupts keep
  running. Any external or internal interrupt (timer overflow, USART
  transmit complete …) wakes the MCU. Set ACD in ACSR to power down the
  comparator if its interrupt isn't needed. An enabled ADC starts a
  conversion on entry.
- **ADC noise reduction** (001): also stops clk~I/O~; the ADC, external
  interrupts, TWI address watch, Timer/Counter2 (asynchronous mode only)
  and watchdog keep running. An enabled ADC starts a conversion on
  entry. Wake-up: ADC conversion complete, external reset, watchdog
  reset or interrupt, brown-out reset, TWI address match, Timer/Counter2
  interrupt, SPM/EEPROM ready, an INT0/INT1 level interrupt, or a pin
  change interrupt.
- **Power-down** (010): the external oscillator stops; external
  interrupts, TWI address watch and watchdog keep running; only
  asynchronous modules work. Wake-up: external reset, watchdog reset or
  interrupt, brown-out reset, TWI address match, an INT0/INT1 level
  interrupt, or a pin change interrupt. Wake-up takes the start-up time
  set by the CKSEL fuses (as the reset time-out); a wake-up level must
  be held until it ends, or the MCU wakes with no interrupt.
- **Power-save** (011): as power-down, but an enabled Timer/Counter2
  keeps running and its overflow or compare match wakes the MCU (with
  the TIMSK2 enable and SREG I set). Timer/Counter2 may run synchronously
  or asynchronously; the unused one of the timer oscillator and the main
  clock stops; a running synchronous clock serves Timer/Counter2 only.
  Use power-down if Timer/Counter2 isn't running.
- **Standby** (110, external crystal or resonator selected): as
  power-down, but the oscillator keeps running; wakes in six clock
  cycles.
- **Extended standby** (111, external crystal or resonator selected): as
  power-save with the oscillator running; wakes in six clock cycles.

#### Power reduction

PRR stops the clock to individual peripherals in active and idle mode
(in other sleep modes the clocks are already stopped). A stopped
module's state is frozen and its I/O registers can't be read or
written; resources it uses stay occupied, so disable the module first.
Clearing the PRR bit restores the state from before the shutdown.

#### Minimizing power consumption

Use the deepest sleep mode possible and disable unneeded functions:

- **ADC**: enabled in all sleep modes if enabled; disable it before
  sleep. The first conversion after turning it on again is an extended
  conversion.
- **Analog comparator**: disable it in idle when unused and always in
  ADC noise reduction; other modes disable it automatically, but if it
  uses the internal reference, disable it in all sleep modes or the
  reference stays on.
- **Brown-out detector**: if enabled by fuses it runs in all sleep modes
  and contributes significantly in deep sleep; turn it off if not needed.
- **Internal voltage reference**: on whenever the BOD, analog comparator
  or ADC needs it; off once those are disabled. After turning it on,
  allow its start-up time before use (see [reset and
  watchdog](#reset-and-watchdog)); kept on in sleep it is usable at once.
- **Watchdog timer**: runs in all sleep modes if enabled; turn it off if
  not needed.
- **Port pins**: no pin should drive a resistive load. With both
  clk~I/O~ and clk~ADC~ stopped, input buffers are disabled except those
  needed for wake-up (see [I/O ports](#io-ports)). An enabled input
  buffer with a floating input or a level near V~CC~/2 draws excessive
  current; for analog inputs disable the digital input buffer at all
  times (DIDR1, DIDR0), since a level near V~CC~/2 draws significant
  current even in active mode.
- **On-chip debug system**: with DWEN programmed, the main clock source
  stays enabled in sleep.

#### SMCR – Sleep mode control register, 0x33 (0x53)

```table id=pm-smcr caption="SMCR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 3:1 | SM[2:0] | R/W | 000 | Sleep mode: 000 = idle, 001 = ADC noise reduction, 010 = power-down, 011 = power-save, 100 and 101 = reserved, 110 = standby [1], 111 = extended standby [1] |
| 0 | SE | R/W | 0 | Sleep enable: must be 1 for SLEEP to enter the mode. Set it just before SLEEP and clear it right after waking. |

[1] Only recommended with external crystals or resonators.
```

#### MCUCR – MCU control register (BODS, BODSE), 0x35 (0x55)

Only BODS and BODSE are described here; PUD is under [I/O
ports](#io-ports), IVSEL and IVCE under [interrupts](#interrupts).

```table id=pm-mcucr caption="MCUCR bits"
| Bit | Name | Description |
|---|---|---|
| 6 | BODS | BOD sleep [1]: 1 turns the BOD off during sleep, set by the timed sequence above |
| 5 | BODSE | BOD sleep enable [1]: enables setting BODS |
| 4 | PUD | Pull-up disable |
| 1 | IVSEL | Interrupt vector select |
| 0 | IVCE | Interrupt vector change enable |

[1] ATmega48PA/88PA/168PA/328P only.
```

#### PRR – Power reduction register, (0x64)

Writing 1 to a bit stops the module's clock.

```table id=pm-prr caption="PRR bits"
| Bit | Name | Description |
|---|---|---|
| 7 | PRTWI | TWI. Re-initialize the TWI after waking it. |
| 6 | PRTIM2 | Timer/Counter2 in synchronous mode (AS2 = 0); continues as before when re-enabled |
| 5 | PRTIM0 | Timer/Counter0; continues as before when re-enabled |
| 3 | PRTIM1 | Timer/Counter1; continues as before when re-enabled |
| 2 | PRSPI | SPI. Re-initialize after waking. Don't set when using debugWIRE. |
| 1 | PRUSART0 | USART0. Re-initialize after waking. |
| 0 | PRADC | ADC; disable the ADC first. The analog comparator can't use the ADC input multiplexer while the ADC is shut down. |
```

### Reset and watchdog

During reset all I/O registers take their initial values and execution
starts at the reset vector, which must hold a JMP to the reset routine
on ATmega168A/168PA/328/328P and an RJMP on ATmega48A/48PA/88A/88PA. If
no interrupt is ever enabled, or the reset vector and the interrupt
vectors are in different sections (application vs boot,
ATmega88A/88PA/168A/168PA/328/328P only), program code may occupy the
vector locations. I/O ports go to their initial state as soon as a
reset source is active, with no clock running. When all sources are
inactive, a delay counter clocked by the watchdog oscillator stretches
the internal reset by the time-out t~TOUT~ set by the SUT and CKSEL fuses
(see [clock system](#clock-system)).

Reset logic: the power-on, brown-out (BODLEVEL[2:0]), external (pin
pull-up and spike filter, disabled by RSTDISBL) and watchdog reset
circuits set the flags in MCUSR and feed the delay counters
(CKSEL[3:0], SUT[1:0]).

Reset sources:

- **Power-on reset**: V~CC~ below the power-on reset threshold V~POT~.
  An on-chip detector holds reset while V~CC~ is below its level; when
  V~CC~ rises past V~POT~ the delay counter starts, and reset returns
  without delay when V~CC~ falls below it again. It also detects supply
  failures.
- **External reset**: a low level on !{RESET} longer than t~RST~, even
  with no clock running; shorter pulses are not ensured to reset. When
  !{RESET} rises past V~RST~, the MCU starts after t~TOUT~. RSTDISBL
  disables it.
- **Watchdog system reset**: the watchdog period expires in system reset
  mode. It produces a one-CK reset pulse; t~TOUT~ starts at its falling
  edge.
- **Brown-out reset**: V~CC~ below V~BOT~ with the BOD enabled by the
  BODLEVEL fuses. The trigger level has hysteresis: V~BOT+~ = V~BOT~ +
  V~HYST~/2, V~BOT−~ = V~BOT~ − V~HYST~/2. Reset activates at once when
  V~CC~ falls below V~BOT−~ for longer than t~BOD~; the MCU starts
  t~TOUT~ after V~CC~ rises above V~BOT+~.

```wavedrom id=rst-por caption="MCU start-up, RESET tied to VCC (or extended externally: the time-out starts when RESET reaches VRST)"
{"signal": [
  {"name": "VCC", "wave": "0.1.......", "node": "..A"},
  {"name": "RESET", "wave": "0.1.......", "node": "..B"},
  {"name": "TIME-OUT", "wave": "0.1....0..", "node": "..C....D"},
  {"name": "internal reset", "wave": "1......0.."}
 ],
 "edge": ["C<->D tTOUT"],
 "config": {"hscale": 2}}
```

```wavedrom id=rst-ext caption="External reset during operation"
{"signal": [
  {"name": "RESET", "wave": "1.0..1.......", "node": ".....A"},
  {"name": "TIME-OUT", "wave": "0....1....0..", "node": ".....B....C"},
  {"name": "internal reset", "wave": "0.1.......0.."}
 ],
 "edge": ["B<->C tTOUT"],
 "config": {"hscale": 2}}
```

```wavedrom id=rst-bod caption="Brown-out reset during operation (VCC falls below VBOT-, rises above VBOT+)"
{"signal": [
  {"name": "VCC ≥ VBOT", "wave": "1.0..1.......", "node": "..A..B"},
  {"name": "TIME-OUT", "wave": "0....1....0..", "node": ".....C....D"},
  {"name": "internal reset", "wave": "0.1.......0.."}
 ],
 "edge": ["C<->D tTOUT"],
 "config": {"hscale": 2}}
```

```wavedrom id=rst-wdt caption="Watchdog system reset during operation"
{"signal": [
  {"name": "CK", "wave": "p............"},
  {"name": "WDT time-out", "wave": "0.10........."},
  {"name": "TIME-OUT", "wave": "0..1.....0...", "node": "...A.....B"},
  {"name": "internal reset", "wave": "0.1......0..."}
 ],
 "edge": ["A<->B tTOUT"],
 "config": {"hscale": 2}}
```

#### Internal voltage reference

An internal bandgap reference serves the BOD and can be an input to the
analog comparator or the ADC. It is on only while the BOD is enabled
(BODLEVEL fuses), while ACBG in ACSR connects it to the comparator, or
while the ADC is enabled. With the BOD off, allow the start-up time
t~BG~ after setting ACBG or enabling the ADC before using their output.
Avoiding all three conditions turns the reference off in power-down.

#### Watchdog timer

The watchdog counts cycles of a separate on-chip 128 kHz oscillator and
gives an interrupt, a system reset, or both, when the counter reaches
the selected time-out (16 ms to 8 s). Software restarts the counter with
the WDR instruction before the time-out.

- **Interrupt mode**: an interrupt at time-out; wakes the device from
  sleep, or serves as a general system timer (e.g. bounding an
  operation's duration).
- **System reset mode**: a reset at time-out, against runaway code.
- **Interrupt and system reset mode**: first an interrupt, then system
  reset mode (e.g. to save parameters before the reset). Executing the
  interrupt vector clears WDIE and WDIF, putting the watchdog in system
  reset mode; to stay in this mode, set WDIE again after each interrupt,
  but not inside the interrupt routine, which would compromise the
  safety function. If the interrupt isn't executed before the next
  time-out, a system reset follows.

The WDTON fuse, if programmed, forces system reset mode: WDE and WDIE
are locked to 1 and 0.

```table id=rst-wdt-modes caption="Watchdog timer configuration"
| WDTON [1] | WDE | WDIE | Mode | Action on time-out |
|:-:|:-:|:-:|---|---|
| 1 | 0 | 0 | Stopped | None |
| 1 | 0 | 1 | Interrupt mode | Interrupt |
| 1 | 1 | 0 | System reset mode | Reset |
| 1 | 1 | 1 | Interrupt and system reset mode | Interrupt, then go to system reset mode |
| 0 | X | X | System reset mode | Reset |

[1] 0 = programmed, 1 = unprogrammed.
```

Clearing WDE or changing the prescaler needs a timed sequence (with
interrupts disabled, and the watchdog reset with WDR first, since a
change to a shorter period can cause a time-out):

1. In one write, set WDCE and WDE to 1 (WDE = 1 regardless of its
   previous value).
2. Within the next four clock cycles, write WDE and WDP as desired with
   WDCE = 0, in one operation.

To turn the watchdog off: WDR, clear WDRF in MCUSR, then the sequence
with WDE = 0 in step 2. A watchdog enabled by accident (runaway pointer,
brown-out) stays enabled across the reset it causes, and can loop in
time-out resets: always clear WDRF and WDE in the initialization code,
even if the watchdog is not used.

#### MCUSR – MCU status register, 0x34 (0x54)

Shows which source caused the last reset. Read it, then clear it, as
early as possible in the program; flags cleared before the next reset
then identify that reset's source.

```table id=rst-mcusr caption="MCUSR bits"
| Bit | Name | Description |
|---|---|---|
| 3 | WDRF | Watchdog system reset flag. Cleared by a power-on reset or by writing 0. |
| 2 | BORF | Brown-out reset flag. Cleared by a power-on reset or by writing 0. |
| 1 | EXTRF | External reset flag. Cleared by a power-on reset or by writing 0. |
| 0 | PORF | Power-on reset flag. Cleared only by writing 0. |
```

#### WDTCSR – Watchdog timer control register, (0x60)

```table id=rst-wdtcsr caption="WDTCSR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | WDIF | R/W | 0 | Watchdog interrupt flag: set on a time-out in interrupt mode. Cleared by executing the interrupt vector or by writing 1. With SREG I and WDIE set, the time-out interrupt executes. |
| 6 | WDIE | R/W | 0 | Watchdog interrupt enable (with SREG I). WDE = 0: interrupt mode; WDE = 1: interrupt and system reset mode. |
| 5 | WDP3 | R/W | 0 | Prescaler bit 3 (table below) |
| 4 | WDCE | R/W | 0 | Change enable: must be set to clear WDE or change WDP. Cleared by hardware after four clock cycles. |
| 3 | WDE | R/W | X | System reset enable. Overridden by WDRF: always set while WDRF is set, so clear WDRF first. |
| 2:0 | WDP[2:0] | R/W | 000 | Prescaler bits 2–0 |
```

```table id=rst-wdp caption="Watchdog timer prescale select"
| WDP[3:0] | WDT oscillator cycles | Typ time-out at V~CC~ = 5.0 V |
|:-:|---|---|
| 0000 | 2K (2048) | 16 ms |
| 0001 | 4K (4096) | 32 ms |
| 0010 | 8K (8192) | 64 ms |
| 0011 | 16K (16384) | 0.125 s |
| 0100 | 32K (32768) | 0.25 s |
| 0101 | 64K (65536) | 0.5 s |
| 0110 | 128K (131072) | 1.0 s |
| 0111 | 256K (262144) | 2.0 s |
| 1000 | 512K (524288) | 4.0 s |
| 1001 | 1024K (1048576) | 8.0 s |
| 1010–1111 | Reserved | |
```

#### System and reset characteristics

```table id=rst-char caption="Reset, brown-out and internal voltage characteristics [1]"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~POT~ | Power-on reset threshold voltage | Rising | 1.1 | 1.4 | 1.6 | V |
| ^ | ^ | Falling [2] | 0.6 | 1.3 | 1.6 | V |
| SR~ON~ | Power-on slope rate | | 0.01 | | 10 | V/ms |
| V~RST~ | !{RESET} pin threshold voltage | | 0.2 × V~CC~ | | 0.9 × V~CC~ | V |
| t~RST~ | Minimum pulse width on !{RESET} | | | | 2.5 | µs |
| V~HYST~ | Brown-out detector hysteresis | | | 50 | | mV |
| t~BOD~ | Minimum pulse width on brown-out reset | | | 2 | | µs |
| V~BG~ | Bandgap reference voltage | V~CC~ = 2.7 V, T~A~ = 25 °C | 1.0 | 1.1 | 1.2 | V |
| t~BG~ | Bandgap reference start-up time | V~CC~ = 2.7 V, T~A~ = 25 °C | | 40 | 70 | µs |
| I~BG~ | Bandgap reference current consumption | V~CC~ = 2.7 V, T~A~ = 25 °C | | 10 | | µA |

[1] Values are guidelines only.
[2] The power-on reset works only if the supply has been below V~POT~ (falling).
```

```table id=rst-bodlevel caption="BODLEVEL fuse coding [1][2]"
| BODLEVEL[2:0] | Min V~BOT~ | Typ V~BOT~ | Max V~BOT~ | Unit |
|:-:|--:|--:|--:|---|
| 111 | BOD disabled | < | < | < |
| 110 | 1.7 | 1.8 | 2.0 | V |
| 101 | 2.5 | 2.7 | 2.9 | V |
| 100 | 4.1 | 4.3 | 4.5 | V |
| 011–000 | Reserved | < | < | < |

[1] V~BOT~ may be below the nominal minimum operating voltage for some devices; these are tested down to V~CC~ = V~BOT~ in production (with BODLEVEL = 110, 101 and 100), so a brown-out reset occurs before V~CC~ drops to where correct operation is no longer ensured.
[2] V~BOT~ tested at 25 °C and 85 °C in production.
```

### Interrupts

The reset and interrupt vector tables and their placement are given in
[the memory map](#memory-map). On ATmega88A/88PA, ATmega168A/168PA and
ATmega328/328P, MCUCR moves the interrupt vectors between the start of
flash and the start of the boot loader section (whose address the
BOOTSZ fuses set; see [boot loader and read-while-write
self-programming](#boot-loader-and-read-while-write-self-programming-atmega88168328)).

#### MCUCR – MCU control register (IVSEL, IVCE), 0x35 (0x55)

Only IVSEL and IVCE are described here; the full bit table is under
[power management and sleep](#power-management-and-sleep).

```table id=int-mcucr caption="MCUCR interrupt vector bits"
| Bit | Name | Description |
|---|---|---|
| 1 | IVSEL | Interrupt vector select: 0 = vectors at the start of flash, 1 = at the start of the boot loader section |
| 0 | IVCE | Interrupt vector change enable: must be set to change IVSEL. Cleared by hardware four cycles after it is written or when IVSEL is written. |
```

Changing IVSEL:

1. Write IVCE = 1.
2. Within four cycles, write the new IVSEL value with IVCE = 0.

Interrupts are disabled automatically from the cycle IVCE is set until
after the instruction following the IVSEL write (four cycles if IVSEL
isn't written); SREG I is not affected. With the vectors in the boot
loader section and boot lock bit BLB02 programmed, interrupts are
disabled while executing from the application section; with the vectors
in the application section and BLB12 programmed, they are disabled while
executing from the boot loader section.

### External interrupts

The INT0 and INT1 pins and the PCINT[23:0] pins trigger the external
interrupts, even when the pins are outputs (a way to make software
interrupts). Pin change interrupt PCI2 triggers when any enabled
PCINT[23:16] pin toggles, PCI1 for PCINT[14:8], PCI0 for PCINT[7:0];
PCMSK2, PCMSK1 and PCMSK0 select the contributing pins. INT0 and INT1
trigger on a falling or rising edge, any change, or a low level (EICRA);
a level interrupt triggers as long as the pin is held low. Pin change
interrupts, low-level interrupts and the INT edge interrupts are
detected asynchronously, so they can wake the part from sleep modes
other than idle (where clk~I/O~ is halted). A level used to wake from
power-down must be held until the start-up time ends, or the MCU wakes
with no interrupt.

Pin change timing: the pin is latched and synchronized; a change on an
enabled pin sets PCIF about three clock cycles later.

```wavedrom id=ext-pcint caption="Pin change interrupt timing"
{"signal": [
  {"name": "clk", "wave": "1010101010"},
  {"name": "PCINT(0)", "wave": "01........"},
  {"name": "pin_lat", "wave": "01........"},
  {"name": "pin_sync", "wave": "0.1......."},
  {"name": "pcint_in_(0)", "wave": "010......."},
  {"name": "pcint_syn", "wave": "0.1.0....."},
  {"name": "pcint_setflag", "wave": "0...1.0..."},
  {"name": "PCIF", "wave": "0.....1..."}
 ]}
```

#### EICRA – External interrupt control register A, (0x69)

```table id=ext-eicra caption="EICRA bits"
| Bit | Name | Description |
|---|---|---|
| 3:2 | ISC1[1:0] | INT1 sense control (table below) |
| 1:0 | ISC0[1:0] | INT0 sense control (table below) |
```

```table id=ext-isc caption="Interrupt sense control (n = 0 for INT0, 1 for INT1)"
| ISCn1 | ISCn0 | Interrupt request on |
|:-:|:-:|---|
| 0 | 0 | Low level of INTn |
| 0 | 1 | Any logical change on INTn |
| 1 | 0 | Falling edge of INTn |
| 1 | 1 | Rising edge of INTn |
```

The pin is sampled before edges are detected. With an edge or toggle
interrupt, pulses longer than one clock period generate an interrupt;
shorter ones are not ensured to. A low level must be held until the
current instruction completes.

#### EIMSK – External interrupt mask register, 0x1D (0x3D)

```table id=ext-eimsk caption="EIMSK bits"
| Bit | Name | Description |
|---|---|---|
| 1 | INT1 | External interrupt request 1 enable (with SREG I); executed from the INT1 vector |
| 0 | INT0 | External interrupt request 0 enable (with SREG I); executed from the INT0 vector |
```

#### EIFR – External interrupt flag register, 0x1C (0x3C)

```table id=ext-eifr caption="EIFR bits"
| Bit | Name | Description |
|---|---|---|
| 1 | INTF1 | Set by an edge or change on INT1 that triggers a request. Cleared when the interrupt routine executes or by writing 1. Always 0 while INT1 is a level interrupt. |
| 0 | INTF0 | As INTF1, for INT0 |
```

#### PCICR – Pin change interrupt control register, (0x68)

```table id=ext-pcicr caption="PCICR bits"
| Bit | Name | Description |
|---|---|---|
| 2 | PCIE2 | Pin change interrupt 2 enable (with SREG I): any change on an enabled PCINT[23:16] pin, PCI2 vector |
| 1 | PCIE1 | Pin change interrupt 1 enable: PCINT[14:8], PCI1 vector |
| 0 | PCIE0 | Pin change interrupt 0 enable: PCINT[7:0], PCI0 vector |
```

#### PCIFR – Pin change interrupt flag register, 0x1B (0x3B)

```table id=ext-pcifr caption="PCIFR bits"
| Bit | Name | Description |
|---|---|---|
| 2 | PCIF2 | Set by a change on any PCINT[23:16] pin that triggers a request; cleared when the interrupt routine executes or by writing 1 |
| 1 | PCIF1 | As PCIF2, for PCINT[14:8] |
| 0 | PCIF0 | As PCIF2, for PCINT[7:0] |
```

#### PCMSK2, PCMSK1, PCMSK0 – Pin change mask registers, (0x6D), (0x6C), (0x6B)

One bit per pin: 1 enables the pin change interrupt on that pin (with
the PCIE bit in PCICR set). All R/W, reset 0.

```table id=ext-pcmsk caption="Pin change mask registers"
| Register | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|---|---|---|---|---|---|---|---|---|
| PCMSK2 | PCINT23 | PCINT22 | PCINT21 | PCINT20 | PCINT19 | PCINT18 | PCINT17 | PCINT16 |
| PCMSK1 | – (reserved, R) | PCINT14 | PCINT13 | PCINT12 | PCINT11 | PCINT10 | PCINT9 | PCINT8 |
| PCMSK0 | PCINT7 | PCINT6 | PCINT5 | PCINT4 | PCINT3 | PCINT2 | PCINT1 | PCINT0 |
```

### I/O ports

Ports B, C and D are bidirectional I/O ports with true read-modify-write:
SBI and CBI change one pin's direction, drive value or pull-up without
disturbing the others. Each output buffer has symmetrical drive with high
sink and source capability, strong enough to drive LED displays directly.
Every pin has an individually selectable pull-up with a supply-voltage
invariant resistance, and protection diodes to V~CC~ and GND (pin
equivalent: R~pu~ and C~pin~ from Pxn to the logic). Limits are in
[electrical characteristics](#electrical-characteristics). Enabling the
alternate function of a pin does not affect the other pins of its port.

Each port has three I/O locations: the data register PORTx and the data
direction register DDRx (read/write), and the input pins PINx (read; a
write of 1 toggles the PORTx bit). In the following, x is the port letter
and n the bit number. PUD in MCUCR disables all pull-ups in all ports.

#### Configuring the pin

DDxn selects the direction: 1 = output, 0 = input. With the pin an input,
PORTxn = 1 enables the pull-up; PORTxn = 0, or making the pin an output,
switches it off. With the pin an output, PORTxn drives it high (1) or low
(0). Port pins are tri-stated while reset is active, even with no clocks
running. Writing 1 to PINxn toggles PORTxn whatever DDxn holds (SBI can
toggle one pin).

```table id=io-config caption="Port pin configurations"
| DDxn | PORTxn | PUD (MCUCR) | I/O | Pull-up | Comment |
|:-:|:-:|:-:|---|---|---|
| 0 | 0 | X | Input | No | Tri-state (Hi-Z) |
| 0 | 1 | 0 | Input | Yes | Pxn sources current if pulled low externally |
| 0 | 1 | 1 | Input | No | Tri-state (Hi-Z) |
| 1 | 0 | X | Output | No | Output low (sink) |
| 1 | 1 | X | Output | No | Output high (source) |
```

Switching between tri-state ({DDxn, PORTxn} = 0b00) and output high (0b11)
passes through either pull-up enabled (0b01) or output low (0b10).
Pull-up enabled is normally acceptable, since a high-impedance load can't
tell a strong high driver from a pull-up; if not, set PUD to disable all
pull-ups. Switching between input with pull-up and output low has the same
problem: use tri-state (0b00) or output high (0b11) as the intermediate
step.

#### Reading the pin

PINxn reads the pin whatever DDxn holds, through a synchronizer (a latch
transparent while the system clock is high, then the PINxn flip-flop) that
avoids metastability but adds delay: the level is latched when the system
clock goes low and clocked into PINxn at the next rising edge, so a pin
transition reaches PINxn ½ to 1½ system clock periods after it occurs
(t~pd,min~ to t~pd,max~).

```wavedrom id=io-sync caption="Synchronization when reading an externally applied pin value"
{"signal": [
  {"name": "SYSTEM CLK", "wave": "lhlhlhlhl"},
  {"name": "INSTRUCTIONS", "wave": "x=.=.=.=.", "data": ["xxx", "xxx", "in r17, PINx", ""]},
  {"name": "SYNC LATCH", "wave": "0..x1....", "node": "..A.B"},
  {"name": "PINxn", "wave": "0....1...", "node": ".....C"},
  {"name": "r17", "wave": "=......=.", "data": ["0x00", "0xFF"]}
 ],
 "edge": ["A<->C tpd,max", "B<->C tpd,min"],
 "config": {"hscale": 2}}
```

To read back a value the software just wrote to PORTx, put a NOP between
the OUT and the IN: OUT sets the sync latch at the rising clock edge, so
the delay t~pd~ through the synchronizer is one system clock period.

#### Digital input enable and sleep modes

In power-down, power-save and standby the sleep controller clamps the
digital input (the Schmitt-trigger input) to ground, so floating inputs or
analog levels near V~CC~/2 draw no current. The clamp is overridden on pins
enabled as external interrupts, and by various alternate functions (DIEOE
and DIEOV below); a pin whose external interrupt is not enabled is clamped.
If a logic high is present on an asynchronous external interrupt pin
configured for rising edge, falling edge or any change while that interrupt
is not enabled, its external interrupt flag is set on wake-up from these
modes, since the clamping produces the requested change.

#### Unconnected pins

Give unused pins a defined level: floating inputs raise the current in the
modes where digital inputs are enabled (reset, active, idle). The simplest
way is the internal pull-up, which is off during reset; if low current
during reset matters, use an external pull-up or pull-down. Don't connect
unused pins directly to V~CC~ or GND: an accidental output configuration
would draw excessive current.

#### Alternate port functions

The alternate functions override the port's control signals per pin. Each
overriding signal is generated inside the module with the alternate
function:

```table id=io-override caption="Overriding signals for alternate functions"
| Signal | Full name | Description |
|---|---|---|
| PUOE | Pull-up override enable | 1: the pull-up enable is PUOV. 0: the pull-up is enabled when {DDxn, PORTxn, PUD} = 0b010. |
| PUOV | Pull-up override value | With PUOE set, the pull-up is enabled/disabled when PUOV is set/cleared, whatever DDxn, PORTxn and PUD hold. |
| DDOE | Data direction override enable | 1: the output driver enable is DDOV. 0: the output driver is enabled by DDxn. |
| DDOV | Data direction override value | With DDOE set, the output driver is enabled/disabled when DDOV is set/cleared, whatever DDxn holds. |
| PVOE | Port value override enable | With the output driver enabled: 1 = the port value is PVOV; 0 = the port value is PORTxn. |
| PVOV | Port value override value | With PVOE set, the port value is PVOV, whatever PORTxn holds. |
| PTOE | Port toggle override enable | 1: PORTxn is inverted. |
| DIEOE | Digital input enable override enable | 1: the digital input enable is DIEOV. 0: it follows the MCU state (normal or sleep). |
| DIEOV | Digital input enable override value | With DIEOE set, the digital input is enabled/disabled when DIEOV is set/cleared, whatever the MCU state. |
| DI | Digital input | Input to the alternate function, taken after the Schmitt trigger but before the synchronizer. Unless it is used as a clock source, the module uses its own synchronizer. |
| AIO | Analog input/output | Connected directly to the pad; bidirectional. |
```

```table id=io-altfn caption="Alternate pin functions"
| Pin | Alternate functions | Behavior |
|---|---|---|
| PB7 | XTAL2 (chip clock oscillator pin 2), TOSC2 (timer oscillator pin 2), PCINT7 | XTAL2: clock pin of the crystal or low-frequency crystal oscillator; then not an I/O pin. TOSC2: only with the calibrated internal RC oscillator as chip clock; with AS2 = 1 and EXCLK = 0 in ASSR the pin leaves the port and becomes the inverting output of the Timer/Counter2 oscillator amplifier (crystal connected, not an I/O pin). Used as a clock pin, DDB7, PORTB7 and PINB7 read 0. |
| PB6 | XTAL1 (chip clock oscillator pin 1 or external clock input), TOSC1 (timer oscillator pin 1), PCINT6 | XTAL1: used for every chip clock source except the calibrated internal RC oscillator; then not an I/O pin. TOSC1: only with the calibrated internal RC oscillator as chip clock; with AS2 = 1 the pin leaves the port and becomes the input of the inverting oscillator amplifier (crystal connected, not an I/O pin). Used as a clock pin, DDB6, PORTB6 and PINB6 read 0. |
| PB5 | SCK (SPI master clock output, slave clock input), PCINT5 | SPI slave: input whatever DDB5 holds. SPI master: direction from DDB5. When forced to input, PORTB5 still controls the pull-up. |
| PB4 | MISO (SPI master data input, slave data output), PCINT4 | SPI master: input whatever DDB4 holds. SPI slave: direction from DDB4. When forced to input, PORTB4 still controls the pull-up. |
| PB3 | MOSI (SPI master data output, slave data input), OC2A (Timer/Counter2 output compare match A output), PCINT3 | SPI slave: input whatever DDB3 holds. SPI master: direction from DDB3. When forced to input, PORTB3 still controls the pull-up. OC2A: compare match and PWM output; needs DDB3 = 1. |
| PB2 | !{SS} (SPI slave select input), OC1B (Timer/Counter1 output compare match B output), PCINT2 | SPI slave: input whatever DDB2 holds; the SPI is active while the pin is low. SPI master: direction from DDB2. When forced to input, PORTB2 still controls the pull-up. OC1B: compare match and PWM output; needs DDB2 = 1. |
| PB1 | OC1A (Timer/Counter1 output compare match A output), PCINT1 | OC1A: compare match and PWM output; needs DDB1 = 1. |
| PB0 | ICP1 (Timer/Counter1 input capture input), CLKO (divided system clock output), PCINT0 | CLKO: the divided system clock is output when the CKOUT fuse is programmed, whatever PORTB0 and DDB0 hold, and also during reset. |
| PC6 | !{RESET} (reset pin), PCINT14 | RSTDISBL fuse programmed: normal I/O pin, and the part relies on power-on and brown-out reset. Unprogrammed: the reset circuit owns the pin (not an I/O pin) and DDC6, PORTC6 and PINC6 read 0. |
| PC5 | ADC5 (ADC input channel 5), SCL (2-wire serial bus clock line), PCINT13 | SCL: with TWEN = 1 in TWCR the pin leaves the port and becomes the TWI clock I/O, with a spike filter suppressing input spikes shorter than 50 ns and an open-drain driver with slew-rate limitation. ADC5 uses digital power. |
| PC4 | ADC4 (ADC input channel 4), SDA (2-wire serial bus data input/output line), PCINT12 | SDA: with TWEN = 1 the pin leaves the port and becomes the TWI data I/O, with the same spike filter and slew-limited open-drain driver. ADC4 uses digital power. |
| PC3–PC0 | ADC3–ADC0 (ADC input channels 3–0), PCINT11–PCINT8 | ADC3–ADC0 use analog power. |
| PD7 | AIN1 (analog comparator negative input), PCINT23 | AIN1: configure as input with the pull-up off so the digital port doesn't interfere with the comparator. |
| PD6 | AIN0 (analog comparator positive input), OC0A (Timer/Counter0 output compare match A output), PCINT22 | AIN0: input with the pull-up off. OC0A: compare match and PWM output; needs DDD6 = 1. |
| PD5 | T1 (Timer/Counter1 external counter input), OC0B (Timer/Counter0 output compare match B output), PCINT21 | OC0B: compare match and PWM output; needs DDD5 = 1. |
| PD4 | XCK (USART external clock input/output), T0 (Timer/Counter0 external counter input), PCINT20 | |
| PD3 | INT1 (external interrupt 1 input), OC2B (Timer/Counter2 output compare match B output), PCINT19 | OC2B: compare match and PWM output; needs DDD3 = 1. |
| PD2 | INT0 (external interrupt 0 input), PCINT18 | |
| PD1 | TXD (USART output pin), PCINT17 | With the USART transmitter enabled the pin is an output whatever DDD1 holds. |
| PD0 | RXD (USART input pin), PCINT16 | With the USART receiver enabled the pin is an input whatever DDD0 holds; PORTD0 still controls the pull-up. |
```

Every PCINTn pin can serve as an external (pin change) interrupt source.
The tables below give each pin's overriding signals. SPI MSTR INPUT and SPI
SLAVE OUTPUT make up the MISO signal; MOSI is split into SPI MSTR OUTPUT and
SPI SLAVE INPUT.

```table id=io-ovr-b caption="Overriding signals, port B [1]"
| Signal | PB7/XTAL2/TOSC2/PCINT7 | PB6/XTAL1/TOSC1/PCINT6 | PB5/SCK/PCINT5 | PB4/MISO/PCINT4 | PB3/MOSI/OC2A/PCINT3 | PB2/!{SS}/OC1B/PCINT2 | PB1/OC1A/PCINT1 | PB0/ICP1/PCINT0 |
|---|---|---|---|---|---|---|---|---|
| PUOE | !{INTRC} • !{EXTCK} + AS2 | !{INTRC} + AS2 | SPE • !{MSTR} | SPE • MSTR | SPE • !{MSTR} | SPE • !{MSTR} | 0 | 0 |
| PUOV | 0 | 0 | PORTB5 • !{PUD} | PORTB4 • !{PUD} | PORTB3 • !{PUD} | PORTB2 • !{PUD} | 0 | 0 |
| DDOE | !{INTRC} • !{EXTCK} + AS2 | !{INTRC} + AS2 | SPE • !{MSTR} | SPE • MSTR | SPE • !{MSTR} | SPE • !{MSTR} | 0 | 0 |
| DDOV | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| PVOE | 0 | 0 | SPE • MSTR | SPE • !{MSTR} | SPE • MSTR + OC2A ENABLE | OC1B ENABLE | OC1A ENABLE | 0 |
| PVOV | 0 | 0 | SCK OUTPUT | SPI SLAVE OUTPUT | SPI MSTR OUTPUT + OC2A | OC1B | OC1A | 0 |
| DIEOE | !{INTRC} • !{EXTCK} + AS2 + PCINT7 • PCIE0 | !{INTRC} + AS2 + PCINT6 • PCIE0 | PCINT5 • PCIE0 | PCINT4 • PCIE0 | PCINT3 • PCIE0 | PCINT2 • PCIE0 | PCINT1 • PCIE0 | PCINT0 • PCIE0 |
| DIEOV | (INTRC + EXTCK) • !{AS2} | INTRC • !{AS2} | 1 | 1 | 1 | 1 | 1 | 1 |
| DI | PCINT7 INPUT | PCINT6 INPUT | PCINT5 INPUT, SCK INPUT | PCINT4 INPUT, SPI MSTR INPUT | PCINT3 INPUT, SPI SLAVE INPUT | PCINT2 INPUT, SPI !{SS} | PCINT1 INPUT | PCINT0 INPUT, ICP1 INPUT |
| AIO | Oscillator output | Oscillator/clock input | – | – | – | – | – | – |

[1] INTRC: one of the internal RC oscillators is selected (CKSEL fuses). EXTCK: external clock is selected (CKSEL fuses).
```

```table id=io-ovr-c caption="Overriding signals, port C [1]"
| Signal | PC6/!{RESET}/PCINT14 | PC5/SCL/ADC5/PCINT13 | PC4/SDA/ADC4/PCINT12 | PC3/ADC3/PCINT11 | PC2/ADC2/PCINT10 | PC1/ADC1/PCINT9 | PC0/ADC0/PCINT8 |
|---|---|---|---|---|---|---|---|
| PUOE | RSTDISBL | TWEN | TWEN | 0 | 0 | 0 | 0 |
| PUOV | 1 | PORTC5 • !{PUD} | PORTC4 • !{PUD} | 0 | 0 | 0 | 0 |
| DDOE | RSTDISBL | TWEN | TWEN | 0 | 0 | 0 | 0 |
| DDOV | 0 | SCL_OUT | SDA_OUT | 0 | 0 | 0 | 0 |
| PVOE | 0 | TWEN | TWEN | 0 | 0 | 0 | 0 |
| PVOV | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| DIEOE | RSTDISBL + PCINT14 • PCIE1 | PCINT13 • PCIE1 + ADC5D | PCINT12 • PCIE1 + ADC4D | PCINT11 • PCIE1 + ADC3D | PCINT10 • PCIE1 + ADC2D | PCINT9 • PCIE1 + ADC1D | PCINT8 • PCIE1 + ADC0D |
| DIEOV | RSTDISBL | PCINT13 • PCIE1 | PCINT12 • PCIE1 | PCINT11 • PCIE1 | PCINT10 • PCIE1 | PCINT9 • PCIE1 | PCINT8 • PCIE1 |
| DI | PCINT14 INPUT | PCINT13 INPUT | PCINT12 INPUT | PCINT11 INPUT | PCINT10 INPUT | PCINT9 INPUT | PCINT8 INPUT |
| AIO | RESET INPUT | ADC5 INPUT / SCL INPUT | ADC4 INPUT / SDA INPUT | ADC3 INPUT | ADC2 INPUT | ADC1 INPUT | ADC0 INPUT |

[1] When enabled, the TWI enables slew-rate control on the outputs PC4 and PC5, and spike filters sit between the AIO outputs and the TWI logic.
```

```table id=io-ovr-d caption="Overriding signals, port D"
| Signal | PD7/AIN1/PCINT23 | PD6/AIN0/OC0A/PCINT22 | PD5/T1/OC0B/PCINT21 | PD4/XCK/T0/PCINT20 | PD3/OC2B/INT1/PCINT19 | PD2/INT0/PCINT18 | PD1/TXD/PCINT17 | PD0/RXD/PCINT16 |
|---|---|---|---|---|---|---|---|---|
| PUOE | 0 | 0 | 0 | 0 | 0 | 0 | TXEN | RXEN |
| PUOV | 0 | 0 | 0 | 0 | 0 | 0 | 0 | PORTD0 • !{PUD} |
| DDOE | 0 | 0 | 0 | 0 | 0 | 0 | TXEN | RXEN |
| DDOV | 0 | 0 | 0 | 0 | 0 | 0 | 1 | 0 |
| PVOE | 0 | OC0A ENABLE | OC0B ENABLE | UMSEL | OC2B ENABLE | 0 | TXEN | 0 |
| PVOV | 0 | OC0A | OC0B | XCK OUTPUT | OC2B | 0 | TXD | 0 |
| DIEOE | PCINT23 • PCIE2 | PCINT22 • PCIE2 | PCINT21 • PCIE2 | PCINT20 • PCIE2 | INT1 ENABLE + PCINT19 • PCIE2 | INT0 ENABLE + PCINT18 • PCIE1 [1] | PCINT17 • PCIE2 | PCINT16 • PCIE2 |
| DIEOV | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
| DI | PCINT23 INPUT | PCINT22 INPUT | PCINT21 INPUT, T1 INPUT | PCINT20 INPUT, XCK INPUT, T0 INPUT | PCINT19 INPUT, INT1 INPUT | PCINT18 INPUT, INT0 INPUT | PCINT17 INPUT | PCINT16 INPUT, RXD |
| AIO | AIN1 INPUT | AIN0 INPUT | – | – | – | – | – | – |

[1] As printed; PCINT18 belongs to pin change interrupt 2 (PCIE2).
```

#### MCUCR – MCU Control Register (PUD), 0x35 (0x55)

Only PUD belongs here. BODS and BODSE are described under
[power management and sleep](#power-management-and-sleep), IVSEL and
IVCE under [interrupts](#interrupts).

```table id=io-mcucr caption="MCUCR bits (PUD)"
| Bit | Name | Description |
|---|---|---|
| 4 | PUD | Pull-up disable: 1 = the pull-ups of all I/O ports are off even where {DDxn, PORTxn} = 0b01 |
```

#### Port registers

Plain bit-per-pin registers: bit n of PORTx, DDRx and PINx belongs to
pin Pxn (PORTB7…PORTB0, DDB7…DDB0, PINB7…PINB0, and likewise for ports C
and D). Port C has no bit 7 (reads 0, read-only).

```table id=io-regs caption="Port registers"
| Address | Name | Bits | Access | Reset | Function |
|---|---|---|---|---|---|
| 0x05 (0x25) | PORTB | PORTB7:0 | R/W | 0x00 | Port B data |
| 0x04 (0x24) | DDRB | DDB7:0 | R/W | 0x00 | Port B data direction |
| 0x03 (0x23) | PINB | PINB7:0 | R/W [1] | N/A | Port B input pins |
| 0x08 (0x28) | PORTC | PORTC6:0 | R/W | 0x00 | Port C data |
| 0x07 (0x27) | DDRC | DDC6:0 | R/W | 0x00 | Port C data direction |
| 0x06 (0x26) | PINC | PINC6:0 | R/W [1] | N/A (bit 7: 0) | Port C input pins |
| 0x0B (0x2B) | PORTD | PORTD7:0 | R/W | 0x00 | Port D data |
| 0x0A (0x2A) | DDRD | DDD7:0 | R/W | 0x00 | Port D data direction |
| 0x09 (0x29) | PIND | PIND7:0 | R/W [1] | N/A | Port D input pins |

[1] Reading returns the synchronized pin levels; writing 1 to a bit toggles the corresponding PORTx bit.
```

### Timer/Counter0

An 8-bit timer/counter with two independent output compare units (double
buffered in the PWM modes), clear timer on compare match (auto reload),
glitch-free phase correct PWM, variable PWM period, frequency generation,
and three interrupt sources (TOV0, OCF0A, OCF0B). Block diagram: the
counter TCNT0 with its control logic and clock select (prescaler or the T0
pin through an edge detector), two 8-bit comparators against OCR0A and
OCR0B feeding the waveform generators of OC0A and OC0B, and the control
registers TCCR0A and TCCR0B. PRTIM0 in PRR must be 0 to enable the module.

TCNT0, OCR0A and OCR0B are 8-bit registers. The interrupt flags are in
TIFR0 and individually masked in TIMSK0. The compare result can drive the
OC0A/OC0B pins (PWM or variable frequency) and sets OCF0A/OCF0B. In the
following, n = 0 and x = A or B.

```table id=t0-defs caption="Definitions"
| Term | Meaning |
|---|---|
| BOTTOM | The counter reaches BOTTOM when it becomes 0x00. |
| MAX | The counter reaches its maximum when it becomes 0xFF (255). |
| TOP | The counter reaches TOP when it equals the highest value in the count sequence: the fixed value 0xFF (MAX) or the value in OCR0A, depending on the mode. |
```

#### Clock sources and counter unit

The clock select bits CS0[2:0] (TCCR0B) pick an internal clock through the
prescaler or an external clock on T0 (see [the prescalers](#timercounter0-and-timercounter1-prescalers));
their output is the timer clock clk~T0~. With CS0[2:0] = 0 the timer is
stopped, but the CPU can still read and write TCNT0; a CPU write overrides
(has priority over) any clear or count operation. Depending on the mode the
counter is cleared, incremented or decremented at each clk~T0~. The
counting sequence is set by WGM01:0 (TCCR0A) and WGM02 (TCCR0B); TOV0 is
set according to the mode and can raise an interrupt.

#### Output compare unit

The comparator continuously compares TCNT0 with OCR0A and OCR0B. A match
sets OCF0x at the next timer clock cycle (and raises the output compare
interrupt if enabled); the flag clears when the interrupt executes, or by
writing 1 to it. The waveform generator uses the match, WGM0[2:0] and
COM0x[1:0] to drive OC0x, and the max and bottom signals for the extreme
values.

OCR0x is double buffered in the PWM modes, updated at TOP or BOTTOM so PWM
pulses can't be odd-length or non-symmetrical (glitch-free). In normal and
CTC modes double buffering is off. With double buffering on, the CPU
accesses the buffer register; otherwise OCR0x directly.

- **Force output compare.** In non-PWM modes, writing 1 to FOC0x forces a
  compare match: OC0x is set, cleared or toggled per COM0x[1:0], but OCF0x
  is not set and the timer is not reloaded or cleared.
- **Compare match blocking by TCNT0 write.** Every CPU write to TCNT0 blocks
  any compare match in the next timer clock cycle, even with the timer
  stopped, so OCR0x can be initialized to TCNT0's value without an
  interrupt when the clock starts.
- **Using the unit.** Because a TCNT0 write blocks compare matches for one
  timer clock, writing TCNT0 equal to OCR0x loses that match (wrong
  waveform), whether the timer runs or not; nor write TCNT0 equal to BOTTOM
  while counting down. Set up OC0x before making the pin an output; the
  easiest way is FOC0x in normal mode. The OC0x registers keep their
  values across mode changes. COM0x[1:0] is not double buffered: a change
  takes effect immediately.

#### Compare match output unit

COM0x[1:0] set the OC0x state at the next compare match and select the
pin's output source: if either bit is set, OC0x from the waveform generator
overrides the port function, but the pin's direction is still DDR's, and
DDR must be set to output for OC0x to show on the pin. The override is
independent of the waveform mode. OC0x (the internal register, not the
pin) is reset to 0 by a system reset, and can be initialized before the
output is enabled. COM0x[1:0] = 0 means no action on OC0x at the next
match in every mode; a COM change takes effect at the first compare match
after the write (in non-PWM modes FOC0x makes it immediate). Some settings
are reserved in some modes (tables under TCCR0A).

#### Modes of operation

The mode (counting sequence, TOP, waveform type) is set by WGM0[2:0]; the
COM0x[1:0] bits select inverted or non-inverted PWM, or set/clear/toggle
at a match in non-PWM modes, and don't affect counting.

**Normal mode** (WGM0[2:0] = 0). Counts up, no clear; overruns from TOP =
0xFF to 0x00. TOV0 is set in the same timer clock cycle that TCNT0 becomes
zero, acting as a ninth bit that is only set, never cleared; with the
overflow interrupt clearing it, software can extend the resolution. A new
counter value can be written at any time. The compare unit can time
interrupts; generating waveforms in normal mode is not recommended (too
much CPU time).

**Clear timer on compare match (CTC)** (WGM0[2:0] = 2). The counter is
cleared when TCNT0 matches OCR0A, so OCR0A is TOP and sets the resolution;
OCF0A can interrupt at each TOP, and the handler may update TOP. CTC has no
double buffering: changing TOP to a value close to BOTTOM while running
with no or a low prescaler needs care, since if the new OCR0A is below
TCNT0 the match is missed and the counter runs to 0xFF and wraps first.
With COM0A[1:0] = 1 (toggle) OC0A toggles at each match (visible only if
the pin is an output), giving at most f~clk_I/O~/2 with OCR0A = 0:

```equation
f~OC0x~ = f~clk_I/O~ / (2 × N × (1 + OCR0x))
```

N is the prescale factor (1, 8, 64, 256 or 1024). TOV0 is set in the
timer clock cycle that the counter goes from MAX to 0x00.

**Fast PWM** (WGM0[2:0] = 3 or 7). Single slope: counts BOTTOM to TOP,
then is cleared at the next timer clock. TOP = 0xFF (mode 3) or OCR0A
(mode 7). Non-inverting (COM0x[1:0] = 2): OC0x clears at the compare match
and sets at BOTTOM; inverting (3): sets at the match and clears at BOTTOM.
Up to twice the frequency of phase correct PWM, suited to power
regulation, rectification and DAC use. TOV0 is set each time the counter
reaches TOP; the handler may update the compare value. COM0A[1:0] = 1
toggles OC0A at each match when WGM02 = 1 (not available for OC0B). The
output is visible only with the pin set as output.

```equation
f~OC0xPWM~ = f~clk_I/O~ / (N × 256)
```

OCR0A = BOTTOM gives a narrow spike every MAX+1 timer clocks; OCR0A = MAX
gives a constant high or low output, per COM0A[1:0]. Toggling OC0x at each
match (COM0x[1:0] = 1) gives a 50 % duty-cycle waveform of at most
f~clk_I/O~/2 with OCR0A = 0, like CTC toggling but with double buffering.

**Phase correct PWM** (WGM0[2:0] = 1 or 5). Dual slope: counts BOTTOM to
TOP and back; TCNT0 equals TOP for one timer clock. TOP = 0xFF (mode 1) or
OCR0A (mode 5). Non-inverting (COM0x[1:0] = 2): OC0x clears at the match
while counting up and sets at the match while counting down; inverting (3)
is the opposite. Lower maximum frequency than single slope, but symmetric,
so preferred for motor control. TOV0 is set each time the counter reaches
BOTTOM. COM0A[1:0] = 1 toggles OC0A at each match when WGM02 = 1 (not
available for OC0B).

```equation
f~OC0xPCPWM~ = f~clk_I/O~ / (N × 510)
```

OCR0A = BOTTOM gives a constant low output, OCR0A = MAX a constant high
(non-inverted; inverted is the opposite). OC0x can change without a compare
match, to keep the output symmetrical around BOTTOM, in two cases: OCR0x
changes from MAX (at MAX, OC0x equals the result of a down-counting match,
and must become that of an up-counting match), or the timer starts counting
from a value above OCR0x and so misses the up-counting match.

#### Timing

The timer is synchronous; clk~T0~ is drawn as a clock enable. Without
prescaling (count sequence near MAX, every mode except phase correct PWM),
TOV0 is set at the clock edge where TCNT0 goes from MAX to BOTTOM:

```wavedrom id=t0-timing caption="Timer/Counter timing, no prescaling"
{"signal": [
  {"name": "clkI/O", "wave": "p...."},
  {"name": "clkTn (clkI/O/1)", "wave": "1...."},
  {"name": "TCNTn", "wave": "=====", "data": ["MAX - 1", "MAX", "BOTTOM", "BOTTOM + 1", ""]},
  {"name": "TOVn", "wave": "0.1.."}
 ]}
```

With the prescaler (f~clk_I/O~/8) the sequence is the same, but TCNT0
changes, and TOV0 is set, only at the clk~I/O~ edge that ends a clk~T0~
enable pulse. OCF0B (every mode) and OCF0A (every mode but CTC and PWM
with OCR0A as TOP) are set when TCNT0 leaves the value equal to OCR0x:

```wavedrom id=t0-ocf caption="Setting of OCF0x, prescaler f~clk_I/O~/8"
{"signal": [
  {"name": "clkI/O", "wave": "p..............................."},
  {"name": "clkTn (clkI/O/8)", "wave": "10......10......10......10......"},
  {"name": "TCNTn", "wave": "x=.......=.......=.......=......", "data": ["OCRnx - 1", "OCRnx", "OCRnx + 1", "OCRnx + 2"]},
  {"name": "OCRnx", "wave": "=...............................", "data": ["OCRnx value"]},
  {"name": "OCFnx", "wave": "0................1.............."}
 ]}
```

In CTC mode, and fast PWM with OCR0A as TOP, OCF0A is set and TCNT0
cleared at the same timer clock:

```wavedrom id=t0-ctc caption="Clear timer on compare match, prescaler f~clk_I/O~/8"
{"signal": [
  {"name": "clkI/O", "wave": "p..............................."},
  {"name": "clkTn (clkI/O/8)", "wave": "10......10......10......10......"},
  {"name": "TCNTn (CTC)", "wave": "x=.......=.......=.......=......", "data": ["TOP - 1", "TOP", "BOTTOM", "BOTTOM + 1"]},
  {"name": "OCRnx", "wave": "=...............................", "data": ["TOP"]},
  {"name": "OCFnx", "wave": "0................1.............."}
 ]}
```

#### TCCR0A – Timer/Counter Control Register A, 0x24 (0x44)

```table id=t0-tccr0a caption="TCCR0A bits"
| Bit | Name | Description |
|---|---|---|
| 7:6 | COM0A[1:0] | Compare match output A mode (OC0A), per the mode tables below. Either bit set overrides the port function; DDR must make the pin an output. |
| 5:4 | COM0B[1:0] | Compare match output B mode (OC0B), as COM0A |
| 1:0 | WGM0[1:0] | Waveform generation mode, with WGM02 in TCCR0B: see [the mode table](#t0-wgm) |
```

```table id=t0-com-nonpwm caption="COM0x[1:0], normal and CTC modes (identical for A and B)"
| COM0x1 | COM0x0 | Description |
|:-:|:-:|---|
| 0 | 0 | Normal port operation, OC0x disconnected |
| 0 | 1 | Toggle OC0x on compare match |
| 1 | 0 | Clear OC0x on compare match |
| 1 | 1 | Set OC0x on compare match |
```

```table id=t0-com-fast caption="COM0x[1:0], fast PWM mode [1]"
| COM0x1 | COM0x0 | OC0A | OC0B |
|:-:|:-:|---|---|
| 0 | 0 | Normal port operation, OC0A disconnected | Normal port operation, OC0B disconnected |
| 0 | 1 | WGM02 = 0: normal port operation, OC0A disconnected. WGM02 = 1: toggle OC0A on compare match | Reserved |
| 1 | 0 | Clear OC0A on compare match, set at BOTTOM (non-inverting) | Clear OC0B on compare match, set at BOTTOM (non-inverting) |
| 1 | 1 | Set OC0A on compare match, clear at BOTTOM (inverting) | Set OC0B on compare match, clear at BOTTOM (inverting) |

[1] Special case: when OCR0x equals TOP and COM0x1 is set, the compare match is ignored, but the set or clear is done at BOTTOM (the source says TOP for OC0B).
```

```table id=t0-com-pc caption="COM0x[1:0], phase correct PWM mode [1]"
| COM0x1 | COM0x0 | OC0A | OC0B |
|:-:|:-:|---|---|
| 0 | 0 | Normal port operation, OC0A disconnected | Normal port operation, OC0B disconnected |
| 0 | 1 | WGM02 = 0: normal port operation, OC0A disconnected. WGM02 = 1: toggle OC0A on compare match | Reserved |
| 1 | 0 | Clear OC0A on compare match when up-counting, set when down-counting | Clear OC0B on compare match when up-counting, set when down-counting |
| 1 | 1 | Set OC0A on compare match when up-counting, clear when down-counting | Set OC0B on compare match when up-counting, clear when down-counting |

[1] Special case: when OCR0x equals TOP and COM0x1 is set, the compare match is ignored, but the set or clear is done at TOP.
```

```table id=t0-wgm caption="Waveform generation modes [1]"
| Mode | WGM02 | WGM01 | WGM00 | Mode of operation | TOP | Update of OCR0x at | TOV0 set on |
|--:|:-:|:-:|:-:|---|---|---|---|
| 0 | 0 | 0 | 0 | Normal | 0xFF | Immediate | MAX |
| 1 | 0 | 0 | 1 | PWM, phase correct | 0xFF | TOP | BOTTOM |
| 2 | 0 | 1 | 0 | CTC | OCRA | Immediate | MAX |
| 3 | 0 | 1 | 1 | Fast PWM | 0xFF | BOTTOM | MAX |
| 4 | 1 | 0 | 0 | Reserved | – | – | – |
| 5 | 1 | 0 | 1 | PWM, phase correct | OCRA | TOP | BOTTOM |
| 6 | 1 | 1 | 0 | Reserved | – | – | – |
| 7 | 1 | 1 | 1 | Fast PWM | OCRA | BOTTOM | TOP |

[1] MAX = 0xFF, BOTTOM = 0x00.
```

#### TCCR0B – Timer/Counter Control Register B, 0x25 (0x45)

```table id=t0-tccr0b caption="TCCR0B bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | FOC0A | W | 0 | Force output compare A: writing 1 forces an immediate compare match on the waveform generator; OC0A changes per COM0A[1:0]. Strobe, reads 0. Active only in non-PWM modes; write 0 when writing TCCR0B in a PWM mode. No interrupt, and no timer clear in CTC mode with OCR0A as TOP. |
| 6 | FOC0B | W | 0 | Force output compare B, as FOC0A for OC0B (no clear in CTC with OCR0B as TOP) |
| 3 | WGM02 | R/W | 0 | Waveform generation mode bit 2: see [the mode table](#t0-wgm) |
| 2:0 | CS0[2:0] | R/W | 0 | Clock select, below |
```

```table id=t0-cs caption="Clock select"
| CS02 | CS01 | CS00 | Clock |
|:-:|:-:|:-:|---|
| 0 | 0 | 0 | No clock source (Timer/Counter stopped) |
| 0 | 0 | 1 | clk~I/O~ (no prescaling) |
| 0 | 1 | 0 | clk~I/O~/8 (prescaler) |
| 0 | 1 | 1 | clk~I/O~/64 (prescaler) |
| 1 | 0 | 0 | clk~I/O~/256 (prescaler) |
| 1 | 0 | 1 | clk~I/O~/1024 (prescaler) |
| 1 | 1 | 0 | External clock on T0, falling edge |
| 1 | 1 | 1 | External clock on T0, rising edge |
```

With an external clock selected, transitions on T0 clock the counter even
when the pin is an output, so software can drive the count.

#### TCNT0 – Timer/Counter Register, 0x26 (0x46)

R/W, reset 0x00: direct read and write access to the 8-bit counter. A
write blocks the compare match on the following timer clock; changing
TCNT0 while the counter runs risks missing a compare match with OCR0x.

#### OCR0A – Output Compare Register A, 0x27 (0x47), and OCR0B – Output Compare Register B, 0x28 (0x48)

R/W, reset 0x00: 8-bit values continuously compared with TCNT0; a match
can raise an output compare interrupt or generate a waveform on OC0A or
OC0B.

#### TIMSK0 – Timer/Counter Interrupt Mask Register, (0x6E)

Each enable takes effect with the I bit in SREG set.

```table id=t0-timsk0 caption="TIMSK0 bits"
| Bit | Name | Description |
|---|---|---|
| 2 | OCIE0B | Output compare match B interrupt enable (executed when OCF0B is set) |
| 1 | OCIE0A | Output compare match A interrupt enable (executed when OCF0A is set) |
| 0 | TOIE0 | Overflow interrupt enable (executed when TOV0 is set) |
```

#### TIFR0 – Timer/Counter 0 Interrupt Flag Register, 0x15 (0x35)

Each flag is cleared by hardware when its interrupt vector executes, or by
writing 1 to it; its interrupt executes when the flag, its enable in TIMSK0
and the SREG I bit are all set.

```table id=t0-tifr0 caption="TIFR0 bits"
| Bit | Name | Description |
|---|---|---|
| 2 | OCF0B | Output compare B match flag: set on a match between TCNT0 and OCR0B |
| 1 | OCF0A | Output compare A match flag: set on a match between TCNT0 and OCR0A |
| 0 | TOV0 | Overflow flag: set on overflow; when depends on WGM0[2:0] ([mode table](#t0-wgm)) |
```

### Timer/Counter1

A 16-bit timer/counter with two output compare units (OCR1A/OCR1B,
double buffered in PWM modes), one input capture unit with a noise
canceler, and four interrupt sources (TOV1, OCF1A, OCF1B, ICF1). It
counts, generates PWM (variable period, glitch-free phase correct and
phase and frequency correct), generates frequencies and counts external
events. Write PRTIM1 in PRR to 0 to enable it.

Block diagram: the counter TCNT1 with its control logic (count, clear,
direction) and clock select (prescaler or the T1 pin through an edge
detector), two 16-bit comparators against OCR1A/OCR1B feeding the
waveform generators for OC1A/OC1B, and the input capture register ICR1
loaded from TCNT1 on an edge of ICP1 or the analog comparator output,
through the noise canceler and edge detector. TCNT1, OCR1A/B and ICR1 are
16-bit registers; TCCR1A/B/C are 8-bit with no access restrictions. All
interrupt requests are visible in TIFR1 and masked in TIMSK1. With no
clock source selected the timer is inactive; the selected clock is
clk~T1~.

TOP, the highest count, is in some modes one of the fixed values 0x00FF,
0x01FF or 0x03FF, or the value in OCR1A or ICR1. With OCR1A as TOP in a
PWM mode, OCR1A can't generate a PWM output, but TOP is then double
buffered and can be changed at run time; ICR1 as TOP gives a fixed TOP
and frees OCR1A for PWM output.

```table id=t1-defs caption="Definitions"
| Term | Meaning |
|---|---|
| BOTTOM | The counter reaches BOTTOM when it becomes 0x0000 |
| MAX | The counter reaches MAX when it becomes 0xFFFF (65535) |
| TOP | The counter reaches TOP when it equals the highest value of the count sequence: 0x00FF, 0x01FF, 0x03FF, OCR1A or ICR1, depending on the mode |
```

#### Accessing 16-bit registers

TCNT1, OCR1A/B and ICR1 are accessed one byte at a time through an 8-bit
temporary register (TEMP) for the high byte, shared by all 16-bit
registers of the timer. Accessing the low byte triggers the 16-bit
operation: writing the low byte copies TEMP (the high byte written
before) and the low byte into the 16-bit register in the same clock
cycle; reading the low byte copies the high byte into TEMP in the same
clock cycle. So **write the high byte first, read the low byte first**.
Reading OCR1A/B does not use TEMP. In C the compiler handles the order.

A 16-bit access is two instructions: if an interrupt between them
accesses any 16-bit register of the timer (and so TEMP), the access
outside the interrupt is corrupted. When both main code and interrupt
code use TEMP, the main code saves SREG, disables interrupts for the two
accesses and restores SREG. When several 16-bit registers are written
with the same high byte, the high byte need be written only once (the
same atomicity rule applies). (Code examples dropped.)

#### Timer/Counter1 clock sources and counter unit

CS1[2:0] in TCCR1B select an internal (prescaled) or external clock;
see [the prescalers](#timercounter0-and-timercounter1-prescalers).

The counter is TCNT1H (high byte) and TCNT1L (low byte). The CPU reaches
TCNT1H only through TEMP: TEMP is loaded with TCNT1H when TCNT1L is read,
and TCNT1H is written from TEMP when TCNT1L is written, so the full
16-bit value moves in one clock cycle. Some writes to TCNT1 while the
counter runs give unpredictable results; they are described under the
modes. Depending on the mode, the counter is cleared, incremented or
decremented on each clk~T1~. With CS1[2:0] = 0 the timer stops, but the
CPU can still access TCNT1; a CPU write has priority over clear and
count. WGM1[3:0] (TCCR1A and TCCR1B) set the counting sequence; TOV1 is
set as the mode defines and can raise an interrupt.

#### Input capture unit

On a level change (event) on ICP1, or on the analog comparator output
ACO, that matches the edge selected by ICES1, TCNT1 is copied into ICR1
and ICF1 is set in the same system clock. ICF1 raises the input capture
interrupt when ICIE1 = 1; it is cleared when the interrupt is executed,
or by writing 1 to it. Read ICR1 low byte (ICR1L) first; ICR1H then reads
TEMP. ICR1 can only be written in a waveform generation mode that uses
ICR1 as TOP; set WGM1[3:0] first, then write ICR1H before ICR1L.

- **Trigger source.** ICP1, or the analog comparator output when ACIC in
  ACSR is set. Changing the source can trigger a capture: clear ICF1
  after the change. ICP1 and ACO are sampled like the T1 pin, with the
  same edge detector. The noise canceler and edge detector input is
  always enabled, unless the mode uses ICR1 as TOP. A capture can be
  triggered by software by driving the ICP1 port pin.
- **Noise canceler** (ICNC1 = 1): the input must be equal for four
  successive samples to change the edge detector's input; this adds four
  system clock cycles of delay from the input change to the ICR1 update.
  It uses the system clock, not the prescaler.
- **Use.** If ICR1 isn't read before the next event, it is overwritten.
  Read ICR1 as early as possible in the interrupt handler; the worst-case
  response time depends on the longest other interrupt handler. Using
  input capture while TOP (resolution) is changed at run time is not
  recommended. To measure duty cycle, change the trigger edge right after
  reading ICR1, then clear ICF1 by writing 1; to measure frequency only,
  clearing ICF1 is not needed when an interrupt handler is used.

#### Output compare units

The 16-bit comparator compares TCNT1 with OCR1x continuously. A match
sets OCF1x at the next timer clock cycle and, with OCIE1x = 1, raises the
output compare interrupt; OCF1x is cleared when the interrupt is
executed, or by writing 1. The waveform generator uses the match, WGM1
and COM1x to drive OC1x; TOP and BOTTOM handle the extreme values. Unit A
can also define TOP, and so the counter resolution and waveform period.

OCR1x is double buffered in the twelve PWM modes (the update is
synchronized to TOP or BOTTOM, preventing odd-length, non-symmetrical
pulses) and not in Normal and CTC modes. With double buffering the CPU
accesses the OCR1x buffer, otherwise OCR1x itself. OCR1x only changes by
CPU write, so reading it does not use TEMP (reading the low byte first is
still good practice). Writes go through TEMP because all 16 bits are
compared continuously: write OCR1xH first (into TEMP), then OCR1xL, which
copies the high byte in the same system clock cycle.

- **Force output compare.** In non-PWM modes, writing 1 to FOC1x forces
  a match: OC1x changes per COM1x[1:0], but OCF1x is not set and the
  timer is not reloaded or cleared.
- **Compare match blocking.** Any CPU write to TCNT1 blocks a compare
  match in the next timer clock cycle, even with the timer stopped, so
  OCR1x can be set to the TCNT1 value without triggering an interrupt
  when the clock is enabled.
- **Using the unit.** Writing TCNT1 equal to OCR1x misses that match,
  giving a wrong waveform. Don't write TCNT1 equal to TOP in PWM modes
  with variable TOP (the TOP match is ignored and the counter runs to
  0xFFFF), nor equal to BOTTOM while counting down. Set up OC1x before
  setting the pin's DDR bit to output; the easiest way to set OC1x is a
  FOC1x strobe in Normal mode. OC1x keeps its value across mode changes.
  COM1x[1:0] are not double buffered: a change acts at once.

#### Timer/Counter1 compare match output unit

COM1x[1:0] define the OC1x state at the next compare match and select
the OC1x pin's output source. If either COM1x bit is set, OC1x overrides
the normal port function, but the pin direction still comes from its DDR
bit, which must be set to output for OC1x to be visible. The port
override is independent of the mode, with exceptions given in the COM
tables. Some COM1x settings are reserved in some modes. OC1x here means
the internal OC1x register (reset to 0), not the pin; the design allows
OC1x to be initialized before the output is enabled. COM1x has no effect
on the input capture unit. COM1x[1:0] = 0 means no action on the next
match. A COM1x change acts at the first compare match after the write;
in non-PWM modes a FOC1x strobe makes it act at once.

#### Timer/Counter1 modes of operation

WGM1[3:0] set the counting sequence; COM1x[1:0] set whether the output is
set, cleared or toggled (non-PWM) or inverted or not (PWM), and don't
affect counting. N below is the prescaler factor (1, 8, 64, 256 or 1024).

**Normal mode** (WGM1 = 0): counts up, no clear; overruns from MAX
(0xFFFF) to BOTTOM (0x0000). TOV1 is set in the timer clock cycle in
which TCNT1 becomes zero; it acts like a 17th bit that is only set, so
with the overflow interrupt (which clears it) software can extend the
resolution. A new counter value can be written any time. For input
capture the interval between events must not exceed the counter range
(extend it with the overflow interrupt or the prescaler). Generating
waveforms with output compare in Normal mode is not recommended (it
takes too much CPU time).

**CTC mode** (WGM1 = 4 or 12): the counter is cleared when TCNT1 matches
OCR1A (mode 4) or ICR1 (mode 12), which define TOP. OCF1A or ICF1
(whichever register is TOP) can interrupt at each TOP, and the handler
may update TOP. CTC has no double buffering: writing a TOP lower than
the current TCNT1 misses the match, and the counter runs to 0xFFFF and
wraps first. Fast PWM with OCR1A as TOP (mode 15), which is double
buffered, avoids this. For a waveform, set COM1A[1:0] = 1 (toggle) and
DDR_OC1A = 1; the highest frequency is f~clk_I/O~/2, with OCR1A = 0.
TOV1 is set in the timer clock cycle in which the counter goes from MAX
to 0x0000.

```equation
f~OC1A~ = f~clk_I/O~ / (2 × N × (1 + OCR1A))
```

**Fast PWM mode** (WGM1 = 5, 6, 7, 14 or 15): single slope, BOTTOM to TOP
then BOTTOM, so up to twice the frequency of the dual-slope modes.
Non-inverting (COM1x = 2): OC1x cleared on the compare match, set at
BOTTOM; inverting (COM1x = 3): set on the match, cleared at BOTTOM. TOP
is 0x00FF, 0x01FF or 0x03FF (modes 5, 6, 7), ICR1 (14) or OCR1A (15);
the counter is cleared at the timer clock cycle after it matches TOP.
Resolution from 2 bits (TOP = 0x0003) to 16 bits (TOP = MAX). TOV1 is set
at each TOP; OCF1A or ICF1 (the TOP register) is set in the same timer
clock cycle. A new TOP must be ≥ every compare register, or no compare
match occurs; with fixed TOP, the unused OCR1x bits are masked to zero on
write. ICR1 isn't double buffered: a new ICR1 below the current TCNT1
misses TOP and the counter runs to 0xFFFF and wraps. OCR1A is: the write
goes to the buffer, copied into OCR1A at the next TOP match, in the same
timer clock cycle as TCNT1 is cleared and TOV1 set. ICR1 as TOP suits a
fixed TOP (OCR1A stays free for PWM); OCR1A as TOP suits a changing base
frequency. OCR1x = BOTTOM gives a narrow spike every TOP + 1 timer clocks;
OCR1x = TOP gives a constant high or low output, by COM1x. With OCR1A as
TOP (mode 15) and COM1A = 1, OC1A toggles at each match: a 50 % duty
waveform up to f~clk_I/O~/2 (OCR1A = 0), like CTC toggle but double
buffered.

```equation
R~FPWM~ = log(TOP + 1) / log(2)
f~OC1xPWM~ = f~clk_I/O~ / (N × (1 + TOP))
```

**Phase correct PWM mode** (WGM1 = 1, 2, 3, 10 or 11): dual slope,
BOTTOM (0x0000) to TOP and back; TCNT1 equals TOP for one timer clock
cycle. Non-inverting (COM1x = 2): OC1x cleared on the match while
counting up, set on the match while counting down; inverting (COM1x = 3)
the opposite. Lower maximum frequency than single slope; the symmetry
suits motor control. TOP is 0x00FF, 0x01FF or 0x03FF (modes 1, 2, 3),
ICR1 (10) or OCR1A (11); resolution 2 to 16 bits. TOV1 is set at BOTTOM;
with OCR1A or ICR1 as TOP, OCF1A or ICF1 is set at TOP, in the same timer
clock cycle as OCR1x is updated from its buffer. A new TOP must be ≥
every compare register; with fixed TOP the unused OCR1x bits are masked
on write. Because OCR1x updates at TOP, the period runs TOP to TOP: the
falling slope takes the old TOP and the rising slope the new one, so
changing TOP while running gives an unsymmetrical period. Use phase and
frequency correct mode when TOP changes at run time; with a static TOP
the two modes are practically the same. OCR1x = BOTTOM gives a
continuously low output and OCR1x = TOP continuously high (non-inverting;
inverted for COM1x = 3). With OCR1A as TOP (mode 11) and COM1A = 1, OC1A
toggles with 50 % duty.

```equation
R~PCPWM~ = log(TOP + 1) / log(2)
f~OC1xPCPWM~ = f~clk_I/O~ / (2 × N × TOP)
```

**Phase and frequency correct PWM mode** (WGM1 = 8 or 9): dual slope like
phase correct, but OCR1x is updated from the buffer at BOTTOM, so both
slopes always have the same length and every period is symmetrical
(frequency correct). TOP is ICR1 (mode 8) or OCR1A (mode 9); resolution
2 to 16 bits. TOV1 is set at BOTTOM, in the same timer clock cycle as the
OCR1x update; OCF1A or ICF1 (the TOP register) is set at TOP. A new TOP
must be ≥ every compare register. ICR1 as TOP suits a fixed TOP, OCR1A as
TOP a changing base frequency (double buffered). Output polarity,
extreme values and the mode 9 50 % toggle (COM1A = 1) are as in phase
correct mode.

```equation
R~PFCPWM~ = log(TOP + 1) / log(2)
f~OC1xPFCPWM~ = f~clk_I/O~ / (2 × N × TOP)
```

#### Timer/Counter1 timing

The timer is synchronous; clk~T1~ is a clock enable. OCF1x is set one
timer clock after TCNT1 equals OCR1x. Near TOP, CTC and fast PWM go
TOP → BOTTOM, the dual-slope modes TOP → TOP − 1. TOV1 (fast PWM) and
ICF1 (ICR1 as TOP) are set, and OCR1x updated, on the timer clock that
leaves TOP. In phase and frequency correct mode, and in modes that set
TOV1 at BOTTOM, read BOTTOM for TOP, BOTTOM + 1 for TOP − 1 and so on.
With the prescaler (f~clk_I/O~/8) the same happens, but only on the
clk~I/O~ cycles where clk~T1~ is enabled.

```wavedrom id=t1-ocf caption="Setting of OCF1x, no prescaling"
{"signal": [
  {"name": "clkI/O", "wave": "p...."},
  {"name": "clkT1", "wave": "p...."},
  {"name": "TCNT1", "wave": "=====", "data": ["OCR1x-1", "OCR1x", "OCR1x+1", "OCR1x+2", ""]},
  {"name": "OCR1x", "wave": "=....", "data": ["OCR1x value"]},
  {"name": "OCF1x", "wave": "0.1.."}
 ],
 "config": {"hscale": 2}}
```

```wavedrom id=t1-top caption="Count sequence at TOP, no prescaling"
{"signal": [
  {"name": "clkI/O", "wave": "p...."},
  {"name": "TCNT1 (CTC, fast PWM)", "wave": "====.", "data": ["TOP-1", "TOP", "BOTTOM", "BOTTOM+1"]},
  {"name": "TCNT1 (PC, PFC PWM)", "wave": "====.", "data": ["TOP-1", "TOP", "TOP-1", "TOP-2"]},
  {"name": "TOV1 (fast PWM), ICF1 (ICR1 as TOP)", "wave": "0.1.."},
  {"name": "OCR1x (update at TOP)", "wave": "=.=..", "data": ["old OCR1x", "new OCR1x"]}
 ],
 "config": {"hscale": 2}}
```

#### TCCR1A – Timer/Counter1 Control Register A, (0x80)

```table id=t1-tccr1a caption="TCCR1A bits"
| Bit | Name | Description |
|---|---|---|
| 7:6 | COM1A[1:0] | Compare output mode, channel A: see the COM tables |
| 5:4 | COM1B[1:0] | Compare output mode, channel B: see the COM tables |
| 1:0 | WGM1[1:0] | Waveform generation mode, with WGM1[3:2] in TCCR1B: see the mode table |
```

If either bit of COM1A (COM1B) is 1, OC1A (OC1B) overrides the normal
port function of its pin; the pin's DDR bit must still be set to enable
the output driver. The function of COM1x depends on WGM1[3:0]:

```table id=t1-com-nonpwm caption="Compare output mode, non-PWM (Normal, CTC); same for COM1A and COM1B"
| COM1x1 | COM1x0 | Description |
|:-:|:-:|---|
| 0 | 0 | Normal port operation, OC1x disconnected |
| 0 | 1 | Toggle OC1x on compare match |
| 1 | 0 | Clear OC1x on compare match (output low) |
| 1 | 1 | Set OC1x on compare match (output high) |
```

```table id=t1-com-fpwm caption="Compare output mode, fast PWM [1]"
| COM1x1 | COM1x0 | Description |
|:-:|:-:|---|
| 0 | 0 | Normal port operation, OC1A/OC1B disconnected |
| 0 | 1 | WGM1 = 14 or 15: toggle OC1A on compare match, OC1B disconnected (normal port operation). Other WGM1 settings: normal port operation, OC1A/OC1B disconnected. |
| 1 | 0 | Clear OC1x on compare match, set OC1x at BOTTOM (non-inverting) |
| 1 | 1 | Set OC1x on compare match, clear OC1x at BOTTOM (inverting) |

[1] Special case: when OCR1x equals TOP and COM1x1 is set, the compare match is ignored, but the set or clear is done at BOTTOM.
```

```table id=t1-com-pcpwm caption="Compare output mode, phase correct and phase and frequency correct PWM [1]"
| COM1x1 | COM1x0 | Description |
|:-:|:-:|---|
| 0 | 0 | Normal port operation, OC1A/OC1B disconnected |
| 0 | 1 | WGM1 = 9 or 11: toggle OC1A on compare match, OC1B disconnected (normal port operation). Other WGM1 settings: normal port operation, OC1A/OC1B disconnected. |
| 1 | 0 | Clear OC1x on compare match when up-counting, set OC1x on compare match when down-counting |
| 1 | 1 | Set OC1x on compare match when up-counting, clear OC1x on compare match when down-counting |

[1] A special case occurs when OCR1x equals TOP and COM1x1 is set (see phase correct PWM mode).
```

```table id=t1-wgm caption="Waveform generation modes [1]"
| Mode | WGM13 | WGM12 (CTC1) | WGM11 (PWM11) | WGM10 (PWM10) | Mode of operation | TOP | Update of OCR1x at | TOV1 set on |
|--:|:-:|:-:|:-:|:-:|---|---|---|---|
| 0 | 0 | 0 | 0 | 0 | Normal | 0xFFFF | Immediate | MAX |
| 1 | 0 | 0 | 0 | 1 | PWM, phase correct, 8-bit | 0x00FF | TOP | BOTTOM |
| 2 | 0 | 0 | 1 | 0 | PWM, phase correct, 9-bit | 0x01FF | TOP | BOTTOM |
| 3 | 0 | 0 | 1 | 1 | PWM, phase correct, 10-bit | 0x03FF | TOP | BOTTOM |
| 4 | 0 | 1 | 0 | 0 | CTC | OCR1A | Immediate | MAX |
| 5 | 0 | 1 | 0 | 1 | Fast PWM, 8-bit | 0x00FF | BOTTOM | TOP |
| 6 | 0 | 1 | 1 | 0 | Fast PWM, 9-bit | 0x01FF | BOTTOM | TOP |
| 7 | 0 | 1 | 1 | 1 | Fast PWM, 10-bit | 0x03FF | BOTTOM | TOP |
| 8 | 1 | 0 | 0 | 0 | PWM, phase and frequency correct | ICR1 | BOTTOM | BOTTOM |
| 9 | 1 | 0 | 0 | 1 | PWM, phase and frequency correct | OCR1A | BOTTOM | BOTTOM |
| 10 | 1 | 0 | 1 | 0 | PWM, phase correct | ICR1 | TOP | BOTTOM |
| 11 | 1 | 0 | 1 | 1 | PWM, phase correct | OCR1A | TOP | BOTTOM |
| 12 | 1 | 1 | 0 | 0 | CTC | ICR1 | Immediate | MAX |
| 13 | 1 | 1 | 0 | 1 | Reserved | – | – | – |
| 14 | 1 | 1 | 1 | 0 | Fast PWM | ICR1 | BOTTOM | TOP |
| 15 | 1 | 1 | 1 | 1 | Fast PWM | OCR1A | BOTTOM | TOP |

[1] The names CTC1 and PWM11:0 are obsolete; use WGM12:0. Function and location are compatible with earlier versions of the timer.
```

#### TCCR1B – Timer/Counter1 Control Register B, (0x81)

```table id=t1-tccr1b caption="TCCR1B bits"
| Bit | Name | Description |
|---|---|---|
| 7 | ICNC1 | Input capture noise canceler: 1 = ICP1 filtered, four successive equal samples needed to change the output; the capture is delayed by four oscillator cycles |
| 6 | ICES1 | Input capture edge select: 0 = falling edge, 1 = rising edge triggers a capture (TCNT1 copied into ICR1, ICF1 set). When ICR1 is TOP, ICP1 is disconnected and input capture is disabled. |
| 4:3 | WGM1[3:2] | Waveform generation mode: see the mode table |
| 2:0 | CS1[2:0] | Clock select: see the table below |
```

```table id=t1-cs caption="Clock select"
| CS12 | CS11 | CS10 | Clock |
|:-:|:-:|:-:|---|
| 0 | 0 | 0 | None (Timer/Counter stopped) |
| 0 | 0 | 1 | clk~I/O~/1 (no prescaling) |
| 0 | 1 | 0 | clk~I/O~/8 (from prescaler) |
| 0 | 1 | 1 | clk~I/O~/64 (from prescaler) |
| 1 | 0 | 0 | clk~I/O~/256 (from prescaler) |
| 1 | 0 | 1 | clk~I/O~/1024 (from prescaler) |
| 1 | 1 | 0 | External clock on T1, falling edge |
| 1 | 1 | 1 | External clock on T1, rising edge |
```

With an external clock selected, transitions on T1 clock the counter even
when the pin is an output, so software can drive the count.

#### TCCR1C – Timer/Counter1 Control Register C, (0x82)

```table id=t1-tccr1c caption="TCCR1C bits"
| Bit | Name | Description |
|---|---|---|
| 7 | FOC1A | Force output compare, channel A |
| 6 | FOC1B | Force output compare, channel B |
```

FOC1A/FOC1B act only in non-PWM modes. Writing 1 forces an immediate
compare match on the waveform generator; OC1x changes per COM1x[1:0].
They are strobes, always read 0, and generate no interrupt nor clear the
timer in CTC mode with OCR1A as TOP.

#### TCNT1H and TCNT1L – Timer/Counter1, (0x85) (0x84)

TCNT1 (R/W, reset 0): read and write access to the 16-bit counter,
through TEMP. Changing TCNT1 while the counter runs risks missing a
compare match; a write blocks the compare match on the next timer clock
for all compare units.

#### OCR1AH/OCR1AL (0x89) (0x88), OCR1BH/OCR1BL (0x8B) (0x8A) – Output Compare Registers 1 A and B

OCR1A, OCR1B (R/W, reset 0): 16-bit values compared continuously with
TCNT1; a match can raise an output compare interrupt or drive OC1x.
Written through TEMP.

#### ICR1H and ICR1L – Input Capture Register 1, (0x87) (0x86)

ICR1 (R/W, reset 0): loaded with TCNT1 at each event on ICP1 (or the
analog comparator output); can define TOP. Read through TEMP.

#### TIMSK1 – Timer/Counter1 Interrupt Mask Register, (0x6F)

```table id=t1-timsk1 caption="TIMSK1 bits"
| Bit | Name | Description |
|---|---|---|
| 5 | ICIE1 | Input capture interrupt enable (with the I flag in SREG): vector runs when ICF1 is set |
| 2 | OCIE1B | Output compare B match interrupt enable: vector runs when OCF1B is set |
| 1 | OCIE1A | Output compare A match interrupt enable: vector runs when OCF1A is set |
| 0 | TOIE1 | Overflow interrupt enable: vector runs when TOV1 is set |
```

#### TIFR1 – Timer/Counter1 Interrupt Flag Register, 0x16 (0x36)

Each flag is cleared when its interrupt vector is executed, or by
writing 1 to it.

```table id=t1-tifr1 caption="TIFR1 bits"
| Bit | Name | Description |
|---|---|---|
| 5 | ICF1 | Input capture flag: set by a capture event on ICP1; when ICR1 is TOP, set when the counter reaches TOP |
| 2 | OCF1B | Output compare B match flag: set in the timer clock cycle after TCNT1 matches OCR1B; not set by a FOC1B strobe |
| 1 | OCF1A | Output compare A match flag: set in the timer clock cycle after TCNT1 matches OCR1A; not set by a FOC1A strobe |
| 0 | TOV1 | Overflow flag: set on overflow in Normal and CTC modes, otherwise as the mode table's "TOV1 set on" column |
```

### Timer/Counter0 and Timer/Counter1 prescalers

Timer/Counter0 and Timer/Counter1 share one prescaler module but can use
different prescaler settings.

**Internal clock.** CSn[2:0] = 1 clocks the timer directly from the
system clock: the fastest operation, f~clk_I/O~. Otherwise one of four
prescaler taps is used: f~clk_I/O~/8, /64, /256 or /1024.

**Prescaler reset.** The prescaler runs free, independent of the timers'
clock select, and is shared by Timer/Counter1 and Timer/Counter0. So when
a timer is enabled on a prescaled clock (6 > CSn[2:0] > 1), the first
count comes 1 to N + 1 system clock cycles later (N = 8, 64, 256 or
1024). A prescaler reset can synchronize a timer to program execution,
but it changes the prescaler period of every timer connected to it.

**External clock.** A clock on T1/T0 can be clk~T1~/clk~T0~. The pin is
sampled once per system clock by the synchronization logic (registers
clocked on the rising clk~I/O~ edge, a latch transparent in the clk~I/O~
high period), then passed to the edge detector, which gives one clk~Tn~
pulse per rising (CSn[2:0] = 7) or falling (CSn[2:0] = 6) edge. The
synchronization and edge detection delay the counter update by 2.5 to
3.5 system clock cycles after the edge on the pin. Enable or disable the
clock input only when T1/T0 has been stable for at least one system clock
cycle, or a false clock pulse may be generated. Each half period of the
external clock must be longer than one system clock cycle: f~ExtClk~ <
f~clk_I/O~/2 at 50 % duty cycle (Nyquist). Because of oscillator
tolerances (crystal, resonator, capacitors), keep the external clock below
f~clk_I/O~/2.5. An external clock can't be prescaled.

#### GTCCR – General Timer/Counter Control Register (TSM, PSRSYNC), 0x23 (0x43)

```table id=psc-gtccr caption="GTCCR bits"
| Bit | Name | Description |
|---|---|---|
| 7 | TSM | Timer/counter synchronization mode: 1 = the values written to PSRASY and PSRSYNC are kept, holding the prescaler resets asserted, so the timers stay halted and can be configured to the same value without one advancing. Writing 0 clears PSRASY and PSRSYNC by hardware and the timers start counting together. |
| 1 | PSRASY | Prescaler reset, Timer/Counter2: see [Timer/Counter2](#timercounter2) |
| 0 | PSRSYNC | Prescaler reset, Timer/Counter1 and Timer/Counter0: 1 resets their shared prescaler (affecting both). Normally cleared at once by hardware, except while TSM is set. |
```

### Timer/Counter2

An 8-bit timer/counter with two output compare units (OCR2A/OCR2B,
double buffered in PWM modes), a 10-bit prescaler, overflow and compare
match interrupts (TOV2, OCF2A, OCF2B), and asynchronous clocking from a
32 kHz watch crystal independent of the I/O clock. It does CTC (auto
reload), glitch-free phase correct PWM and frequency generation. Write
PRTIM2 in PRR to 0 to enable it.

Block diagram: as Timer/Counter0, with the clock select fed by a
prescaler whose input is clk~I/O~ or the timer/counter oscillator on
TOSC1/TOSC2. TCNT2, OCR2A and OCR2B are 8-bit registers; interrupt
requests are visible in TIFR2 and masked in TIMSK2. ASSR controls the
asynchronous operation. With no clock source selected the timer is
inactive; the selected clock is clk~T2~. OCR2A/OCR2B are compared with
TCNT2 at all times; a match can drive OC2A/OC2B (PWM or variable
frequency) and sets OCF2A/OCF2B.

```table id=t2-defs caption="Definitions"
| Term | Meaning |
|---|---|
| BOTTOM | The counter reaches BOTTOM when it becomes 0x00 |
| MAX | The counter reaches MAX when it becomes 0xFF (255) |
| TOP | The counter reaches TOP when it equals the highest value of the count sequence: 0xFF (MAX) or OCR2A, depending on the mode |
```

**Clock sources.** clk~T2~ defaults to the MCU clock clk~I/O~; with AS2 =
1 in ASSR it comes from the timer/counter oscillator on TOSC1/TOSC2.

**Counter unit.** Depending on the mode, the counter is cleared,
incremented or decremented on each clk~T2~, selected by CS2[2:0]. With
CS2[2:0] = 0 the timer stops, but the CPU can still access TCNT2; a CPU
write has priority over clear and count. WGM2[1:0] (TCCR2A) and WGM22
(TCCR2B) set the counting sequence; TOV2 is set as the mode defines and
can raise an interrupt.

**Output compare unit.** The 8-bit comparator compares TCNT2 with OCR2A
and OCR2B continuously. A match sets OCF2x at the next timer clock cycle
and, if enabled, raises the output compare interrupt; OCF2x is cleared
when the interrupt is executed, or by writing 1. The waveform generator
uses the match, WGM2[2:0] and COM2x[1:0]; the max and bottom signals
handle the extreme values. OCR2x is double buffered in the PWM modes
(the update is synchronized to TOP or BOTTOM, preventing odd-length,
non-symmetrical pulses) and not in Normal and CTC modes; with double
buffering the CPU accesses the OCR2x buffer, otherwise OCR2x itself.

- **Force output compare.** In non-PWM modes, writing 1 to FOC2x forces
  a match: OC2x changes per COM2x[1:0], but OCF2x is not set and the
  timer is not reloaded or cleared.
- **Compare match blocking.** Any CPU write to TCNT2 blocks a compare
  match in the next timer clock cycle, even with the timer stopped, so
  OCR2x can be set to the TCNT2 value without triggering an interrupt
  when the clock is enabled.
- **Using the unit.** Writing TCNT2 equal to OCR2x misses that match,
  giving a wrong waveform; don't write TCNT2 equal to BOTTOM while
  counting down. Set up OC2x before setting the pin's DDR bit to output;
  the easiest way to set OC2x is a FOC2x strobe in Normal mode. OC2x
  keeps its value across mode changes. COM2x[1:0] are not double
  buffered: a change acts at once.

**Compare match output unit.** COM2x[1:0] define the OC2x state (the
internal OC2x register, not the pin) at the next compare match and select
the OC2x pin's output source. If either COM2x bit is set, OC2x overrides
the normal port function, but the pin direction still comes from its DDR
bit, which must be set to output for OC2x to be visible. The port override
is independent of the mode. OC2x can be initialized before the output is
enabled. Some COM2x settings are reserved in some modes. COM2x[1:0] = 0
means no action on the next match; a COM2x change acts at the first
compare match after the write, or at once with a FOC2x strobe (non-PWM
modes).

#### Timer/Counter2 modes of operation

WGM2[2:0] set the counting sequence; COM2x[1:0] set whether the output is
set, cleared or toggled (non-PWM) or inverted or not (PWM), and don't
affect counting. N below is the prescale factor (1, 8, 32, 64, 128, 256 or
1024).

**Normal mode** (WGM2 = 0): counts up, no clear; overruns from 0xFF to
0x00. TOV2 is set in the timer clock cycle in which TCNT2 becomes zero;
it acts like a ninth bit that is only set, so with the overflow interrupt
(which clears it) software can extend the resolution. A new counter value
can be written any time. Generating waveforms with output compare in
Normal mode is not recommended (it takes too much CPU time).

**CTC mode** (WGM2 = 2): the counter is cleared when TCNT2 matches OCR2A,
which defines TOP. OCF2A can interrupt at each TOP, and the handler may
update TOP. CTC has no double buffering: writing an OCR2A lower than the
current TCNT2 misses the match, and the counter runs to 0xFF and wraps
first. For a waveform, set COM2A[1:0] = 1 (toggle) and the pin's DDR bit;
the highest frequency is f~clk_I/O~/2, with OCR2A = 0. TOV2 is set in the
timer clock cycle in which the counter goes from MAX to 0x00.

```equation
f~OC2x~ = f~clk_I/O~ / (2 × N × (1 + OCR2x))
```

**Fast PWM mode** (WGM2 = 3 or 7): single slope, BOTTOM to TOP then
BOTTOM, up to twice the frequency of phase correct mode. TOP is 0xFF
(mode 3) or OCR2A (mode 7); the counter is cleared at the timer clock
cycle after it matches TOP. Non-inverting (COM2x = 2): OC2x cleared on
the compare match, set at BOTTOM (when the counter goes from TOP to
BOTTOM); inverting (COM2x = 3): set on the match, cleared at BOTTOM. TOV2
is set at each TOP; the handler may update the compare value. OCR2A =
BOTTOM gives a narrow spike every MAX + 1 timer clocks; OCR2A = MAX gives a
constant high or low output, by COM2A. With COM2x = 1 (toggle, OC2A with
WGM22 = 1) OC2x toggles at each match: a 50 % duty waveform up to
f~clk_I/O~/2 (OCR2A = 0), like CTC toggle but double buffered.

```equation
f~OC2xPWM~ = f~clk_I/O~ / (N × 256)
```

**Phase correct PWM mode** (WGM2 = 1 or 5): dual slope, BOTTOM to TOP
and back; TOP is 0xFF (mode 1) or OCR2A (mode 5);
TCNT2 equals TOP for one timer clock cycle.
Non-inverting (COM2x = 2): OC2x cleared on the match while counting up,
set on the match while counting down; inverting (COM2x = 3) the opposite.
Lower maximum frequency than single slope; the symmetry suits motor
control. TOV2 is set at BOTTOM and can interrupt there. OCR2A = BOTTOM
gives a continuously low output and OCR2A = MAX continuously high
(non-inverting; inverted for COM2x = 3). OC2x can change without a
compare match, to keep the output symmetrical around BOTTOM, in two
cases: OCR2A changes from MAX (at MAX the pin equals the result of a
down-counting match, so it must take the result of an up-counting one),
and the timer starts counting from a value above OCR2A, missing the
match (and the OC2x change) on the way up.

```equation
f~OC2xPCPWM~ = f~clk_I/O~ / (N × 510)
```

#### Timer/Counter2 timing

The timer clock clk~T2~ is a clock enable; in asynchronous mode read the
timer/counter oscillator clock for clk~I/O~. TOV2 is set as TCNT2 goes
from MAX to BOTTOM (all modes but phase correct PWM). OCF2A is set one
timer clock after TCNT2 equals OCR2A (all modes but CTC). In CTC mode
OCF2A is set and TCNT2 cleared as TCNT2 leaves TOP. With the prescaler
(f~clk_I/O~/8) the same happens, on the clk~I/O~ cycles where clk~T2~ is
enabled.

```wavedrom id=t2-tov caption="TOV2 at MAX, no prescaling"
{"signal": [
  {"name": "clkI/O", "wave": "p...."},
  {"name": "clkT2", "wave": "p...."},
  {"name": "TCNT2", "wave": "====.", "data": ["MAX-1", "MAX", "BOTTOM", "BOTTOM+1"]},
  {"name": "TOV2", "wave": "0.1.."}
 ],
 "config": {"hscale": 2}}
```

```wavedrom id=t2-ctc caption="OCF2A and TCNT2 clear in CTC mode"
{"signal": [
  {"name": "clkT2", "wave": "p...."},
  {"name": "TCNT2", "wave": "====.", "data": ["TOP-1", "TOP", "BOTTOM", "BOTTOM+1"]},
  {"name": "OCR2A", "wave": "=....", "data": ["TOP"]},
  {"name": "OCF2A", "wave": "0.1.."}
 ],
 "config": {"hscale": 2}}
```

#### Asynchronous operation

1. **Switching clock source.** Changing between asynchronous and
   synchronous clocking may corrupt TCNT2, OCR2x and TCCR2x. Safe
   procedure: (a) disable the Timer/Counter2 interrupts (clear OCIE2x and
   TOIE2); (b) select the clock source with AS2; (c) write new values to
   TCNT2, OCR2x and TCCR2x; (d) when switching to asynchronous operation,
   wait for TCN2UB, OCR2xUB and TCR2xUB to clear; (e) clear the
   Timer/Counter2 interrupt flags; (f) enable interrupts if needed.
2. The CPU main clock must be more than four times the oscillator
   frequency.
3. A write to TCNT2, OCR2x or TCCR2x goes to a temporary register, latched
   after two rising edges on TOSC1. Don't write a new value before the
   transfer is done. Each of the five registers has its own temporary
   register (a TCNT2 write doesn't disturb an OCR2x write in progress);
   ASSR shows when a transfer is done.
4. Before entering Power-save or ADC noise reduction mode after writing
   TCNT2, OCR2x or TCCR2x, wait until the written register is updated if
   Timer/Counter2 is to wake the device; otherwise the MCU sleeps before
   the change takes effect. This matters most for the output compare
   wake-up: output compare is disabled while OCR2x or TCNT2 is written,
   so if the MCU sleeps before OCR2xUB returns to 0, no compare interrupt
   comes and the MCU never wakes.
5. After waking from Power-save or ADC noise reduction by
   Timer/Counter2, re-entering sleep within the same TOSC1 cycle makes the
   interrupt occur again at once: multiple interrupts and wake-ups within
   one TOSC1 cycle. To be sure one TOSC1 cycle has elapsed: write a value
   to TCCR2x, TCNT2 or OCR2x, wait for its update busy flag in ASSR to
   return to 0, then enter the sleep mode.
6. With asynchronous operation selected, the 32.768 kHz oscillator always
   runs, except in Power-down and Standby. After a power-up reset or a
   wake-up from Power-down or Standby it may take up to one second to
   stabilize: wait at least one second before using Timer/Counter2. All
   Timer/Counter2 registers must be considered lost after a wake-up from
   Power-down or Standby (unstable clock at start-up), whether the
   oscillator or a clock on TOSC1 is used.
7. **Wake-up** from Power-save or ADC noise reduction with the timer
   clocked asynchronously: the wake-up starts on the timer clock cycle
   after the interrupt condition, so the timer has always advanced by at
   least one before the CPU can read it. The MCU is then halted for four
   cycles, runs the interrupt routine and resumes after the SLEEP
   instruction.
8. **Reading TCNT2** shortly after a wake-up from Power-save may give a
   wrong result: TCNT2 is read through a register synchronized to clk~I/O~
   on every rising TOSC1 edge, so after wake-up it reads the value from
   before sleep until the next rising TOSC1 edge, whose phase is
   unpredictable. Procedure: write any value to OCR2x or TCCR2x, wait for
   its update busy flag to clear, then read TCNT2.
9. The interrupt flags take 3 processor cycles plus one timer cycle to
   synchronize, so the timer has advanced by at least one before the CPU
   can read the value that set the flag. The output compare pin changes on
   the timer clock, not synchronized to the processor clock.

#### Timer/Counter2 prescaler

The Timer/Counter2 clock source is clk~T2S~: clk~I/O~ by default, or
TOSC1 when AS2 = 1 in ASSR, which makes Timer/Counter2 a real-time
counter. With AS2 set, TOSC1 and TOSC2 are disconnected from Port B and a
crystal between them is an independent clock for Timer/Counter2; the
oscillator is optimized for a 32.768 kHz crystal. The 10-bit prescaler
offers clk~T2S~/8, /32, /64, /128, /256 and /1024, plus clk~T2S~ and 0
(stop). PSRASY in GTCCR resets the prescaler, for a predictable
prescaler.

#### TCCR2A – Timer/Counter Control Register A, (0xB0)

```table id=t2-tccr2a caption="TCCR2A bits"
| Bit | Name | Description |
|---|---|---|
| 7:6 | COM2A[1:0] | Compare match output A mode: see the COM2A tables |
| 5:4 | COM2B[1:0] | Compare match output B mode: see the COM2B tables |
| 1:0 | WGM2[1:0] | Waveform generation mode, with WGM22 in TCCR2B: see the mode table |
```

If either bit of COM2A (COM2B) is set, OC2A (OC2B) overrides the normal
port function of its pin; the pin's DDR bit must still be set to enable
the output driver. The function depends on WGM2[2:0].

```table id=t2-com-nonpwm caption="Compare output mode, non-PWM (Normal, CTC); same for COM2A and COM2B"
| COM2x1 | COM2x0 | Description |
|:-:|:-:|---|
| 0 | 0 | Normal port operation, OC2x disconnected |
| 0 | 1 | Toggle OC2x on compare match |
| 1 | 0 | Clear OC2x on compare match |
| 1 | 1 | Set OC2x on compare match |
```

```table id=t2-com-fpwm caption="Compare output mode, fast PWM [1]"
| COM2x1 | COM2x0 | OC2A | OC2B |
|:-:|:-:|---|---|
| 0 | 0 | Normal port operation, OC2A disconnected | Normal port operation, OC2B disconnected |
| 0 | 1 | WGM22 = 0: normal port operation, OC2A disconnected. WGM22 = 1: toggle OC2A on compare match. | Reserved |
| 1 | 0 | Clear OC2A on compare match, set OC2A at BOTTOM (non-inverting) | Clear OC2B on compare match, set OC2B at BOTTOM (non-inverting) |
| 1 | 1 | Set OC2A on compare match, clear OC2A at BOTTOM (inverting) | Set OC2B on compare match, clear OC2B at BOTTOM (inverting) |

[1] Special case: when OCR2x equals TOP and COM2x1 is set, the compare match is ignored, but the set or clear is done at BOTTOM.
```

```table id=t2-com-pcpwm caption="Compare output mode, phase correct PWM [1]"
| COM2x1 | COM2x0 | OC2A | OC2B |
|:-:|:-:|---|---|
| 0 | 0 | Normal port operation, OC2A disconnected | Normal port operation, OC2B disconnected |
| 0 | 1 | WGM22 = 0: normal port operation, OC2A disconnected. WGM22 = 1: toggle OC2A on compare match. | Reserved |
| 1 | 0 | Clear OC2A on compare match when up-counting, set when down-counting | Clear OC2B on compare match when up-counting, set when down-counting |
| 1 | 1 | Set OC2A on compare match when up-counting, clear when down-counting | Set OC2B on compare match when up-counting, clear when down-counting |

[1] Special case: when OCR2x equals TOP and COM2x1 is set, the compare match is ignored, but the set or clear is done at TOP.
```

```table id=t2-wgm caption="Waveform generation modes [1]"
| Mode | WGM22 | WGM21 | WGM20 | Mode of operation | TOP | Update of OCR2x at | TOV2 set on |
|--:|:-:|:-:|:-:|---|---|---|---|
| 0 | 0 | 0 | 0 | Normal | 0xFF | Immediate | MAX |
| 1 | 0 | 0 | 1 | PWM, phase correct | 0xFF | TOP | BOTTOM |
| 2 | 0 | 1 | 0 | CTC | OCR2A | Immediate | MAX |
| 3 | 0 | 1 | 1 | Fast PWM | 0xFF | BOTTOM | MAX |
| 4 | 1 | 0 | 0 | Reserved | – | – | – |
| 5 | 1 | 0 | 1 | PWM, phase correct | OCR2A | TOP | BOTTOM |
| 6 | 1 | 1 | 0 | Reserved | – | – | – |
| 7 | 1 | 1 | 1 | Fast PWM | OCR2A | BOTTOM | TOP |

[1] MAX = 0xFF, BOTTOM = 0x00.
```

#### TCCR2B – Timer/Counter Control Register B, (0xB1)

```table id=t2-tccr2b caption="TCCR2B bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | FOC2A | W | 0 | Force output compare A |
| 6 | FOC2B | W | 0 | Force output compare B |
| 3 | WGM22 | R/W | 0 | Waveform generation mode: see the mode table |
| 2:0 | CS2[2:0] | R/W | 0 | Clock select: see the table below |
```

FOC2A/FOC2B act only in non-PWM modes; write them 0 when writing TCCR2B
in a PWM mode (compatibility). Writing 1 forces an immediate compare match
on the waveform generator; OC2x changes per COM2x[1:0]. They are strobes,
always read 0, and generate no interrupt nor clear the timer in CTC mode.

```table id=t2-cs caption="Clock select"
| CS22 | CS21 | CS20 | Clock |
|:-:|:-:|:-:|---|
| 0 | 0 | 0 | None (Timer/Counter stopped) |
| 0 | 0 | 1 | clk~T2S~ (no prescaling) |
| 0 | 1 | 0 | clk~T2S~/8 |
| 0 | 1 | 1 | clk~T2S~/32 |
| 1 | 0 | 0 | clk~T2S~/64 |
| 1 | 0 | 1 | clk~T2S~/128 |
| 1 | 1 | 0 | clk~T2S~/256 |
| 1 | 1 | 1 | clk~T2S~/1024 |
```

#### TCNT2 (0xB2), OCR2A (0xB3), OCR2B (0xB4) – Timer/Counter2 and Output Compare Registers

TCNT2 (R/W, reset 0) gives read and write access to the 8-bit counter; a
write blocks the compare match on the following timer clock, and changing
TCNT2 while running risks missing a compare match. OCR2A and OCR2B (R/W,
reset 0) are compared continuously with TCNT2; a match can raise an
output compare interrupt or drive OC2A/OC2B.

#### TIMSK2 – Timer/Counter2 Interrupt Mask Register, (0x70)

```table id=t2-timsk2 caption="TIMSK2 bits"
| Bit | Name | Description |
|---|---|---|
| 2 | OCIE2B | Compare match B interrupt enable (with the I bit in SREG): runs when OCF2B is set |
| 1 | OCIE2A | Compare match A interrupt enable: runs when OCF2A is set |
| 0 | TOIE2 | Overflow interrupt enable: runs when TOV2 is set |
```

#### TIFR2 – Timer/Counter2 Interrupt Flag Register, 0x17 (0x37)

Each flag is cleared by hardware when its interrupt vector is executed,
or by writing 1 to it.

```table id=t2-tifr2 caption="TIFR2 bits"
| Bit | Name | Description |
|---|---|---|
| 2 | OCF2B | Output compare flag B: set on a compare match between TCNT2 and OCR2B |
| 1 | OCF2A | Output compare flag A: set on a compare match between TCNT2 and OCR2A |
| 0 | TOV2 | Overflow flag: set on an overflow; in PWM mode, set when the counter changes direction at 0x00 |
```

#### ASSR – Asynchronous Status Register, (0xB6)

```table id=t2-assr caption="ASSR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6 | EXCLK | R/W | 0 | Enable external clock input: 1 (with the asynchronous clock selected) enables the external clock input buffer, so a clock can drive TOSC1 instead of a 32 kHz crystal. Write it before selecting asynchronous operation; the crystal oscillator only runs while EXCLK = 0. |
| 5 | AS2 | R/W | 0 | Asynchronous Timer/Counter2: 0 = clocked from clk~I/O~, 1 = from the crystal oscillator on TOSC1. Changing AS2 may corrupt TCNT2, OCR2A, OCR2B, TCCR2A and TCCR2B. |
| 4 | TCN2UB | R | 0 | TCNT2 update busy |
| 3 | OCR2AUB | R | 0 | OCR2A update busy |
| 2 | OCR2BUB | R | 0 | OCR2B update busy |
| 1 | TCR2AUB | R | 0 | TCCR2A update busy |
| 0 | TCR2BUB | R | 0 | TCCR2B update busy |
```

In asynchronous operation, writing TCNT2, OCR2A, OCR2B, TCCR2A or TCCR2B
sets its update busy flag; hardware clears it when the register has been
updated from its temporary register (0 = ready for a new value). Writing a
register while its busy flag is set may corrupt the value and cause an
unintended interrupt. Reading TCNT2 returns the actual timer value;
reading OCR2A, OCR2B, TCCR2A or TCCR2B returns the temporary register.

#### GTCCR – General Timer/Counter Control Register (PSRASY), 0x23 (0x43)

PSRASY (bit 1, R/W, reset 0): 1 resets the Timer/Counter2 prescaler.
Normally cleared at once by hardware; written in asynchronous mode, it
stays 1 until the prescaler has been reset; not cleared while TSM is set.
TSM and PSRSYNC: see [the prescalers](#timercounter0-and-timercounter1-prescalers).

### SPI

A full-duplex, three-wire synchronous serial port, master or slave,
MSB or LSB first, seven master bit rates, with an end-of-transfer
interrupt flag, write-collision flag and wake-up from Idle mode. PRSPI
in PRR must be 0 to enable it. The USART can also act as an SPI master
(see [USART in SPI mode](#usart-in-spi-mode)). The SPI port is also used for
serial programming of the Flash and EEPROM.

Block diagram: an 8-bit shift register with a receive buffer, the clock
logic (f~osc~ divider /2…/128, SPI2X), pin control logic and the SPCR,
SPSR, SPDR registers. Master and slave connect MOSI to MOSI, MISO to
MISO and SCK to SCK, forming one 16-bit circular shift register; the
master drives the slave's !{SS}.

**Master.** The SPI does not drive !{SS}: software pulls the slave's
!{SS} low first. Writing SPDR starts the clock generator; eight bits are
shifted, the clock stops and SPIF is set (interrupt if SPIE = 1). The
master writes the next byte to SPDR, or ends the packet by raising
!{SS}. The last incoming byte stays in the receive buffer.

**Slave.** While !{SS} is high the SPI sleeps with MISO tri-stated;
software may update SPDR, but nothing is shifted until !{SS} goes low.
After a byte, SPIF is set (interrupt if SPIE = 1). The slave may write
the next byte to SPDR before reading the received one. The last incoming
byte stays in the receive buffer.

Transmit is single-buffered (SPDR can't be written until the shift
completes); receive is double-buffered (read the received byte before
the next one is completely shifted in, or the first is lost). In slave
mode SCK is sampled by the CPU clock: its low and high periods must each
be longer than 2 CPU clock cycles.

```table id=spi-pins caption="SPI pin direction overrides when the SPI is enabled [1]"
| Pin | Master | Slave |
|---|---|---|
| MOSI | User defined | Input |
| MISO | Input | User defined |
| SCK | User defined | Input |
| !{SS} | User defined | Input |

[1] User-defined directions are set with the DDRB bits (see the Port B alternate functions).
```

**!{SS}, slave mode.** !{SS} is always an input. Low activates the SPI
and MISO becomes an output if so configured; all other pins are inputs.
High makes all pins inputs and the SPI passive: it immediately resets
the send and receive logic and drops a partly received byte, which keeps
the slave's bit counter in step with the master.

**!{SS}, master mode** (MSTR = 1): as an output, !{SS} is a general
output pin that doesn't affect the SPI. As an input it must be held
high. If it is driven low, the SPI takes this as another master
selecting it: MSTR is cleared (the SPI becomes a slave; MOSI and SCK
become inputs) and SPIF is set, with an interrupt if enabled. An
interrupt-driven master should therefore check that MSTR is still set,
and set it again if a slave select cleared it.

#### Data modes

CPOL and CPHA give four combinations of SCK phase and polarity. Data is
shifted out and latched in on opposite SCK edges.

```table id=spi-modes caption="SPI modes"
| SPI mode | CPOL | CPHA | Leading edge | Trailing edge |
|--:|:-:|:-:|---|---|
| 0 | 0 | 0 | Sample (rising) | Setup (falling) |
| 1 | 0 | 1 | Setup (rising) | Sample (falling) |
| 2 | 1 | 0 | Sample (falling) | Setup (rising) |
| 3 | 1 | 1 | Setup (falling) | Sample (rising) |
```

Bit order on MOSI and MISO: MSB, bit 6 … bit 1, LSB with DORD = 0;
LSB, bit 1 … bit 6, MSB with DORD = 1.

```wavedrom id=spi-cpha0 caption="SPI transfer format, CPHA = 0 (modes 0 and 2), DORD = 0"
{"signal": [
  {"name": "SS#", "wave": "10.......1"},
  {"name": "SCK (CPOL = 0)", "wave": "0.hlhl|hl."},
  {"name": "SCK (CPOL = 1)", "wave": "1.lhlh|lh."},
  {"name": "MOSI/MISO", "wave": "x=.=.=|=.x", "data": ["MSB", "bit 6", "…", "LSB"]}
 ],
 "config": {"hscale": 2}}
```

```wavedrom id=spi-cpha1 caption="SPI transfer format, CPHA = 1 (modes 1 and 3), DORD = 0"
{"signal": [
  {"name": "SS#", "wave": "10........1"},
  {"name": "SCK (CPOL = 0)", "wave": "0.hlhl|hl.."},
  {"name": "SCK (CPOL = 1)", "wave": "1.lhlh|lh.."},
  {"name": "MOSI/MISO", "wave": "x.=.=.|=.x.", "data": ["MSB", "bit 6", "LSB"]}
 ],
 "config": {"hscale": 2}}
```

#### SPCR – SPI Control Register, 0x2C (0x4C)

```table id=spi-spcr caption="SPCR bits"
| Bit | Name | Description |
|---|---|---|
| 7 | SPIE | SPI interrupt enable: interrupt when SPIF = 1 and the I bit in SREG is set |
| 6 | SPE | SPI enable; must be 1 for any SPI operation |
| 5 | DORD | Data order: 0 = MSB first, 1 = LSB first |
| 4 | MSTR | 1 = master, 0 = slave. Cleared (and SPIF set) when !{SS} is an input and driven low in master mode |
| 3 | CPOL | Clock polarity: 0 = SCK low when idle (leading edge rising), 1 = SCK high when idle (leading edge falling) |
| 2 | CPHA | Clock phase: 0 = sample on the leading edge, setup on the trailing edge; 1 = setup on the leading edge, sample on the trailing edge |
| 1:0 | SPR[1:0] | SCK rate in master mode, with SPI2X (table below); no effect in slave mode |
```

```table id=spi-rate caption="SCK frequency (master)"
| SPI2X | SPR1 | SPR0 | SCK frequency |
|:-:|:-:|:-:|---|
| 0 | 0 | 0 | f~osc~/4 |
| 0 | 0 | 1 | f~osc~/16 |
| 0 | 1 | 0 | f~osc~/64 |
| 0 | 1 | 1 | f~osc~/128 |
| 1 | 0 | 0 | f~osc~/2 |
| 1 | 0 | 1 | f~osc~/8 |
| 1 | 1 | 0 | f~osc~/32 |
| 1 | 1 | 1 | f~osc~/64 |
```

#### SPSR – SPI Status Register, 0x2D (0x4D)

```table id=spi-spsr caption="SPSR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | SPIF | R | 0 | SPI interrupt flag: set when a transfer completes, or when !{SS} (input) is driven low in master mode. Cleared by executing the interrupt vector, or by reading SPSR with SPIF set and then accessing SPDR |
| 6 | WCOL | R | 0 | Write collision: set when SPDR is written during a transfer. Cleared (with SPIF) by reading SPSR with WCOL set and then accessing SPDR |
| 0 | SPI2X | R/W | 0 | Double SPI speed in master mode (minimum SCK period two CPU clocks). As a slave the SPI is only guaranteed to work at f~osc~/4 or lower |
```

#### SPDR – SPI Data Register, 0x2E (0x4E)

Read/write, reset undefined. Writing starts a transmission; reading
returns the shift register's receive buffer.

#### SPI timing characteristics

The source numbers the parameters 1–18 instead of naming them; the
diagrams use the numbers. t~ck~ = CPU clock period, t~sck~ = SCK period.

```table id=spi-timing caption="SPI timing parameters"
| No. | Parameter | Mode | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| 1 | SCK period | Master | | [1] | | ns |
| 2 | SCK high/low [3] | Master | | | | ns |
| 3 | Rise/fall time | Master | | 3.6 | | ns |
| 4 | Setup | Master | | 10 | | ns |
| 5 | Hold | Master | | 10 | | ns |
| 6 | Out to SCK | Master | | 0.5 × t~sck~ | | ns |
| 7 | SCK to out | Master | | 10 | | ns |
| 8 | SCK to out high | Master | | 10 | | ns |
| 9 | !{SS} low to out | Slave | | 15 | | ns |
| 10 | SCK period | Slave | 4 × t~ck~ | | | ns |
| 11 | SCK high/low [2] | Slave | 2 × t~ck~ | | | ns |
| 12 | Rise/fall time | Slave | | | 1600 | ns |
| 13 | Setup | Slave | 10 | | | ns |
| 14 | Hold | Slave | t~ck~ | | | ns |
| 15 | SCK to out | Slave | | 15 | | ns |
| 16 | SCK to !{SS} high | Slave | 20 | | | ns |
| 17 | !{SS} high to tri-state | Slave | | 10 | | ns |
| 18 | !{SS} low to SCK | Slave | 20 | | | ns |

[1] Set by SPR1, SPR0 and SPI2X (see the SCK frequency table).
[2] In SPI programming mode the minimum SCK high/low period is 2 t~CLCL~ for f~CK~ < 12 MHz and 3 t~CLCL~ for f~CK~ > 12 MHz.
[3] 50 % duty cycle.
```

```wavedrom id=spi-tim-master caption="SPI timing, master mode (numbers refer to the SPI timing table)"
{"signal": [
  {"name": "SS#", "wave": "10.......", "node": ".A"},
  {"name": "SCK (CPOL = 0)", "wave": "0.1.0|1.0", "node": "..B...C.D"},
  {"name": "MISO", "wave": "x.=x.|=x.", "data": ["MSB", "LSB"], "node": "..EF"},
  {"name": "MOSI", "wave": "x=...|.=.", "data": ["MSB", "LSB"], "node": ".......G"}
 ],
 "edge": ["A~>B 6", "E-F 4,5", "C<->D 2", "C~>G 8"],
 "config": {"hscale": 2}}
```

```wavedrom id=spi-tim-slave caption="SPI timing, slave mode (numbers refer to the SPI timing table)"
{"signal": [
  {"name": "SS#", "wave": "10.......1", "node": ".A.......J"},
  {"name": "SCK (CPOL = 0)", "wave": "0.1.0|1.0.", "node": "..B...C.D"},
  {"name": "MOSI", "wave": "x.=x.|=x..", "data": ["MSB", "LSB"], "node": "..EF"},
  {"name": "MISO", "wave": "z=...|.=z.", "data": ["MSB", "X"], "node": ".H......K"}
 ],
 "edge": ["A~>H 9", "A~>B 18", "E-F 13,14", "C<->D 11", "D~>J 16", "J~>K 17"],
 "config": {"hscale": 2}}
```

### USART0

A full-duplex universal synchronous and asynchronous serial receiver
and transmitter: asynchronous or synchronous (master or slave clocked)
operation, a 12-bit baud rate generator, frames of 5 to 9 data bits and
1 or 2 stop bits, hardware odd/even parity generation and check, data
overrun and framing error detection, noise filtering (false start bit
detection, digital low-pass filter), three interrupts (TX complete, TX
data register empty, RX complete), multi-processor communication mode
and double-speed asynchronous mode. It can also run as an SPI master
(see [USART in SPI mode](#usart-in-spi-mode)). PRUSART0 in PRR must be 0.

Block diagram: three parts sharing the control registers UCSR0A–C. The
clock generator holds the baud rate generator (UBRR0) and the
synchronization logic for an external XCK0 clock (synchronous slave).
The transmitter has a single write buffer (so frames can follow without
delay), a shift register, a parity generator and frame-format control
driving TxD0. The receiver has clock and data recovery units (for
asynchronous reception), a parity checker, a shift register and a
two-level receive buffer (UDR0), and detects frame error, data overrun
and parity errors.

#### Clock generation

Four clock modes: normal asynchronous, double-speed asynchronous,
synchronous master and synchronous slave. UMSEL0 (UCSR0C) selects
asynchronous or synchronous; U2X0 (UCSR0A) selects double speed
(asynchronous only). In synchronous mode the XCK0 pin's DDR bit selects
an internal clock (master, XCK0 output) or an external one (slave, XCK0
input); XCK0 is only active in synchronous mode.

**Baud rate generator** (asynchronous and synchronous master): a
down-counter running at f~osc~, reloaded from UBRR0 each time it reaches
zero or when UBRR0L is written; it produces a clock at
f~osc~/(UBRR0 + 1). The transmitter divides this by 2, 8 or 16
depending on mode; the receiver's recovery units use it directly, with
a state machine of 2, 8 or 16 states set by UMSEL0, U2X0 and the XCK0
DDR bit.

```table id=usart-baud-eq caption="Baud rate equations (BAUD in bit/s; UBRR0 = 0–4095)"
| Mode | Baud rate | UBRR0 value |
|---|---|---|
| Asynchronous normal (U2X0 = 0) | BAUD = f~osc~ / (16 × (UBRR0 + 1)) | UBRR0 = f~osc~ / (16 × BAUD) – 1 |
| Asynchronous double speed (U2X0 = 1) | BAUD = f~osc~ / (8 × (UBRR0 + 1)) | UBRR0 = f~osc~ / (8 × BAUD) – 1 |
| Synchronous master | BAUD = f~osc~ / (2 × (UBRR0 + 1)) | UBRR0 = f~osc~ / (2 × BAUD) – 1 |
```

**Double speed** (U2X0 = 1) halves the divisor from 16 to 8, doubling
the asynchronous rate; the receiver then takes only 8 samples per bit
for data sampling and clock recovery, so baud rate and system clock must
be more accurate. No downside for the transmitter. Write U2X0 = 0 in
synchronous mode.

**External clock** (synchronous slave): XCK0 passes a synchronization
register and an edge detector, a delay of two CPU clocks, which limits
its frequency:

```equation
f~XCK~ < f~osc~ / 4
```

f~osc~ depends on the stability of the system clock; leave some margin.

**Synchronous clock operation** (UMSEL0 = 1): XCK0 is the clock input
(slave) or output (master). Data input (RxD0) is sampled on the opposite
XCK0 edge from the one on which data output (TxD0) changes. UCPOL0 = 0:
TxD0 changes on the rising edge, RxD0 is sampled on the falling edge;
UCPOL0 = 1: the reverse.

```wavedrom id=usart-xck caption="Synchronous mode XCK0 timing, UCPOL0 = 0 (with UCPOL0 = 1 the XCK0 edges are inverted)"
{"signal": [
  {"name": "XCK0", "wave": "0.1.0.1.0.1.0", "node": "....A...B"},
  {"name": "TxD0 / RxD0", "wave": "x.=...=...=..", "data": ["bit n", "bit n+1", "bit n+2"]}
 ],
 "config": {"hscale": 2}}
```

Data changes on the rising XCK0 edges and is sampled on the falling
edges (A, B).

#### Frame formats

A frame is 1 start bit, 5–9 data bits (LSB first), no/even/odd parity
bit, and 1 or 2 stop bits: all 30 combinations are valid. A frame may be
followed directly by the next, or the line idles high. UCSZ0[2:0],
UPM0[1:0] and USBS0 set the format for both receiver and transmitter;
changing them corrupts ongoing communication in both directions. The
receiver ignores the second stop bit, so a frame error is only detected
when the first stop bit is 0.

```wavedrom id=usart-frame caption="Frame format (bracketed bits optional); then the next start bit or idle (high)"
{"signal": [
  {"name": "TxD0 / RxD0", "wave": "1=============1", "data": ["St", "0", "1", "2", "3", "4", "[5]", "[6]", "[7]", "[8]", "[P]", "Sp1", "[Sp2]"]}
 ],
 "config": {"hscale": 2}}
```

St = start bit, always low; 0–8 = data bits; P = parity bit; Sp = stop
bit, always high; idle line high.

The parity bit is the exclusive OR of the data bits, inverted for odd
parity, placed between the last data bit and the first stop bit:

```equation
P~even~ = d~n–1~ ⊕ … ⊕ d~3~ ⊕ d~2~ ⊕ d~1~ ⊕ d~0~ ⊕ 0
P~odd~ = d~n–1~ ⊕ … ⊕ d~3~ ⊕ d~2~ ⊕ d~1~ ⊕ d~0~ ⊕ 1
```

#### Initialization

Set the baud rate (UBRR0), set the frame format (UCSR0C, UCSZ02) and
enable the transmitter and/or receiver (UCSR0B). For interrupt-driven
operation, clear the global interrupt flag during initialization. Before
re-initializing with a new baud rate or frame format, make sure no
transmission is in progress: TXC0 shows the transmitter has finished
(clear it before each UDR0 write if used for this), RXC0 shows the
receive buffer has no unread data.

#### Transmitter

TXEN0 = 1 enables the transmitter; it overrides TxD0 (and, in
synchronous mode, XCK0, used as the transmit clock). Set baud rate,
mode and frame format first.

**5 to 8 data bits.** Writing UDR0 loads the transmit buffer; it moves to
the shift register when that is idle or right after the last stop bit of
the previous frame, and the frame is sent at the rate set by UBRR0, U2X0
or XCK0. Unused high bits of UDR0 are ignored. Wait for UDRE0 = 1 before
writing.

**9 data bits** (UCSZ0 = 7): write the ninth bit to TXB80 before writing
the low byte to UDR0. The ninth bit can mark an address frame
(multi-processor mode) or serve other protocol purposes.

**Flags and interrupts.** UDRE0 = 1 when the transmit buffer is empty and
can take new data; it is cleared by writing UDR0. With UDRIE0 = 1 the
data register empty interrupt runs as long as UDRE0 is set: its routine
must write UDR0 or disable the interrupt, or it re-enters at once. TXC0
is set when the whole frame has left the shift register and the buffer
is empty; it is cleared by executing the TX complete interrupt
(TXCIE0 = 1) or by writing 1 to it. TXC0 suits half-duplex buses
(RS-485), where the transmitter must release the bus right after the
last frame.

**Parity generator**: with UPM01 = 1, the parity bit is inserted between
the last data bit and the first stop bit.

**Disabling** (TXEN0 = 0) takes effect only when the shift register and
the transmit buffer hold no more data; then TxD0 returns to port
function.

#### Receiver

RXEN0 = 1 enables the receiver; it overrides RxD0 (XCK0 is the clock in
synchronous mode). Reception starts at a valid start bit; each following
bit is sampled at the baud rate or XCK0 clock and shifted in up to the
first stop bit (a second stop bit is ignored). The complete frame then
moves to the receive buffer, read through UDR0. With fewer than 8 data
bits the unused high bits read 0.

**9 data bits**: read RXB80 (UCSR0B), and the status flags FE0, DOR0
and UPE0 (UCSR0A), before reading UDR0: reading UDR0 advances the
receive FIFO, which holds RXB80, FE0, DOR0 and UPE0 with each frame.
Reading all of them before processing frees the buffer as early as
possible.

**RXC0** is 1 while the receive buffer holds unread data, 0 when empty;
disabling the receiver flushes the buffer and clears RXC0. With
RXCIE0 = 1 the RX complete interrupt runs as long as RXC0 is set: its
routine must read UDR0, or it re-enters at once.

**Error flags** FE0, DOR0 and UPE0 are stored in the receive buffer with
their frame, so read UCSR0A before UDR0. Software can't set them; write
them 0 whenever UCSR0A is written. None generates an interrupt.
- FE0: the first stop bit of the next readable frame was 0 (1 when the
  stop bit was correctly read as 1). Useful for detecting loss of
  synchronization, break conditions and protocol handling. Not affected
  by USBS0.
- DOR0: data overrun: the receive buffer was full (two characters), a
  new character waited in the shift register, and a new start bit was
  detected. One or more frames were lost between the last frame read and
  the next. Cleared when a received frame moves from the shift register
  to the buffer.
- UPE0: the next frame in the buffer had a parity error, with parity
  checking enabled (UPM01 = 1) at the time. Always 0 when parity checking
  is off. Valid until UDR0 is read.

**Parity checker**: active with UPM01 = 1; UPM00 selects odd or even. It
computes the parity of incoming data bits, compares it with the received
parity bit and stores the result in the buffer with the frame (UPE0).

**Disabling** the receiver is immediate: a frame being received is lost,
RxD0 returns to port function and the receive buffer is flushed
(unread data lost). To flush the buffer in normal operation, read UDR0
until RXC0 is 0.

#### Asynchronous data reception

Clock recovery synchronizes the internal baud clock to incoming frames;
data recovery samples and low-pass filters each bit. The operating range
depends on the baud clock accuracy, the incoming rate and the frame
length.

**Clock recovery.** The receiver samples RxD0 at 16 times the baud rate
(normal) or 8 times (double speed). A high-to-low transition starts
start-bit detection; sample 1 is the first low sample. Samples 8, 9 and
10 (normal) or 4, 5 and 6 (double speed) decide: if two or three are
high, the start bit is rejected as a spike and the receiver waits for
the next transition; otherwise the clock recovery is synchronized and
data recovery begins. This repeats for every start bit. Double speed has
a larger synchronization uncertainty.

**Data recovery.** A state machine with 16 states per bit (normal) or 8
(double speed) samples each bit. The bit value is the majority of the
three centre samples (8, 9, 10 normal; 4, 5, 6 double speed); this acts
as a low-pass filter. The stop bit is voted the same way; a 0 sets FE0.
The next frame's start bit may begin right after the last voting sample
of the stop bit: in normal mode the first low sample may be the one after
sample 10 (point A of the source figure); in double speed mode the first
low level must be delayed to a later point (B), still before the end of a
full-length stop bit (C).
This early start-bit detection affects the operating range.

**Operating range.** The ratio of the slowest and fastest acceptable
incoming data rate to the receiver baud rate:

```equation
R~slow~ = (D + 1) × S / (S – 1 + D × S + S~F~)
R~fast~ = (D + 2) × S / ((D + 1) × S + S~M~)
```

D = character size plus parity (5 to 10 bits); S = samples per bit
(16 normal, 8 double speed); S~F~ = first sample used for voting (8
normal, 4 double speed); S~M~ = middle voting sample (9 normal, 5 double
speed).

```table id=usart-range caption="Maximum receiver baud rate error [1]"
| D (data + parity bits) | Mode | R~slow~ (%) | R~fast~ (%) | Max total error (%) | Recommended max receiver error (%) |
|--:|---|--:|--:|---|--:|
| 5 | Normal (U2X0 = 0) | 93.20 | 106.67 | +6.67/-6.8 | ±3.0 |
| 6 | ^ | 94.12 | 105.79 | +5.79/-5.88 | ±2.5 |
| 7 | ^ | 94.81 | 105.11 | +5.11/-5.19 | ±2.0 |
| 8 | ^ | 95.36 | 104.58 | +4.58/-4.54 | ±2.0 |
| 9 | ^ | 95.81 | 104.14 | +4.14/-4.19 | ±1.5 |
| 10 | ^ | 96.17 | 103.78 | +3.78/-3.83 | ±1.5 |
| 5 | Double speed (U2X0 = 1) | 94.12 | 105.66 | +5.66/-5.88 | ±2.5 |
| 6 | ^ | 94.92 | 104.92 | +4.92/-5.08 | ±2.0 |
| 7 | ^ | 95.52 | 104.35 | +4.35/-4.48 | ±1.5 |
| 8 | ^ | 96.00 | 103.90 | +3.90/-4.00 | ±1.5 |
| 9 | ^ | 96.39 | 103.53 | +3.53/-3.61 | ±1.5 |
| 10 | ^ | 96.70 | 103.23 | +3.23/-3.30 | ±1.0 |

[1] The recommended receiver error assumes receiver and transmitter share the maximum total error equally.
```

Normal speed tolerates more baud rate error. The receiver's error has
two sources: system clock instability over supply and temperature
(rarely a problem with a crystal; a resonator may be off by more than
2 %), and the baud rate generator not dividing the system clock exactly
(pick a UBRR0 value with a low enough error).

#### Multi-processor communication mode

MPCM0 = 1 makes the receiver drop frames that carry no address: with 5–8
data bits the first stop bit marks the frame type, with 9 data bits the
ninth bit (RXB80) does; 1 = address frame, 0 = data frame. The
transmitter is unaffected. A master uses 9-bit frames (UCSZ0 = 7) with
TXB80 = 1 for address and 0 for data frames; the slaves also use 9-bit
frames.

1. All slaves set MPCM0.
2. The master sends an address frame; every slave receives it (RXC0 set
   as normal).
3. Each slave reads UDR0; the addressed slave clears MPCM0, the others
   keep it set and wait for the next address.
4. The addressed slave receives the data frames; the others ignore them.
5. After the last data frame the addressed slave sets MPCM0 again and
   waits for the next address frame (back to step 2).

5- to 8-bit frames work but are impractical: the receiver must switch
between n and n+1 bit frames, which hinders full duplex since receiver
and transmitter share the character size, and the transmitter must use
two stop bits (USBS0 = 1). Don't set or clear MPCM0 with SBI or CBI: it
shares the location with TXC0, which they may clear by accident.

#### Examples of baud rate setting

```equation
Error [%] = (BaudRate~closest match~ / BaudRate – 1) × 100 %
```

Higher errors are usable, but the receiver's noise resistance drops,
especially for long frames (see the operating range above).

```table id=usart-baud-a caption="UBRR0 settings and baud rate error, f~osc~ = 1.0000, 1.8432 and 2.0000 MHz [1]"
| Baud rate | f~osc~ = 1.0000 MHz | < | < | < | f~osc~ = 1.8432 MHz | < | < | < | f~osc~ = 2.0000 MHz | < | < | < |
| ^ | U2X0 = 0 | < | U2X0 = 1 | < | U2X0 = 0 | < | U2X0 = 1 | < | U2X0 = 0 | < | U2X0 = 1 | < |
| ^ | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) |
|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2400 | 25 | 0.2 | 51 | 0.2 | 47 | 0.0 | 95 | 0.0 | 51 | 0.2 | 103 | 0.2 |
| 4800 | 12 | 0.2 | 25 | 0.2 | 23 | 0.0 | 47 | 0.0 | 25 | 0.2 | 51 | 0.2 |
| 9600 | 6 | -7.0 | 12 | 0.2 | 11 | 0.0 | 23 | 0.0 | 12 | 0.2 | 25 | 0.2 |
| 14.4k | 3 | 8.5 | 8 | -3.5 | 7 | 0.0 | 15 | 0.0 | 8 | -3.5 | 16 | 2.1 |
| 19.2k | 2 | 8.5 | 6 | -7.0 | 5 | 0.0 | 11 | 0.0 | 6 | -7.0 | 12 | 0.2 |
| 28.8k | 1 | 8.5 | 3 | 8.5 | 3 | 0.0 | 7 | 0.0 | 3 | 8.5 | 8 | -3.5 |
| 38.4k | 1 | -18.6 | 2 | 8.5 | 2 | 0.0 | 5 | 0.0 | 2 | 8.5 | 6 | -7.0 |
| 57.6k | 0 | 8.5 | 1 | 8.5 | 1 | 0.0 | 3 | 0.0 | 1 | 8.5 | 3 | 8.5 |
| 76.8k |  |  | 1 | -18.6 | 1 | -25.0 | 2 | 0.0 | 1 | -18.6 | 2 | 8.5 |
| 115.2k |  |  | 0 | 8.5 | 0 | 0.0 | 1 | 0.0 | 0 | 8.5 | 1 | 8.5 |
| 230.4k |  |  |  |  |  |  | 0 | 0.0 |  |  |  |  |
| 250k |  |  |  |  |  |  |  |  |  |  | 0 | 0.0 |
| Max. [2] | 62.5 kbps | < | 125 kbps | < | 115.2 kbps | < | 230.4 kbps | < | 125 kbps | < | 250 kbps | < |

[1] Baud rate in bps (k = 1000, M = 10^6^). Empty cells: rate not reachable. The source prints settings with less than 0.5 % error in bold.
[2] Maximum rate, UBRR0 = 0, error 0.0 %.
```

```table id=usart-baud-b caption="UBRR0 settings and baud rate error, f~osc~ = 3.6864, 4.0000 and 7.3728 MHz [1]"
| Baud rate | f~osc~ = 3.6864 MHz | < | < | < | f~osc~ = 4.0000 MHz | < | < | < | f~osc~ = 7.3728 MHz | < | < | < |
| ^ | U2X0 = 0 | < | U2X0 = 1 | < | U2X0 = 0 | < | U2X0 = 1 | < | U2X0 = 0 | < | U2X0 = 1 | < |
| ^ | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) |
|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2400 | 95 | 0.0 | 191 | 0.0 | 103 | 0.2 | 207 | 0.2 | 191 | 0.0 | 383 | 0.0 |
| 4800 | 47 | 0.0 | 95 | 0.0 | 51 | 0.2 | 103 | 0.2 | 95 | 0.0 | 191 | 0.0 |
| 9600 | 23 | 0.0 | 47 | 0.0 | 25 | 0.2 | 51 | 0.2 | 47 | 0.0 | 95 | 0.0 |
| 14.4k | 15 | 0.0 | 31 | 0.0 | 16 | 2.1 | 34 | -0.8 | 31 | 0.0 | 63 | 0.0 |
| 19.2k | 11 | 0.0 | 23 | 0.0 | 12 | 0.2 | 25 | 0.2 | 23 | 0.0 | 47 | 0.0 |
| 28.8k | 7 | 0.0 | 15 | 0.0 | 8 | -3.5 | 16 | 2.1 | 15 | 0.0 | 31 | 0.0 |
| 38.4k | 5 | 0.0 | 11 | 0.0 | 6 | -7.0 | 12 | 0.2 | 11 | 0.0 | 23 | 0.0 |
| 57.6k | 3 | 0.0 | 7 | 0.0 | 3 | 8.5 | 8 | -3.5 | 7 | 0.0 | 15 | 0.0 |
| 76.8k | 2 | 0.0 | 5 | 0.0 | 2 | 8.5 | 6 | -7.0 | 5 | 0.0 | 11 | 0.0 |
| 115.2k | 1 | 0.0 | 3 | 0.0 | 1 | 8.5 | 3 | 8.5 | 3 | 0.0 | 7 | 0.0 |
| 230.4k | 0 | 0.0 | 1 | 0.0 | 0 | 8.5 | 1 | 8.5 | 1 | 0.0 | 3 | 0.0 |
| 250k | 0 | -7.8 | 1 | -7.8 | 0 | 0.0 | 1 | 0.0 | 1 | -7.8 | 3 | -7.8 |
| 0.5M |  |  | 0 | -7.8 |  |  | 0 | 0.0 | 0 | -7.8 | 1 | -7.8 |
| 1M |  |  |  |  |  |  |  |  |  |  | 0 | -7.8 |
| Max. [2] | 230.4 kbps | < | 460.8 kbps | < | 250 kbps | < | 0.5 Mbps | < | 460.8 kbps | < | 921.6 kbps | < |

[1] Baud rate in bps (k = 1000, M = 10^6^). Empty cells: rate not reachable. The source prints settings with less than 0.5 % error in bold.
[2] Maximum rate, UBRR0 = 0, error 0.0 %.
```

```table id=usart-baud-c caption="UBRR0 settings and baud rate error, f~osc~ = 8.0000, 11.0592 and 14.7456 MHz [1]"
| Baud rate | f~osc~ = 8.0000 MHz | < | < | < | f~osc~ = 11.0592 MHz | < | < | < | f~osc~ = 14.7456 MHz | < | < | < |
| ^ | U2X0 = 0 | < | U2X0 = 1 | < | U2X0 = 0 | < | U2X0 = 1 | < | U2X0 = 0 | < | U2X0 = 1 | < |
| ^ | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) |
|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2400 | 207 | 0.2 | 416 | -0.1 | 287 | 0.0 | 575 | 0.0 | 383 | 0.0 | 767 | 0.0 |
| 4800 | 103 | 0.2 | 207 | 0.2 | 143 | 0.0 | 287 | 0.0 | 191 | 0.0 | 383 | 0.0 |
| 9600 | 51 | 0.2 | 103 | 0.2 | 71 | 0.0 | 143 | 0.0 | 95 | 0.0 | 191 | 0.0 |
| 14.4k | 34 | -0.8 | 68 | 0.6 | 47 | 0.0 | 95 | 0.0 | 63 | 0.0 | 127 | 0.0 |
| 19.2k | 25 | 0.2 | 51 | 0.2 | 35 | 0.0 | 71 | 0.0 | 47 | 0.0 | 95 | 0.0 |
| 28.8k | 16 | 2.1 | 34 | -0.8 | 23 | 0.0 | 47 | 0.0 | 31 | 0.0 | 63 | 0.0 |
| 38.4k | 12 | 0.2 | 25 | 0.2 | 17 | 0.0 | 35 | 0.0 | 23 | 0.0 | 47 | 0.0 |
| 57.6k | 8 | -3.5 | 16 | 2.1 | 11 | 0.0 | 23 | 0.0 | 15 | 0.0 | 31 | 0.0 |
| 76.8k | 6 | -7.0 | 12 | 0.2 | 8 | 0.0 | 17 | 0.0 | 11 | 0.0 | 23 | 0.0 |
| 115.2k | 3 | 8.5 | 8 | -3.5 | 5 | 0.0 | 11 | 0.0 | 7 | 0.0 | 15 | 0.0 |
| 230.4k | 1 | 8.5 | 3 | 8.5 | 2 | 0.0 | 5 | 0.0 | 3 | 0.0 | 7 | 0.0 |
| 250k | 1 | 0.0 | 3 | 0.0 | 2 | -7.8 | 5 | -7.8 | 3 | -7.8 | 6 | 5.3 |
| 0.5M | 0 | 0.0 | 1 | 0.0 |  |  | 2 | -7.8 | 1 | -7.8 | 3 | -7.8 |
| 1M |  |  | 0 | 0.0 |  |  |  |  | 0 | -7.8 | 1 | -7.8 |
| Max. [2] | 0.5 Mbps | < | 1 Mbps | < | 691.2 kbps | < | 1.3824 Mbps | < | 921.6 kbps | < | 1.8432 Mbps | < |

[1] Baud rate in bps (k = 1000, M = 10^6^). Empty cells: rate not reachable. The source prints settings with less than 0.5 % error in bold.
[2] Maximum rate, UBRR0 = 0, error 0.0 %.
```

```table id=usart-baud-d caption="UBRR0 settings and baud rate error, f~osc~ = 16.0000, 18.4320 and 20.0000 MHz [1]"
| Baud rate | f~osc~ = 16.0000 MHz | < | < | < | f~osc~ = 18.4320 MHz | < | < | < | f~osc~ = 20.0000 MHz | < | < | < |
| ^ | U2X0 = 0 | < | U2X0 = 1 | < | U2X0 = 0 | < | U2X0 = 1 | < | U2X0 = 0 | < | U2X0 = 1 | < |
| ^ | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) | UBRR0 | Error (%) |
|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|
| 2400 | 416 | -0.1 | 832 | 0.0 | 479 | 0.0 | 959 | 0.0 | 520 | 0.0 | 1041 | 0.0 |
| 4800 | 207 | 0.2 | 416 | -0.1 | 239 | 0.0 | 479 | 0.0 | 259 | 0.2 | 520 | 0.0 |
| 9600 | 103 | 0.2 | 207 | 0.2 | 119 | 0.0 | 239 | 0.0 | 129 | 0.2 | 259 | 0.2 |
| 14.4k | 68 | 0.6 | 138 | -0.1 | 79 | 0.0 | 159 | 0.0 | 86 | -0.2 | 173 | -0.2 |
| 19.2k | 51 | 0.2 | 103 | 0.2 | 59 | 0.0 | 119 | 0.0 | 64 | 0.2 | 129 | 0.2 |
| 28.8k | 34 | -0.8 | 68 | 0.6 | 39 | 0.0 | 79 | 0.0 | 42 | 0.9 | 86 | -0.2 |
| 38.4k | 25 | 0.2 | 51 | 0.2 | 29 | 0.0 | 59 | 0.0 | 32 | -1.4 | 64 | 0.2 |
| 57.6k | 16 | 2.1 | 34 | -0.8 | 19 | 0.0 | 39 | 0.0 | 21 | -1.4 | 42 | 0.9 |
| 76.8k | 12 | 0.2 | 25 | 0.2 | 14 | 0.0 | 29 | 0.0 | 15 | 1.7 | 32 | -1.4 |
| 115.2k | 8 | -3.5 | 16 | 2.1 | 9 | 0.0 | 19 | 0.0 | 10 | -1.4 | 21 | -1.4 |
| 230.4k | 3 | 8.5 | 8 | -3.5 | 4 | 0.0 | 9 | 0.0 | 4 | 8.5 | 10 | -1.4 |
| 250k | 3 | 0.0 | 7 | 0.0 | 4 | -7.8 | 8 | 2.4 | 4 | 0.0 | 9 | 0.0 |
| 0.5M | 1 | 0.0 | 3 | 0.0 |  |  | 4 | -7.8 |  |  | 4 | 0.0 |
| 1M | 0 | 0.0 | 1 | 0.0 |  |  |  |  |  |  |  |  |
| Max. [2] | 1 Mbps | < | 2 Mbps | < | 1.152 Mbps | < | 2.304 Mbps | < | 1.25 Mbps | < | 2.5 Mbps | < |

[1] Baud rate in bps (k = 1000, M = 10^6^). Empty cells: rate not reachable. The source prints settings with less than 0.5 % error in bold.
[2] Maximum rate, UBRR0 = 0, error 0.0 %.
```
#### UDR0 – USART I/O Data Register 0, (0xC6)

Read/write, reset 0. Writes go to the transmit data buffer (TXB),
reads return the receive data buffer (RXB). With 5–7 bit characters the
unused high bits are ignored by the transmitter and read 0. Write it
only while UDRE0 = 1; other writes are ignored. The receive buffer is a
two-level FIFO that changes state on every access: don't use
read-modify-write instructions (SBI, CBI) on UDR0, and beware that SBIC
and SBIS also change the FIFO state.

#### UCSR0A – USART Control and Status Register 0 A, (0xC0)

```table id=usart-ucsr0a caption="UCSR0A bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RXC0 | R | 0 | Receive complete: 1 while the receive buffer holds unread data; cleared when it is empty or the receiver is disabled. Can raise the RX complete interrupt |
| 6 | TXC0 | R/W | 0 | Transmit complete: set when the whole frame has been shifted out and UDR0 holds no new data. Cleared by executing the TX complete interrupt or by writing 1 |
| 5 | UDRE0 | R | 1 | Data register empty: 1 = UDR0 can be written. Set after reset. Can raise the data register empty interrupt. Write 0 when writing UCSR0A |
| 4 | FE0 | R | 0 | Frame error: the first stop bit of the next character in the receive buffer was 0. Valid until UDR0 is read. Write 0 |
| 3 | DOR0 | R | 0 | Data overrun (see Receiver). Valid until UDR0 is read. Write 0 |
| 2 | UPE0 | R | 0 | Parity error in the next character in the receive buffer, with parity checking enabled (UPM01 = 1). Valid until UDR0 is read. Write 0 |
| 1 | U2X0 | R/W | 0 | Double speed: divisor 8 instead of 16, asynchronous mode only. Write 0 in synchronous mode |
| 0 | MPCM0 | R/W | 0 | Multi-processor communication mode: 1 = frames without address information are ignored. The transmitter is unaffected |
```

#### UCSR0B – USART Control and Status Register 0 B, (0xC1)

```table id=usart-ucsr0b caption="UCSR0B bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RXCIE0 | R/W | 0 | RX complete interrupt enable (interrupt when RXC0 = 1 and the I bit in SREG is set) |
| 6 | TXCIE0 | R/W | 0 | TX complete interrupt enable (on TXC0) |
| 5 | UDRIE0 | R/W | 0 | Data register empty interrupt enable (on UDRE0) |
| 4 | RXEN0 | R/W | 0 | Receiver enable; overrides RxD0. Disabling flushes the receive buffer, invalidating FE0, DOR0, UPE0 |
| 3 | TXEN0 | R/W | 0 | Transmitter enable; overrides TxD0. Disabling takes effect after ongoing and pending transmissions complete |
| 2 | UCSZ02 | R/W | 0 | Character size bit 2, with UCSZ0[1:0] in UCSR0C |
| 1 | RXB80 | R | 0 | Ninth bit of the received character (9-bit frames); read before UDR0 |
| 0 | TXB80 | R/W | 0 | Ninth bit of the character to transmit (9-bit frames); write before UDR0 |
```

#### UCSR0C – USART Control and Status Register 0 C, (0xC2)

```table id=usart-ucsr0c caption="UCSR0C bits (USART modes; see USART in SPI mode for MSPIM)"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | UMSEL0[1:0] | R/W | 0 | Mode: 00 = asynchronous USART, 01 = synchronous USART, 10 = reserved, 11 = master SPI (MSPIM) |
| 5:4 | UPM0[1:0] | R/W | 0 | Parity: 00 = disabled, 01 = reserved, 10 = even, 11 = odd. The receiver sets UPE0 on a mismatch |
| 3 | USBS0 | R/W | 0 | Stop bits sent by the transmitter: 0 = 1 bit, 1 = 2 bits. Ignored by the receiver |
| 2:1 | UCSZ0[1:0] | R/W | 11 | Character size, with UCSZ02 (table below) |
| 0 | UCPOL0 | R/W | 0 | Clock polarity, synchronous mode only (write 0 in asynchronous mode): 0 = TxD0 changes on the rising XCK0 edge, RxD0 sampled on the falling edge; 1 = TxD0 changes on the falling edge, RxD0 sampled on the rising edge |
```

```table id=usart-ucsz caption="Character size"
| UCSZ02 | UCSZ01 | UCSZ00 | Character size |
|:-:|:-:|:-:|---|
| 0 | 0 | 0 | 5 bits |
| 0 | 0 | 1 | 6 bits |
| 0 | 1 | 0 | 7 bits |
| 0 | 1 | 1 | 8 bits |
| 1 | 0 | 0 | Reserved |
| 1 | 0 | 1 | Reserved |
| 1 | 1 | 0 | Reserved |
| 1 | 1 | 1 | 9 bits |
```

#### UBRR0H, UBRR0L – USART Baud Rate Registers, (0xC5), (0xC4)

UBRR0[11:0] is the 12-bit baud rate value: UBRR0H bits 3:0 hold bits
11:8, UBRR0L bits 7:0; UBRR0H bits 7:4 are reserved (read 0, write 0).
R/W, reset 0. Changing the baud rate corrupts ongoing transmissions and
receptions; writing UBRR0L updates the baud rate prescaler at once.

### USART in SPI mode

With UMSEL0[1:0] = 11 the USART becomes an SPI master (MSPIM): full
duplex, three-wire, all four SPI modes, LSB or MSB first, double-buffered
(queued) operation, the 12-bit baud rate generator, f~XCK~ up to
f~CK~/2, and the USART's interrupts. The SPI control logic takes over the
transmit and receive shift registers and buffers and the baud rate
generator; the parity generator and checker, the data and clock recovery
and the RX/TX control logic are disabled and replaced by a common SPI
transfer control. Pin control and interrupt generation are as in USART
mode; the registers are at the same locations, some with other meanings.

#### MSPIM clock generation

Only the internal clock (master) is supported: set the XCK0 DDR bit to 1
(output), preferably before enabling TXEN0 and RXEN0. Clock generation
equals synchronous master mode:

```equation
BAUD = f~osc~ / (2 × (UBRR0 + 1))
UBRR0 = f~osc~ / (2 × BAUD) – 1
```

#### Data modes and frame format

UCPOL0 and UCPHA0 select the four SPI modes exactly like CPOL and CPHA
of the SPI (see [the SPI modes table](#spi-modes) and the SPI transfer
diagrams, which apply with XCK0 for SCK, TxD0 for MOSI and RxD0 for
MISO). Data is shifted out and latched in on opposite XCK0 edges.

```table id=mspim-modes caption="UCPOL0 and UCPHA0"
| UCPOL0 | UCPHA0 | SPI mode | Leading edge | Trailing edge |
|:-:|:-:|--:|---|---|
| 0 | 0 | 0 | Sample (rising) | Setup (falling) |
| 0 | 1 | 1 | Setup (rising) | Sample (falling) |
| 1 | 0 | 2 | Sample (falling) | Setup (rising) |
| 1 | 1 | 3 | Setup (falling) | Sample (rising) |
```

A frame is 8 data bits, MSB first (UDORD0 = 0) or LSB first
(UDORD0 = 1), for both receiver and transmitter. Frames may follow each
other directly, or the line idles high. Changing UDORD0, UCPHA0 or
UCPOL0 corrupts ongoing communication in both directions. A 16-bit
transfer is two bytes written to UDR0; the transmit complete interrupt
then signals that both have been shifted out.

#### MSPIM initialization

1. Set UBRR0 = 0 (already 0 right after reset).
2. Set the XCK0 DDR bit to 1 (master).
3. Write UCSR0C: UMSEL0 = 11 plus UDORD0, UCPHA0, UCPOL0 (one write).
4. Enable the transmitter (and the receiver if needed) in UCSR0B.
5. Write the wanted UBRR0 value, after enabling the transmitter but
   before the first transfer; UBRR0 = 0 at enable makes XCK0 start at
   once.

Only the transmitter can work alone. For interrupt-driven operation,
clear the global interrupt flag during initialization. Before
re-initializing (baud rate, data mode, frame format), make sure no
transfer is in progress (TXC0, RXC0 as in USART mode).

#### Data transfer

TXEN0 = 1 is required: TxD0 becomes the serial output. RXEN0 = 1 is
optional: RxD0 becomes the serial input. XCK0 is the transfer clock in
both cases. Writing UDR0 starts a transfer, for sending and for
receiving, since the transmitter drives the clock; the byte moves from
the transmit buffer to the shift register when that is ready. Read UDR0
once per byte transmitted to keep the receive buffer in step: on
overflow the last received byte is lost, not the first (with four bytes
sent and none read, byte 3 is lost).

RXC0, TXC0, UDRE0 and their interrupts work as in USART mode; FE0, DOR0
and UPE0 are unused and read 0. Disabling the transmitter or receiver
works as in USART mode.

```table id=mspim-vs-spi caption="USART in MSPIM mode compared with the SPI"
| Feature | USART MSPIM | SPI |
|---|---|---|
| Master timing, CPOL, CPHA, DORD | Identical to the SPI (UCPOL0, UCPHA0, UDORD0) | CPOL, CPHA, DORD |
| Operation | Master only | Master or slave |
| Transmit buffering | Double-buffered | None |
| Receive buffering | One more buffer level | Double-buffered |
| Write collision flag | None | WCOL |
| Double speed | Set by UBRR0 | SPI2X |
| Interrupt timing | Not compatible with the SPI | – |
| Master out / master in / clock pins | TxD0 / RxD0 / XCK0 | MOSI / MISO / SCK |
| Slave select | Not supported | !{SS} |
```

#### UDR0, UBRR0 in MSPIM mode

UDR0 and UBRR0H/L work as in USART mode.

#### UCSR0A in MSPIM mode, (0xC0)

```table id=mspim-ucsr0a caption="UCSR0A bits, MSPIM mode [1]"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | RXC0 | R | 0 | Receive complete, as in USART mode |
| 6 | TXC0 | R/W | 0 | Transmit complete, as in USART mode |
| 5 | UDRE0 | R | 0 | Data register empty, as in USART mode (set after reset) |

[1] Reset values as printed in the source (bits 2:1 = 1, UDRE0 = 0); they contradict the USART-mode register and the text ("UDRE0 is set after a reset").
```

#### UCSR0B in MSPIM mode, (0xC1)

```table id=mspim-ucsr0b caption="UCSR0B bits, MSPIM mode [1]"
| Bit | Name | Description |
|---|---|---|
| 7 | RXCIE0 | RX complete interrupt enable |
| 6 | TXCIE0 | TX complete interrupt enable |
| 5 | UDRIE0 | Data register empty interrupt enable |
| 4 | RXEN0 | Receiver enable; overrides RxD0; disabling flushes the receive buffer. RXEN0 = 1 with TXEN0 = 0 is meaningless (the transmitter drives the clock) |
| 3 | TXEN0 | Transmitter enable; overrides TxD0; disabling waits for pending transfers |

[1] Reset values of bits 2:0 as printed in the source.
```

#### UCSR0C in MSPIM mode, (0xC2)

```table id=mspim-ucsr0c caption="UCSR0C bits, MSPIM mode"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | UMSEL0[1:0] | R/W | 0 | 11 = master SPI (other values as in USART mode). UDORD0, UCPHA0 and UCPOL0 may be set in the same write |
| 2 | UDORD0 | R/W | 1 | Data order: 1 = LSB first, 0 = MSB first |
| 1 | UCPHA0 | R/W | 1 | Clock phase: 0 = sample on the leading XCK0 edge, 1 = sample on the trailing edge |
| 0 | UCPOL0 | R/W | 0 | Clock polarity of XCK0; with UCPHA0 sets the SPI mode |
```

### 2-wire serial interface (TWI)

A byte-oriented, interrupt-based I²C-compatible interface on SCL (PC5) and
SDA (PC4): master and slave, transmitter and receiver, 7-bit addressing
(up to 128 slaves), multi-master arbitration, up to 400 kHz, slew-rate
limited outputs, spike suppression on the inputs, a programmable slave
address with general call, and address recognition that wakes the part
from sleep. PRTWI in PRR must be 0 to enable it.

```table id=twi-terms caption="TWI terminology"
| Term | Description |
|---|---|
| Master | The device that initiates and terminates a transmission; it also generates the SCL clock |
| Slave | The device addressed by a master |
| Transmitter | The device placing data on the bus |
| Receiver | The device reading data from the bus |
```

Both bus lines are pulled up to V~CC~ by one resistor each; all bus
drivers are open-drain or open-collector, giving a wired-AND: a line is
low when one or more devices output 0, high when all release it. All AVR
devices on the bus must be powered for any bus operation. The number of
devices is limited only by the 400 pF bus capacitance limit and the 7-bit
address space.

#### Data transfer and frame format

Each data bit comes with a pulse on SCL; SDA must be stable while SCL is
high, except for START and STOP. The master starts a transmission with a
START (SDA falls while SCL is high) and ends it with a STOP (SDA rises
while SCL is high); in between the bus is busy. A START issued before the
STOP is a REPEATED START, which starts a new transfer without releasing
the bus; "START" below means both unless noted.

```wavedrom id=twi-start-stop caption="START, STOP, START, REPEATED START, STOP"
{"signal": [
  {"name": "SDA", "wave": "10..1.0.1.0..1."},
  {"name": "SCL", "wave": "1.01...0.1.01.."}
 ],
 "config": {"hscale": 2}}
```

Left to right: START (SDA falls, SCL high), STOP (SDA rises, SCL high),
START, REPEATED START (a START while the bus is busy), STOP.

**Address packet.** 9 bits: 7 address bits (MSB first), a READ/WRITE bit
(1 = read, 0 = write) and an acknowledge bit. An addressed slave pulls SDA
low in the ninth SCL cycle (ACK); a busy slave, or one that can't serve the
request, leaves SDA high, and the master can then send a STOP or a
REPEATED START. The packets are called SLA+R and SLA+W. Address 0000 000
is the general call: with a write bit, all slaves set up to acknowledge it
pull SDA low and receive the following data packets; a general call with a
read bit is meaningless (contention). Addresses 1111 xxx are reserved.

**Data packet.** 9 bits: one data byte (MSB first) and an acknowledge bit.
The master generates the clock, START and STOP; the receiver acknowledges
by pulling SDA low in the ninth SCL cycle, or signals NACK by leaving it
high. After the last byte, or when it can take no more, the receiver sends
NACK.

**Transmission.** A START, SLA+R/W, one or more data packets, a STOP. An
empty message (START then STOP) is illegal. A slave may stretch the SCL
low period by holding SCL low (handshaking); the SCL high period is set by
the master.

```wavedrom id=twi-transmission caption="Typical data transmission"
{"signal": [
  {"name": "SDA", "wave": "10=.=.=.=.=.=.=.=.=.0..1", "data": ["A6", "A5", "A0", "R/W", "ACK", "D7", "D6", "D0", "ACK"]},
  {"name": "SCL", "wave": "1.0101010101010101010.1."}
 ],
 "config": {"hscale": 2}}
```

#### Multi-master arbitration and synchronization

Several masters may start at once. Arbitration lets only one finish; a
losing master switches at once to slave mode to check whether the winner
addresses it, and the slaves never see the contention (data is not
corrupted). The wired-AND also synchronizes the masters' clocks: the
combined SCL high period equals the shortest master high period, the low
period the longest master low period; every master starts counting its
high and low time-outs when the combined SCL goes high or low.

Every master monitors SDA after outputting data; when the value read
differs from the value output (it output 1 while another output 0), it
has lost. The loser leaves SDA high but may keep generating SCL until the
end of the current address or data packet. Arbitration may take many bits
and continues into the data packet when several masters address the same
slave. Arbitration is not allowed between a REPEATED START and a data
bit, a STOP and a data bit, or a REPEATED START and a STOP; software must
prevent these, so in multi-master systems every transfer must use the same
composition of SLA+R/W and data packets (the same number of data packets),
or the result of arbitration is undefined.

#### Module overview

Block diagram: the SCL and SDA pins (slew-rate control and spike filter
each), the bus interface unit (START/STOP control, spike suppression,
arbitration detection, TWDR, ACK), the bit rate generator (prescaler,
TWBR), the address match unit (TWAR, address comparator) and the control
unit (TWSR, TWCR, state machine).

**SCL and SDA pins.** Output drivers have a slew-rate limiter; inputs
suppress spikes shorter than 50 ns. The port's internal pull-ups can be
enabled by setting the PORT bits of SCL and SDA, which may make external
pull-ups unnecessary.

**Bit rate generator.** Sets the SCL period in master modes from TWBR and
the prescaler bits TWPS in TWSR. Slave operation doesn't depend on these,
but the slave's CPU clock must be at least 16 times the SCL frequency.
Slaves may stretch the SCL low period, lowering the average bus clock.
Choose pull-up values from the SCL frequency and bus capacitance (see the
TWI characteristics below).

```equation
f~SCL~ = f~CPU~ / (16 + 2 × TWBR × prescaler value)
```

**Bus interface unit.** TWDR holds the address or data byte to transmit
or the one received; a separate (N)ACK register, not directly accessible,
holds the acknowledge bit: when receiving it is set through TWCR (TWEA),
when transmitting the received value shows in TWSR. The START/STOP
controller generates and detects START, REPEATED START and STOP, also in
sleep modes, so the MCU can wake when addressed. The arbitration detection
hardware monitors a transmission the TWI started as master and tells the
control unit when arbitration is lost.

**Address match unit.** Compares received address bytes with the 7-bit
address in TWAR, and with the general call address when TWGCE = 1; on a
match it informs the control unit. Whether the TWI acknowledges depends on
TWCR. Works in sleep modes too.

**Control unit.** When a bus event needs the application, TWINT is set,
and in the next clock cycle TWSR gets a status code for it. TWSR is valid
only while TWINT is set; otherwise it reads 0xF8 (no relevant
information). While TWINT is set, SCL is held low. TWINT is set after the
TWI has: transmitted a START/REPEATED START; transmitted SLA+R/W;
transmitted an address byte; lost arbitration; been addressed by its own
slave address or the general call; received a data byte; received a STOP
or REPEATED START while still addressed as a slave; or detected a bus
error (illegal START or STOP).

#### Using the TWI

Interrupts follow every bus event. With TWIE = 1 and the I bit in SREG
set, TWINT requests an interrupt; with TWIE = 0 the application polls
TWINT. When TWINT is set, TWSR holds the bus state, and the application
sets up the next bus cycle through TWCR and TWDR. A master sending one
byte to a slave:

1. Write TWCR to send a START (TWINT = 1 in the value written: writing 1
   clears the flag, and the TWI does nothing while TWINT is set). The TWI
   sends the START as soon as TWINT is cleared.
2. When the START is sent, TWINT is set and TWSR shows "START sent".
3. Check TWSR (take error action if unexpected). Load SLA+W into TWDR
   (TWDR is used for both address and data), then write TWCR with
   TWINT = 1 and TWSTA = 0 to send the address packet.
4. When it is sent, TWINT is set; TWSR shows whether a slave acknowledged.
5. Check TWSR. Load the data byte into TWDR and write TWCR with TWINT = 1
   to send it.
6. When it is sent, TWINT is set; TWSR shows whether it was acknowledged.
7. Check TWSR. Write TWCR with TWINT = 1 and TWSTO = 1 to send a STOP.
   TWINT is not set after a STOP has been sent.

In every transfer: when TWINT is set, SCL is held low until it is cleared;
the application then updates all TWI registers for the next bus cycle
(TWDR with the next value to send), and finally writes TWCR with TWINT = 1,
which starts whatever operation the TWCR value specifies.

#### Transmission modes

Four modes: master transmitter (MT), master receiver (MR), slave
transmitter (ST) and slave receiver (SR); one application may use
several. Status codes below have the prescaler bits masked to zero. In
the "Next" column: S = START, Rs = REPEATED START, P = STOP, SLA = slave
address, R/W = read (SDA high) / write (SDA low) bit, ACK = SDA low,
NACK = SDA high, data = 8-bit byte. The TWCR columns give the bits written
with the response (X = don't care); TWEN = 1 throughout.

```table id=twi-twcr-values caption="TWCR values written to drive a transfer"
| Action | TWINT | TWEA | TWSTA | TWSTO | TWWC | TWEN | Bit 1 | TWIE |
|---|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|
| Send START or REPEATED START | 1 | X | 1 | 0 | X | 1 | 0 | X |
| Send SLA+R/W or a data byte from TWDR; continue | 1 | X | 0 | 0 | X | 1 | 0 | X |
| Send STOP | 1 | X | 0 | 1 | X | 1 | 0 | X |
| Initialize slave (SR, ST) | 0 | 1 | 0 | 0 | 0 | 1 | 0 | X |
```

##### Master transmitter mode

Send a START (status 0x08). Write SLA+W to TWDR and clear TWINT to enter
MT mode (SLA+R enters MR mode); then status 0x18, 0x20 or 0x38. Next
write each data byte to TWDR and clear TWINT. TWDR may only be written
while TWINT is set; otherwise the write is discarded and TWWC is set.
End with a STOP or a REPEATED START. After a REPEATED START (0x10) the
master can address the same or a new slave, or switch between MT and MR,
without releasing the bus.

```table id=twi-mt caption="Status codes, master transmitter mode"
| Status | Bus and TWI state | TWDR | STA | STO | TWINT | TWEA | Next action by the TWI |
|---|---|---|:-:|:-:|:-:|:-:|---|
| 0x08 | START sent | Load SLA+W | 0 | 0 | 1 | X | SLA+W sent; ACK or NACK received |
| 0x10 | REPEATED START sent | Load SLA+W | 0 | 0 | 1 | X | SLA+W sent; ACK or NACK received |
| ^ | ^ | Load SLA+R | 0 | 0 | 1 | X | SLA+R sent; switch to master receiver mode |
| 0x18 | SLA+W sent; ACK received | Load data byte | 0 | 0 | 1 | X | Data byte sent; ACK or NACK received |
| ^ | ^ | No action | 1 | 0 | 1 | X | REPEATED START sent |
| ^ | ^ | No action | 0 | 1 | 1 | X | STOP sent; TWSTO cleared |
| ^ | ^ | No action | 1 | 1 | 1 | X | STOP then START sent; TWSTO cleared |
| 0x20 | SLA+W sent; NACK received | Load data byte | 0 | 0 | 1 | X | Data byte sent; ACK or NACK received |
| ^ | ^ | No action | 1 | 0 | 1 | X | REPEATED START sent |
| ^ | ^ | No action | 0 | 1 | 1 | X | STOP sent; TWSTO cleared |
| ^ | ^ | No action | 1 | 1 | 1 | X | STOP then START sent; TWSTO cleared |
| 0x28 | Data byte sent; ACK received | Load data byte | 0 | 0 | 1 | X | Data byte sent; ACK or NACK received |
| ^ | ^ | No action | 1 | 0 | 1 | X | REPEATED START sent |
| ^ | ^ | No action | 0 | 1 | 1 | X | STOP sent; TWSTO cleared |
| ^ | ^ | No action | 1 | 1 | 1 | X | STOP then START sent; TWSTO cleared |
| 0x30 | Data byte sent; NACK received | Load data byte | 0 | 0 | 1 | X | Data byte sent; ACK or NACK received |
| ^ | ^ | No action | 1 | 0 | 1 | X | REPEATED START sent |
| ^ | ^ | No action | 0 | 1 | 1 | X | STOP sent; TWSTO cleared |
| ^ | ^ | No action | 1 | 1 | 1 | X | STOP then START sent; TWSTO cleared |
| 0x38 | Arbitration lost in SLA+W or data bytes | No action | 0 | 0 | 1 | X | Bus released; not addressed slave mode entered |
| ^ | ^ | No action | 1 | 0 | 1 | X | START sent when the bus becomes free |
```

Sequences: S, SLA+W (0x08), ACK (0x18), data, ACK (0x28), P — or Rs (0x10)
for the next transfer, then SLA+W, or SLA+R into MR mode. NACK after the
address gives 0x20, NACK after a data byte 0x30, arbitration lost in the
address or a data byte 0x38; arbitration lost and addressed as slave gives
0x68, 0x78 or 0xB0 (slave mode states).

##### Master receiver mode

Send a START (0x08), write SLA+R to TWDR and clear TWINT; then status
0x38, 0x40 or 0x48. Received data is read from TWDR while TWINT is set.
After the last byte the master receiver sends NACK, then ends with a STOP
or a REPEATED START (0x10), as in MT mode.

```table id=twi-mr caption="Status codes, master receiver mode"
| Status | Bus and TWI state | TWDR | STA | STO | TWINT | TWEA | Next action by the TWI |
|---|---|---|:-:|:-:|:-:|:-:|---|
| 0x08 | START sent | Load SLA+R | 0 | 0 | 1 | X | SLA+R sent; ACK or NACK received |
| 0x10 | REPEATED START sent | Load SLA+R | 0 | 0 | 1 | X | SLA+R sent; ACK or NACK received |
| ^ | ^ | Load SLA+W | 0 | 0 | 1 | X | SLA+W sent; switch to master transmitter mode |
| 0x38 | Arbitration lost in SLA+R or NACK bit | No action | 0 | 0 | 1 | X | Bus released; not addressed slave mode entered |
| ^ | ^ | No action | 1 | 0 | 1 | X | START sent when the bus becomes free |
| 0x40 | SLA+R sent; ACK received | No action | 0 | 0 | 1 | 0 | Data byte received; NACK returned |
| ^ | ^ | No action | 0 | 0 | 1 | 1 | Data byte received; ACK returned |
| 0x48 | SLA+R sent; NACK received | No action | 1 | 0 | 1 | X | REPEATED START sent |
| ^ | ^ | No action | 0 | 1 | 1 | X | STOP sent; TWSTO cleared |
| ^ | ^ | No action | 1 | 1 | 1 | X | STOP then START sent; TWSTO cleared |
| 0x50 | Data byte received; ACK returned | Read data byte | 0 | 0 | 1 | 0 | Data byte received; NACK returned |
| ^ | ^ | Read data byte | 0 | 0 | 1 | 1 | Data byte received; ACK returned |
| 0x58 | Data byte received; NACK returned | Read data byte | 1 | 0 | 1 | X | REPEATED START sent |
| ^ | ^ | Read data byte | 0 | 1 | 1 | X | STOP sent; TWSTO cleared |
| ^ | ^ | Read data byte | 1 | 1 | 1 | X | STOP then START sent; TWSTO cleared |
```

Sequences: S (0x08), SLA+R, ACK (0x40), data, ACK (0x50) …, data, NACK
(0x58), P or Rs (0x10). NACK after the address: 0x48. Arbitration lost:
0x38, or 0x68/0x78/0xB0 when addressed as slave.

##### Slave receiver mode

Initialize TWAR with the own slave address in bits 7:1 and TWGCE in bit 0
(1 = also answer the general call address 0x00), and TWCR with TWEA = 1
(acknowledge own address or general call), TWEN = 1, TWSTA = TWSTO = 0.
The TWI then waits until it is addressed; direction bit 0 (write) enters
SR mode, 1 enters ST mode. After its own address and the write bit,
TWINT is set with a valid status. SR mode is also entered when
arbitration is lost as master (0x68, 0x78).

Clearing TWEA during a transfer returns NACK after the next received byte
(the slave can take no more). While TWEA = 0 the TWI ignores its own
address but keeps monitoring the bus; setting TWEA resumes address
recognition, so TWEA can isolate the TWI from the bus temporarily.

In every sleep mode except idle the TWI clock is off. With TWEA = 1 the
TWI still acknowledges its own or the general call address, clocked by
SCL; the part wakes and the TWI holds SCL low during wake-up and until
TWINT is cleared; reception then continues normally. With a long start-up
time SCL may be held low for a long time, blocking other transfers. After
such a wake-up TWDR does not hold the last byte seen on the bus.

```table id=twi-sr caption="Status codes, slave receiver mode"
| Status | Bus and TWI state | TWDR | STA | STO | TWINT | TWEA | Next action by the TWI |
|---|---|---|:-:|:-:|:-:|:-:|---|
| 0x60 | Own SLA+W received; ACK returned | No action | X | 0 | 1 | 0 | Data byte received; NACK returned |
| ^ | ^ | No action | X | 0 | 1 | 1 | Data byte received; ACK returned |
| 0x68 | Arbitration lost in SLA+R/W as master; own SLA+W received; ACK returned | No action | X | 0 | 1 | 0 | Data byte received; NACK returned |
| ^ | ^ | No action | X | 0 | 1 | 1 | Data byte received; ACK returned |
| 0x70 | General call address received; ACK returned | No action | X | 0 | 1 | 0 | Data byte received; NACK returned |
| ^ | ^ | No action | X | 0 | 1 | 1 | Data byte received; ACK returned |
| 0x78 | Arbitration lost in SLA+R/W as master; general call address received; ACK returned | No action | X | 0 | 1 | 0 | Data byte received; NACK returned |
| ^ | ^ | No action | X | 0 | 1 | 1 | Data byte received; ACK returned |
| 0x80 | Addressed with own SLA+W; data received; ACK returned | Read data byte | X | 0 | 1 | 0 | Data byte received; NACK returned |
| ^ | ^ | Read data byte | X | 0 | 1 | 1 | Data byte received; ACK returned |
| 0x88 | Addressed with own SLA+W; data received; NACK returned | Read data byte | 0 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized |
| ^ | ^ | Read data byte | 0 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1 |
| ^ | ^ | Read data byte | 1 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized; START sent when the bus becomes free |
| ^ | ^ | Read data byte | 1 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1; START sent when the bus becomes free |
| 0x90 | Addressed with general call; data received; ACK returned | Read data byte | X | 0 | 1 | 0 | Data byte received; NACK returned |
| ^ | ^ | Read data byte | X | 0 | 1 | 1 | Data byte received; ACK returned |
| 0x98 | Addressed with general call; data received; NACK returned | Read data byte | 0 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized |
| ^ | ^ | Read data byte | 0 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1 |
| ^ | ^ | Read data byte | 1 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized; START sent when the bus becomes free |
| ^ | ^ | Read data byte | 1 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1; START sent when the bus becomes free |
| 0xA0 | STOP or REPEATED START received while still addressed as slave | No action | 0 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized |
| ^ | ^ | No action | 0 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1 |
| ^ | ^ | No action | 1 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized; START sent when the bus becomes free |
| ^ | ^ | No action | 1 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1; START sent when the bus becomes free |
```

Sequences: S, SLA+W, ACK (0x60), data, ACK (0x80) …, P or S (0xA0); last
byte not acknowledged: 0x88. General call: ACK (0x70), data, ACK (0x90) …,
P or S (0xA0); last byte NACK: 0x98. Arbitration lost as master and
addressed: 0x68 (own address), 0x78 (general call).

##### Slave transmitter mode

Initialize TWAR and TWCR as for SR mode. Own address with direction bit 1
(read) enters ST mode; also entered when arbitration is lost as master
(0xB0). Clearing TWEA during a transfer makes the TWI send the last byte;
it then enters 0xC0 (master sent NACK) or 0xC8 (master sent ACK, asking
for more), switches to not addressed slave mode and ignores the master,
which then reads all 1s. TWEA = 0 isolation and the sleep-mode wake-up
behave as in SR mode.

```table id=twi-st caption="Status codes, slave transmitter mode"
| Status | Bus and TWI state | TWDR | STA | STO | TWINT | TWEA | Next action by the TWI |
|---|---|---|:-:|:-:|:-:|:-:|---|
| 0xA8 | Own SLA+R received; ACK returned | Load data byte | X | 0 | 1 | 0 | Last data byte sent; NACK should be received |
| ^ | ^ | Load data byte | X | 0 | 1 | 1 | Data byte sent; ACK should be received |
| 0xB0 | Arbitration lost in SLA+R/W as master; own SLA+R received; ACK returned | Load data byte | X | 0 | 1 | 0 | Last data byte sent; NACK should be received |
| ^ | ^ | Load data byte | X | 0 | 1 | 1 | Data byte sent; ACK should be received |
| 0xB8 | Data byte in TWDR sent; ACK received | Load data byte | X | 0 | 1 | 0 | Last data byte sent; NACK should be received |
| ^ | ^ | Load data byte | X | 0 | 1 | 1 | Data byte sent; ACK should be received |
| 0xC0 | Data byte in TWDR sent; NACK received | No action | 0 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized |
| ^ | ^ | No action | 0 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1 |
| ^ | ^ | No action | 1 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized; START sent when the bus becomes free |
| ^ | ^ | No action | 1 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1; START sent when the bus becomes free |
| 0xC8 | Last data byte in TWDR sent (TWEA = 0); ACK received | No action | 0 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized |
| ^ | ^ | No action | 0 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1 |
| ^ | ^ | No action | 1 | 0 | 1 | 0 | Not addressed slave mode; own SLA and GCA not recognized; START sent when the bus becomes free |
| ^ | ^ | No action | 1 | 0 | 1 | 1 | Not addressed slave mode; own SLA recognized, GCA if TWGCE = 1; START sent when the bus becomes free |
```

Sequences: S, SLA+R, ACK (0xA8), data, ACK (0xB8) …, data, NACK (0xC0),
P or S. Last byte sent with TWEA = 0 and ACK received: 0xC8, then all 1s,
P or S. Arbitration lost as master and addressed: 0xB0.

##### Miscellaneous states

0xF8: no relevant information (TWINT = 0), between states and when the
TWI is not in a transfer. 0x00: bus error, a START or STOP at an illegal
position in the frame (during an address byte, data byte or acknowledge
bit); TWINT is set. To recover, set TWSTO and clear TWINT: the TWI enters
not addressed slave mode and clears TWSTO (no other TWCR bit changes),
SDA and SCL are released, and no STOP is sent.

```table id=twi-misc caption="Miscellaneous status codes"
| Status | Bus and TWI state | TWDR | STA | STO | TWINT | TWEA | Next action by the TWI |
|---|---|---|:-:|:-:|:-:|:-:|---|
| 0xF8 | No relevant state information; TWINT = 0 | No action | No TWCR action | < | < | < | Wait or proceed with the current transfer |
| 0x00 | Bus error, illegal START or STOP | No action | 0 | 1 | 1 | X | Only internal hardware affected, no STOP sent; the bus is released and TWSTO cleared |
```

##### Combining modes and multi-master systems

Reading a serial EEPROM needs MT mode (send the location to read) then MR
mode (read the data), done as one atomic operation so that another master
can't move the EEPROM's data pointer in between: S, SLA+W, ACK, address,
ACK, Rs, SLA+R, ACK, data, NACK, P. The REPEATED START changes the
direction while the master keeps the bus.

When several masters start at once, the TWI lets one proceed without
data loss:

- Masters doing identical communication with the same slave: no one
  notices the contention.
- Masters accessing the same slave with different data or direction bit:
  arbitration in the R/W bit or the data bits; a master outputting 1 while
  another outputs 0 loses, and switches to not addressed slave mode or
  waits for a free bus and sends a new START, as the software decides.
- Masters accessing different slaves: arbitration in the SLA bits; losers
  switch to slave mode, and enter SR or ST mode if addressed by the
  winner, otherwise not addressed slave mode or a new START when the bus
  is free.

Arbitration lost in SLA: own address or general call received → 0x68/0x78
(write: data received, ACK or NACK returned) or 0xB0 (read: data sent);
not addressed → 0x38. Arbitration lost in data → 0x38 (bus released, or a
START sent when the bus is free).

#### TWBR – TWI Bit Rate Register, (0xB8)

Reset 0x00, R/W. The division factor of the bit rate generator, which
makes SCL in the master modes (see the f~SCL~ equation above).

#### TWCR – TWI Control Register, (0xBC)

Enables the TWI, starts a master access with a START, generates the
receiver acknowledge, generates a STOP, halts the bus while TWDR is being
written, and flags a write collision.

```table id=twi-twcr caption="TWCR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | TWINT | R/W | 0 | TWI interrupt flag. Set by hardware when the TWI has finished its current job and expects a software response; with the I bit and TWIE set the MCU jumps to the TWI vector. While set, the SCL low period is stretched. Cleared only by writing 1 (not by running the interrupt routine); clearing it starts the TWI operation, so all accesses to TWAR, TWSR and TWDR must be done before. |
| 6 | TWEA | R/W | 0 | Enable acknowledge: 1 = generate ACK when the own slave address is received, when a general call is received while TWGCE = 1, or when a data byte is received in MR or SR mode. 0 = virtually disconnect from the bus; writing 1 resumes address recognition. |
| 5 | TWSTA | R/W | 0 | START condition: 1 = become master; a START is sent when the bus is free, otherwise after the next STOP is detected. Clear it in software after the START has been sent. |
| 4 | TWSTO | R/W | 0 | STOP condition: 1 in master mode sends a STOP; cleared automatically when it has been executed. In slave mode, 1 recovers from an error: no STOP is sent, but the TWI returns to the unaddressed slave mode and releases SCL and SDA (high impedance). |
| 3 | TWWC | R | 0 | Write collision flag: set by writing TWDR while TWINT is 0; cleared by writing TWDR while TWINT is 1 |
| 2 | TWEN | R/W | 0 | TWI enable: 1 = the TWI takes over the SCL and SDA pins and enables the slew-rate limiters and spike filters. 0 = TWI off; every transmission is terminated, whatever is in progress. |
| 0 | TWIE | R/W | 0 | Interrupt enable: with the I bit set, the TWI interrupt is requested for as long as TWINT is 1 |
```

#### TWSR – TWI Status Register, (0xB9)

```table id=twi-twsr caption="TWSR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:3 | TWS[7:3] | R | 11111 | Status of the TWI logic and the bus (status code tables above). Mask the prescaler bits to zero when checking the status, so the check doesn't depend on the prescaler. |
| 1:0 | TWPS[1:0] | R/W | 00 | Bit rate prescaler: 00 = 1, 01 = 4, 10 = 16, 11 = 64 |
```

#### TWDR – TWI Data Register, (0xBB)

Reset 0xFF, R/W. In transmit mode the next byte to send (address or
data), in receive mode the last byte received. Writable only while the
TWI is not shifting a byte, that is while TWINT is set; it can't be
initialized before the first interrupt. Stable while TWINT is set. Data
on the bus is shifted in while data is shifted out, so TWDR always holds
the last byte on the bus, except after a wake-up from sleep by the TWI
interrupt (then undefined). No data is lost when lost arbitration
switches the TWI from master to slave. The ACK bit is handled by the TWI
logic; the CPU can't access it.

#### TWAR – TWI (Slave) Address Register, (0xBA)

```table id=twi-twar caption="TWAR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:1 | TWA[6:0] | R/W | 1111111 | Own 7-bit slave address, used in slave transmitter and receiver modes; not needed in master modes. In multi-master systems, set it in masters that other masters can address as slaves. A received address that matches (or the general call, if enabled) requests an interrupt. |
| 0 | TWGCE | R/W | 0 | General call recognition enable: 1 = also recognize the general call address 0x00 |
```

#### TWAMR – TWI (Slave) Address Mask Register, (0xBD)

```table id=twi-twamr caption="TWAMR bits"
| Bit | Name | Description |
|---|---|---|
| 7:1 | TWAM[6:0] | Address mask: a 1 makes the address match logic ignore the compare between that incoming address bit and the corresponding TWAR bit |
```

#### TWI characteristics

The requirements for devices on the bus; the TWI meets or exceeds them
under the conditions given.

```table id=twi-chars caption="Two-wire serial bus requirements"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~IL~ | Input low voltage | | -0.5 | 0.3 × V~CC~ | V |
| V~IH~ | Input high voltage | | 0.7 × V~CC~ | V~CC~ + 0.5 | V |
| V~hys~ | Hysteresis of Schmitt-trigger inputs [1] | | 0.05 × V~CC~ [2] | | V |
| V~OL~ | Output low voltage [1] | 3 mA sink current | 0 | 0.4 | V |
| t~r~ | Rise time for both SDA and SCL [1] | | 20 + 0.1 × C~b~ [3][2] | 300 | ns |
| t~of~ | Output fall time from V~IHmin~ to V~ILmax~ [1] | 10 pF < C~b~ < 400 pF [3] | 20 + 0.1 × C~b~ [3][2] | 250 | ns |
| t~SP~ | Spikes suppressed by the input filter [1] | | 0 | 50 [2] | ns |
| I~i~ | Input current, each I/O pin | 0.1 × V~CC~ < V~i~ < 0.9 × V~CC~ | -10 | 10 | µA |
| C~i~ | Capacitance, each I/O pin [1] | | | 10 | pF |
| f~SCL~ | SCL clock frequency | f~CK~ [4] > max(16 × f~SCL~, 250 kHz) [5] | 0 | 400 | kHz |
| R~p~ | Pull-up resistor value | f~SCL~ ≤ 100 kHz | (V~CC~ - 0.4 V) / 3 mA | 1000 ns / C~b~ | Ω |
| ^ | ^ | f~SCL~ > 100 kHz | (V~CC~ - 0.4 V) / 3 mA | 300 ns / C~b~ | Ω |
| t~HD;STA~ | Hold time, (repeated) START condition | f~SCL~ ≤ 100 kHz | 4.0 | | µs |
| ^ | ^ | f~SCL~ > 100 kHz | 0.6 | | µs |
| t~LOW~ | Low period of the SCL clock | f~SCL~ ≤ 100 kHz | 4.7 | | µs |
| ^ | ^ | f~SCL~ > 100 kHz | 1.3 | | µs |
| t~HIGH~ | High period of the SCL clock | f~SCL~ ≤ 100 kHz | 4.0 | | µs |
| ^ | ^ | f~SCL~ > 100 kHz | 0.6 | | µs |
| t~SU;STA~ | Setup time, repeated START condition | f~SCL~ ≤ 100 kHz | 4.7 | | µs |
| ^ | ^ | f~SCL~ > 100 kHz | 0.6 | | µs |
| t~HD;DAT~ | Data hold time | f~SCL~ ≤ 100 kHz | 0 | 3.45 | µs |
| ^ | ^ | f~SCL~ > 100 kHz | 0 | 0.9 | µs |
| t~SU;DAT~ | Data setup time | f~SCL~ ≤ 100 kHz | 250 | | ns |
| ^ | ^ | f~SCL~ > 100 kHz | 100 | | ns |
| t~SU;STO~ | Setup time, STOP condition | f~SCL~ ≤ 100 kHz | 4.0 | | µs |
| ^ | ^ | f~SCL~ > 100 kHz | 0.6 | | µs |
| t~BUF~ | Bus free time between a STOP and START condition | f~SCL~ ≤ 100 kHz | 4.7 | | µs |
| ^ | ^ | f~SCL~ > 100 kHz | 1.3 | | µs |

[1] Characterized, not 100 % tested.
[2] Required only for f~SCL~ > 100 kHz.
[3] C~b~ = capacitance of one bus line in pF.
[4] f~CK~ = CPU clock frequency.
[5] Applies to all operation of this part's TWI; other devices on the bus need only obey the general f~SCL~ requirement.
```

```wavedrom id=twi-timing caption="Two-wire serial bus timing"
{"signal": [
  {"name": "SCL", "wave": "1..0.1.0..1.0.1....", "node": "...A.B.C..D...E"},
  {"name": "SDA", "wave": "1.0.....=....0..1.0", "data": ["data"], "node": "..F.....G.......H.I"}
 ],
 "edge": ["F~>A tHD;STA", "A<->B tLOW", "B<->C tHIGH", "C~>G tHD;DAT", "G~>D tSU;DAT", "E~>H tSU;STO", "H<->I tBUF"],
 "config": {"hscale": 2}}
```

### Analog comparator

Compares the positive input AIN0 with the negative input AIN1; when AIN0
is higher, the output ACO is 1. The output can trigger the Timer/Counter1
input capture, and its own interrupt on output rise, fall or toggle. PRADC
in PRR must be 0 to use the ADC input multiplexer. Block diagram: AIN0 or
the bandgap reference (ACBG) on the + input, AIN1 or the ADC multiplexer
output (ACME, ADEN) on the − input, then the interrupt select logic (ACIS,
ACIE, ACI) and the Timer/Counter1 input capture connection (ACIC).

Any of ADC7…ADC0 can replace the negative input, through the ADC
multiplexer; the ADC must then be off.

```table id=ac-mux caption="Analog comparator negative input"
| ACME | ADEN | MUX[2:0] | Negative input |
|:-:|:-:|:-:|---|
| 0 | X | XXX | AIN1 |
| 1 | 1 | XXX | AIN1 |
| 1 | 0 | 000 | ADC0 |
| 1 | 0 | 001 | ADC1 |
| 1 | 0 | 010 | ADC2 |
| 1 | 0 | 011 | ADC3 |
| 1 | 0 | 100 | ADC4 |
| 1 | 0 | 101 | ADC5 |
| 1 | 0 | 110 | ADC6 |
| 1 | 0 | 111 | ADC7 |
```

#### ADCSRB – ADC Control and Status Register B (ACME), (0x7B)

Bit 6, ACME (R/W, reset 0), analog comparator multiplexer enable: 1 with
the ADC off (ADEN = 0) lets the ADC multiplexer select the comparator's
negative input; 0 applies AIN1. Bits 2:0 (ADTS) are described under
[ADC](#adc).

#### ACSR – Analog Comparator Control and Status Register, 0x30 (0x50)

```table id=ac-acsr caption="ACSR bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | ACD | R/W | 0 | Comparator disable: 1 = power to the comparator off (saves power in active and idle mode); may be set at any time. Clear ACIE before changing ACD, or an interrupt can occur. |
| 6 | ACBG | R/W | 0 | Bandgap select: 1 = the fixed bandgap reference replaces AIN0 on the + input; it needs time to stabilize, and an unstabilized first conversion may be wrong. 0 = AIN0. |
| 5 | ACO | R | N/A | Comparator output, synchronized (1–2 clock cycles of delay) |
| 4 | ACI | R/W | 0 | Interrupt flag: set when an output event matches ACIS; the interrupt runs if ACIE and the I bit are set. Cleared by hardware when the vector executes, or by writing 1. |
| 3 | ACIE | R/W | 0 | Interrupt enable (with the I bit set) |
| 2 | ACIC | R/W | 0 | Input capture enable: 1 = the comparator output drives the Timer/Counter1 input capture front end directly (using its noise canceler and edge select); set ICIE1 in TIMSK1 to get the input capture interrupt. 0 = no connection. |
| 1:0 | ACIS[1:0] | R/W | 00 | Interrupt mode: 00 = output toggle, 01 = reserved, 10 = falling output edge, 11 = rising output edge. Clear ACIE before changing them, or an interrupt can occur. |
```

#### DIDR1 – Digital Input Disable Register 1, (0x7F)

```table id=ac-didr1 caption="DIDR1 bits"
| Bit | Name | Description |
|---|---|---|
| 1:0 | AIN1D, AIN0D | 1 = digital input buffer on AIN1/AIN0 off (its PIN bit reads 0). Set when the pin carries an analog signal and its digital input isn't needed, to save power. |
```

### ADC

A 10-bit successive approximation ADC with a sample-and-hold, fed by an
8-channel single-ended multiplexer (ADC0–ADC5 on all packages, ADC6 and
ADC7 on TQFP and VQFN only), plus the temperature sensor, the 1.1 V
bandgap and GND. Inputs refer to GND; input range 0 V to V~CC~. 0.5 LSB
integral non-linearity, ±2 LSB absolute accuracy, 13–260 µs conversion
time, up to 76.9 kSPS (15 kSPS at full resolution), free running or single
conversion, interrupt on conversion complete, sleep mode noise canceler,
optional left-adjusted result. PRADC in PRR must be 0 to enable the ADC.

Block diagram: the input multiplexer (MUX decoder from ADMUX) and the
reference selection (AV~CC~, internal 1.1 V, AREF) feed a 10-bit DAC and
comparator in the sample-and-hold and conversion logic, clocked by the
prescaler (ADCSRA) and producing ADC[9:0] in ADCH/ADCL and the
conversion-complete interrupt.

The ADC has a separate analog supply, AV~CC~, which must be within ±0.3 V
of V~CC~ (see the noise canceling techniques below). The internal
references (1.1 V, AV~CC~) can be decoupled by a capacitor on AREF. The
result ranges from GND (0) to V~REF~ − 1 LSB.

The channel is selected with MUX in ADMUX; reference and channel take
effect only once ADEN is set. With ADEN = 0 the ADC draws no power, so
switch it off before power-saving sleep modes. The result is right
adjusted in ADCH:ADCL, or left adjusted with ADLAR = 1; with a left
adjusted result and 8 bits enough, reading ADCH suffices. Otherwise read
ADCL first, then ADCH: reading ADCL blocks ADC access to both registers
until ADCH is read, so a conversion completing in between is lost (its
interrupt still triggers).

#### Starting a conversion

- **Single conversion:** with PRADC = 0, write ADSC = 1. ADSC stays 1
  while the conversion runs and is cleared by hardware at the end. A
  channel change during a conversion takes effect after it.
- **Auto trigger:** ADATE = 1; ADTS in ADCSRB selects the source. A
  positive edge on the trigger resets the ADC prescaler and starts a
  conversion (conversions at fixed intervals). If the trigger is still
  set when the conversion completes, no new one starts; an edge during a
  conversion is ignored. The trigger's interrupt flag is set even when its
  interrupt is disabled or the I bit is clear, so a conversion can be
  triggered without an interrupt, but the flag must be cleared before the
  next event can trigger again.
- **Free running:** the ADC interrupt flag as trigger source starts a new
  conversion as soon as one finishes, whether or not ADIF is cleared.
  Start the first one by writing ADSC = 1.

With auto trigger enabled, writing ADSC = 1 still starts a single
conversion. ADSC reads 1 during any conversion, however it was started.

#### Prescaling and conversion timing

Full 10-bit resolution needs an ADC clock of 50 kHz to 200 kHz; for lower
resolution the clock may be higher, for a higher sample rate. The 7-bit
prescaler (ADPS in ADCSRA, CK/2 to CK/128) makes a suitable clock from any
CPU clock above 100 kHz; it runs while ADEN = 1 and is held reset while
ADEN = 0.

A conversion started with ADSC begins on the next rising ADC clock edge.
A normal conversion takes 13 ADC clocks; the first one after ADEN is set
takes 25, to initialize the analog circuitry. With the bandgap as input it
needs time to stabilize, and the first value read after the first
conversion may be wrong. Sample-and-hold happens 1.5 ADC clocks after the
start of a normal conversion, 13.5 after the start of a first conversion.
At the end, the result is written to the data registers and ADIF is set;
in single conversion mode ADSC is cleared at the same time, and setting it
again starts a new conversion on the first rising ADC clock edge.

With auto trigger, the prescaler is reset at the trigger event, giving a
fixed delay to the start of conversion; sample-and-hold happens two ADC
clocks after the rising edge of the trigger, and three more CPU clocks
are used for synchronization. In free running mode a new conversion
starts right after one completes, while ADSC stays 1.

```table id=adc-conv-time caption="ADC conversion time"
| Condition | Sample-and-hold (ADC clocks from start of conversion) | Conversion time (ADC clocks) |
|---|--:|--:|
| First conversion | 13.5 | 25 |
| Normal conversion, single ended | 1.5 | 13 |
| Auto triggered conversion | 2 | 13.5 |
```

```wavedrom id=adc-first caption="First conversion (single conversion mode)"
{"signal": [
  {"name": "ADC clock", "wave": "p.|......|....|..."},
  {"name": "ADEN", "wave": "01|..............."},
  {"name": "ADSC", "wave": "01|...........0..."},
  {"name": "ADIF", "wave": "0.|...........1..."},
  {"name": "ADCH, ADCL", "wave": "x.|...........=...", "data": ["result"]},
  {"name": "event", "wave": "x.=.|.=..|.....=x.", "data": ["MUX, REFS update", "sample & hold (13.5)", "complete (25), MUX, REFS update"]}
 ],
 "config": {"hscale": 2}}
```

```wavedrom id=adc-single caption="Single conversion"
{"signal": [
  {"name": "ADC clock", "wave": "p............."},
  {"name": "ADSC", "wave": "1..........0.."},
  {"name": "ADIF", "wave": "0..........1.."},
  {"name": "ADCH, ADCL", "wave": "x..........=..", "data": ["result"]},
  {"name": "event", "wave": "x=.=.......=x.", "data": ["MUX, REFS update", "sample & hold (1.5)", "complete (13), MUX, REFS update"]}
 ],
 "config": {"hscale": 2}}
```

```wavedrom id=adc-auto caption="Auto triggered conversion"
{"signal": [
  {"name": "ADC clock", "wave": "p............."},
  {"name": "Trigger source", "wave": "01.0.........."},
  {"name": "ADATE", "wave": "1............."},
  {"name": "ADIF", "wave": "0..........1.."},
  {"name": "ADCH, ADCL", "wave": "x..........=..", "data": ["result"]},
  {"name": "event", "wave": "x==.=......=x.", "data": ["prescaler reset", "MUX, REFS update", "sample & hold (2)", "complete (13.5), prescaler reset"]}
 ],
 "config": {"hscale": 2}}
```

```wavedrom id=adc-free caption="Free running conversion"
{"signal": [
  {"name": "ADC clock", "wave": "p......"},
  {"name": "ADSC", "wave": "1......"},
  {"name": "ADIF", "wave": "0..1..."},
  {"name": "ADCH, ADCL", "wave": "x..=...", "data": ["result"]},
  {"name": "event", "wave": "x..=.=.", "data": ["complete (cycle 13); MUX, REFS update", "sample & hold"]}
 ],
 "config": {"hscale": 2}}
```

#### Changing channel or reference

MUX and REFS in ADMUX are single-buffered through a temporary register the
CPU accesses freely; the selection is updated continuously until a
conversion starts, then locked (for sufficient sampling time), and updates
resume in the last ADC clock before the conversion completes (ADIF set).
The conversion starts on the rising ADC clock edge after ADSC is written,
so write new MUX or REFS values no sooner than one ADC clock after ADSC.
With auto trigger the trigger time can be indeterministic, and with ADATE
and ADEN both 1 an event can come at any time; ADMUX is safely updated
(affecting the next conversion):

1. while ADATE or ADEN is 0;
2. during a conversion, at least one ADC clock after the trigger event;
3. after a conversion, before the interrupt flag used as trigger source
   is cleared.

**Input channels.** Single conversion mode: select the channel before
starting; it may be changed one ADC clock after ADSC is written, but the
simplest is to wait for the conversion to complete. Free running mode:
select the channel before the first conversion; it may be changed one ADC
clock after ADSC is written, but the simplest is to wait for the first
conversion to complete and then change it; the next result still reflects
the old channel (that conversion has already started), the ones after it
the new one.

**Voltage reference.** V~REF~ sets the conversion range; single-ended
inputs above V~REF~ give codes close to 0x3FF. V~REF~ is AV~CC~ (through a
passive switch), the internal 1.1 V (from the bandgap V~BG~ through an
amplifier) or the AREF pin. AREF connects directly to the ADC in every
case: a capacitor from AREF to ground reduces noise, and V~REF~ can be
measured there with a high-impedance voltmeter. V~REF~ is a
high-impedance source; load it only capacitively. With a fixed voltage on
AREF the other reference options must not be used (they would be shorted
to it); without one, AV~CC~ and 1.1 V may be switched. Discard the first
result after switching the reference.

#### ADC noise canceler

Converts during sleep (ADC noise reduction or idle mode) to reduce noise
from the CPU core and other I/O:

1. Make sure the ADC is enabled and not converting; select single
   conversion mode and enable the conversion-complete interrupt.
2. Enter ADC noise reduction (or idle) mode. The conversion starts once
   the CPU has halted.
3. If no other interrupt comes first, the ADC interrupt wakes the CPU and
   its routine runs. If another interrupt wakes the CPU first, it runs, and
   the ADC interrupt request follows when the conversion completes. The CPU
   stays active until the next SLEEP.

The ADC is not switched off automatically in other sleep modes; write
ADEN = 0 before entering them to avoid excessive power consumption.

#### Analog input circuitry

A source on ADCn sees the pin capacitance and input leakage (I~IH~, I~IL~)
whether or not the channel is selected; when selected, it must charge the
sample-and-hold capacitor C~S/H~ = 14 pF (referred to V~CC~/2) through the
input path's series resistance of 1–100 kΩ. The ADC is optimized for
sources of about 10 kΩ or less output impedance, where the sampling time
is negligible; with higher impedance the sampling time depends on how long
the source takes to charge C~S/H~, which can vary widely. Use low-impedance
sources with slowly varying signals. Signal components above the Nyquist
frequency (f~ADC~/2) must not be present; remove them with a low-pass
filter.

#### Analog noise canceling techniques

1. Keep analog signal paths as short as possible, run analog tracks over
   the analog ground plane, and keep them well away from high-speed
   switching digital tracks.
2. Connect AV~CC~ to the digital V~CC~ through an LC network (10 µH series,
   100 nF to the analog ground plane); decouple AREF to the analog ground
   plane.
3. Use the ADC noise canceler to reduce noise from the CPU.
4. ADC[3:0] port pins used as digital outputs must not switch while a
   conversion is in progress. Using the 2-wire interface (ADC4, ADC5)
   affects only conversions on ADC4 and ADC5.

#### Accuracy definitions

An n-bit single-ended ADC converts GND to V~REF~ linearly in 2^n^ steps
(LSBs); the lowest code reads 0, the highest 2^n^ − 1.

- **Offset:** deviation of the first transition (0x000 to 0x001) from the
  ideal (at 0.5 LSB). Ideal: 0 LSB.
- **Gain error:** after adjusting for offset, deviation of the last
  transition (0x3FE to 0x3FF) from the ideal (1.5 LSB below maximum).
  Ideal: 0 LSB.
- **Integral non-linearity (INL):** after adjusting for offset and gain,
  the maximum deviation of an actual transition from the ideal one for any
  code. Ideal: 0 LSB.
- **Differential non-linearity (DNL):** maximum deviation of an actual
  code width (between two adjacent transitions) from the ideal 1 LSB.
  Ideal: 0 LSB.
- **Quantization error:** a 1 LSB wide range of inputs codes to the same
  value. Always ±0.5 LSB.
- **Absolute accuracy:** maximum deviation of an actual (unadjusted)
  transition from the ideal for any code: the compound effect of offset,
  gain error, differential error, non-linearity and quantization error.
  Ideal: ±0.5 LSB.

#### Conversion result

After the conversion (ADIF = 1) the result is in ADCL and ADCH. For a
single-ended conversion, with V~IN~ on the selected input and V~REF~ the
selected reference, 0x000 is analog ground and 0x3FF is V~REF~ − 1 LSB:

```equation
ADC = V~IN~ × 1024 / V~REF~
```

#### Temperature measurement

The on-chip temperature sensor is the single-ended channel ADC8, enabled
by MUX[3:0] = 1000, with the internal 1.1 V reference selected; measure it
in single conversion mode. The voltage is linear in temperature, about
1 mV/°C, with ±10 °C accuracy. The output varies from chip to chip; for
better accuracy, calibrate in software with an offset T~OS~ measured and
stored (in a register or EEPROM) per chip in production, and a fixed
coefficient k:

```table id=adc-temp caption="Temperature sensor output voltage, typical"
| Temperature | -45 °C | 25 °C | 85 °C |
|---|--:|--:|--:|
| Voltage | 242 mV | 314 mV | 380 mV |
```

```equation
T = ((ADCH << 8) | ADCL − T~OS~) / k
```

#### ADMUX – ADC Multiplexer Selection Register, (0x7C)

```table id=adc-admux caption="ADMUX bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7:6 | REFS[1:0] | R/W | 00 | Reference selection (table below). A change during a conversion takes effect when it completes (ADIF set). The internal reference options must not be used with an external voltage on AREF. |
| 5 | ADLAR | R/W | 0 | 1 = left adjust the result, 0 = right adjust; takes effect on the data register at once, whatever conversion is running |
| 3:0 | MUX[3:0] | R/W | 0000 | Input channel (table below). A change during a conversion takes effect when it completes (ADIF set). |
```

```table id=adc-refs caption="Voltage reference selection"
| REFS1 | REFS0 | Reference |
|:-:|:-:|---|
| 0 | 0 | AREF, internal V~REF~ off |
| 0 | 1 | AV~CC~, with external capacitor at AREF |
| 1 | 0 | Reserved |
| 1 | 1 | Internal 1.1 V, with external capacitor at AREF |
```

```table id=adc-mux caption="Input channel selection"
| MUX[3:0] | Single-ended input |
|:-:|---|
| 0000–0111 | ADC0–ADC7 |
| 1000 | ADC8, temperature sensor |
| 1001–1101 | Reserved |
| 1110 | 1.1 V (V~BG~) |
| 1111 | 0 V (GND) |
```

#### ADCSRA – ADC Control and Status Register A, (0x7A)

```table id=adc-adcsra caption="ADCSRA bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | ADEN | R/W | 0 | ADC enable; writing 0 turns the ADC off and terminates a conversion in progress |
| 6 | ADSC | R/W | 0 | Start conversion: in single conversion mode write 1 to start each conversion, in free running mode to start the first. The first conversion after the ADC is enabled (or with ADSC written together with ADEN) takes 25 ADC clocks instead of 13 and initializes the ADC. Reads 1 while a conversion runs, 0 when done; writing 0 has no effect. |
| 5 | ADATE | R/W | 0 | Auto trigger enable: a conversion starts on a positive edge of the trigger selected by ADTS in ADCSRB |
| 4 | ADIF | R/W | 0 | Interrupt flag: set when a conversion completes and the data registers are updated; the interrupt runs if ADIE and the I bit are set. Cleared by hardware when the vector executes, or by writing 1. A read-modify-write of ADCSRA (also SBI, CBI) can clear a pending flag. |
| 3 | ADIE | R/W | 0 | Conversion complete interrupt enable (with the I bit set) |
| 2:0 | ADPS[2:0] | R/W | 000 | Prescaler, system clock ÷ ADC clock: 000 = 2, 001 = 2, 010 = 4, 011 = 8, 100 = 16, 101 = 32, 110 = 64, 111 = 128 |
```

#### ADCL and ADCH – ADC Data Register, (0x78), (0x79)

Read-only, reset 0. ADC[9:0] is the conversion result. Once ADCL is
read, the register is not updated until ADCH is read; with a left
adjusted result and 8-bit precision, reading ADCH is enough, otherwise
read ADCL first.

```table id=adc-data caption="ADC data register layout"
| ADLAR | Register | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 |
|:-:|---|---|---|---|---|---|---|---|---|
| 0 | ADCH | – | – | – | – | – | – | ADC9 | ADC8 |
| ^ | ADCL | ADC7 | ADC6 | ADC5 | ADC4 | ADC3 | ADC2 | ADC1 | ADC0 |
| 1 | ADCH | ADC9 | ADC8 | ADC7 | ADC6 | ADC5 | ADC4 | ADC3 | ADC2 |
| ^ | ADCL | ADC1 | ADC0 | – | – | – | – | – | – |
```

#### ADCSRB – ADC Control and Status Register B (ADTS), (0x7B)

```table id=adc-adcsrb caption="ADCSRB bits"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 6 | ACME | R/W | 0 | Analog comparator multiplexer enable (see [analog comparator](#analog-comparator)) |
| 2:0 | ADTS[2:0] | R/W | 000 | Auto trigger source, used only with ADATE = 1 (table below). The rising edge of the selected interrupt flag triggers a conversion. Switching from a cleared to a set trigger source makes a positive edge, which starts a conversion if ADEN is set. Switching to free running (000) is no trigger event, even with ADIF set. |
```

```table id=adc-adts caption="Auto trigger source"
| ADTS[2:0] | Trigger source |
|:-:|---|
| 000 | Free running mode |
| 001 | Analog comparator |
| 010 | External interrupt request 0 |
| 011 | Timer/Counter0 compare match A |
| 100 | Timer/Counter0 overflow |
| 101 | Timer/Counter1 compare match B |
| 110 | Timer/Counter1 overflow |
| 111 | Timer/Counter1 capture event |
```

#### DIDR0 – Digital Input Disable Register 0, (0x7E)

```table id=adc-didr0 caption="DIDR0 bits"
| Bit | Name | Description |
|---|---|---|
| 5:0 | ADC5D–ADC0D | 1 = digital input buffer on ADC5…ADC0 off (its PIN bit reads 0). Set when the pin carries an analog signal and its digital input isn't needed, to save power. ADC7 and ADC6 have no digital input buffers. |
```

#### ADC characteristics

```table id=adc-chars caption="ADC characteristics"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| | Resolution | | | 10 | | bits |
| | Absolute accuracy (including INL, DNL, quantization, gain and offset error) | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 2 | | LSB |
| ^ | ^ | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 1 MHz | | 4.5 | | LSB |
| ^ | ^ | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz, noise reduction mode | | 2 | | LSB |
| ^ | ^ | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 1 MHz, noise reduction mode | | 4.5 | | LSB |
| | Integral non-linearity (INL) | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 0.5 | | LSB |
| | Differential non-linearity (DNL) | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 0.25 | | LSB |
| | Gain error | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 2 | | LSB |
| | Offset error | V~REF~ = 4 V, V~CC~ = 4 V, ADC clock = 200 kHz | | 2 | | LSB |
| | Conversion time | Free running conversion | 13 | | 260 | µs |
| | Clock frequency | | 50 | | 1000 | kHz |
| AV~CC~ | Analog supply voltage [1] | | V~CC~ - 0.3 | | V~CC~ + 0.3 | V |
| V~REF~ | Reference voltage | | 1.0 | | AV~CC~ | V |
| V~IN~ | Input voltage | | GND | | V~REF~ | V |
| | Input bandwidth | | | 38.5 | | kHz |
| V~INT~ | Internal voltage reference | | 1.0 | 1.1 | 1.2 | V |
| R~REF~ | Reference input resistance | | | 32 | | kΩ |
| R~AIN~ | Analog input resistance | | | 100 | | MΩ |

[1] AV~CC~ absolute minimum 1.8 V, maximum 5.5 V.
```

### debugWIRE

A one-wire, bidirectional on-chip debug interface: it controls program flow, executes AVR
instructions in the CPU and programs the non-volatile memories. It emulates every on-chip
function, digital and analog, except the !{RESET} pin, runs in real time and
non-intrusively, with electrical characteristics identical to the real device, and gives an
unlimited number of program break points (software break points).

With the DWEN fuse programmed and the lock bits unprogrammed, the !{RESET} pin becomes a
wired-AND (open-drain) bidirectional I/O with pull-up enabled, the dW line to the emulator
(target V~CC~ 1.8 V to 5.5 V). The system clock is not affected; it stays the source the CKSEL
fuses select. For correct operation:

- A pull-up on dW/!{RESET} must not be smaller than 10 kΩ; it is not required.
- Connecting !{RESET} directly to V~CC~ does not work.
- Capacitors on !{RESET} must be disconnected.
- All external reset sources must be disconnected.

**Break points.** A break point replaces an instruction in flash with BREAK (the debugger
keeps the replaced instruction and executes it before continuing); BREAK may also be placed in
the program by hand. The flash is reprogrammed every time a break point changes, which reduces
flash data retention: parts used for debugging should not be shipped.

**Limitations.** dW shares the pin with external reset, so an external reset source is not
supported while debugWIRE is enabled. A programmed DWEN fuse keeps parts of the clock system
running in every sleep mode, which raises sleep current: unprogram DWEN when debugWIRE is not
used.

#### DWDR – debugWIRE Data Register

DWDR[7:0] (R/W, reset 0x00; the source gives no address): a communication channel from the running program to the
debugger. Only debugWIRE can access it, so it can't be used as a general-purpose register.

### Self-programming (ATmega48A/48PA)

The ATmega48A/48PA have no read-while-write support and no separate boot loader section: SPM
can execute from the whole flash, if the SELFPRGEN fuse is programmed. The CPU is halted during
every page erase and page write. Software can download code over any interface and program it
into flash page by page.

A page must be erased before it is written from the temporary page buffer. The buffer is filled
one word at a time with SPM, either before the page erase (alternative 1: fill, erase, write) or
between the erase and the write (alternative 2: erase, fill, write). To change part of a page,
the rest must be saved (for example in the buffer) before the erase and written back;
alternative 1 allows read-modify-write (read the old page while loading), alternative 2 does
not. The buffer can be accessed in any order. Page erase and page write are addressed
independently and must address the same page.

Each SPM operation: set up Z (and R1:R0), write the SPMCSR value, execute SPM within four
clock cycles. Disable interrupts first, since an interrupt in between breaks the four-cycle
timing.

```table id=spm48-ops caption="SPM operations, ATmega48A/48PA"
| Operation | SPMCSR | Z-pointer | R1:R0 | Notes |
|---|---|---|---|---|
| Page erase | 00000011 | page address in PCPAGE; other bits ignored | ignored | CPU halted |
| Fill temporary buffer (page load) | 00000001 | word address in PCWORD | instruction word | Buffer auto-erases after page write, on writing RWWSRE and on reset. Each address can be written only once without erasing the buffer. An EEPROM write during page load loses all loaded data. |
| Page write | 00000101 | page address in PCPAGE; other bits must be 0 | ignored | CPU halted |
```

**Addressing.** Z (R31:R30) addresses SPM: the low bits (PCWORD) select the word within a page,
the high bits (PCPAGE) the page; page size and the PCPAGE/PCWORD bit positions are in the
[page size table](#prog-pagesize). Z0 is unused by SPM. LPM addresses flash byte by byte and
uses Z0.

**EEPROM write blocks SPM.** An EEPROM write blocks all software programming of flash and
reading fuse and lock bits from software. Check that EEPE (EECR) is clear before writing
SPMCSR.

**Reading fuse and lock bits.** Load Z, set BLBSET and SPMEN in SPMCSR, and execute LPM within
three CPU cycles: the byte is loaded into the destination register. BLBSET and SPMEN clear on
completion, or if no LPM follows within three cycles or no SPM within four; LPM then works
normally. Programmed bits read 0, unprogrammed bits 1.

```table id=spm48-fuseread caption="Fuse and lock bit reading by LPM, ATmega48A/48PA"
| Z | Byte read | Bits 7…0 |
|---|---|---|
| 0x0000 | Fuse low byte | FLB7…FLB0 |
| 0x0001 | Lock bits | –, –, –, –, –, –, LB2, LB1 |
| 0x0002 | Extended fuse byte | EFB7…EFB0 [1] |
| 0x0003 | Fuse high byte | FHB7…FHB0 |

[1] The source labels these bits FHB7…FHB0.
```

**Preventing flash corruption.** At too low V~CC~ a flash write may fail, and the CPU may execute
instructions incorrectly. One of these is sufficient: keep !{RESET} active (low) while V~CC~ is
insufficient, with the BOD if its level matches the operating voltage or an external low-V~CC~
reset circuit (a write in progress at reset completes if the supply is sufficient); or keep the
core in power-down sleep while V~CC~ is low, which protects SPMCSR from unintentional writes.

```table id=spm48-time caption="SPM programming time [1]"
| Parameter | Min | Max | Unit |
|---|--:|--:|---|
| Flash write (page erase, page write, lock-bit write by SPM) | 3.7 | 4.5 | ms |

[1] Per operation. Timed by the calibrated RC oscillator.
```

#### SPMCSR – Store Program Memory Control and Status Register (ATmega48A/48PA), 0x37 (0x57)

```table id=spm48-spmcsr caption="SPMCSR bits, ATmega48A/48PA"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | SPMIE | R/W | 0 | SPM ready interrupt enable (with the I bit): the interrupt runs as long as SPMEN is 0. Not generated during an EEPROM write or SPM. |
| 6 | RWWSB | R | 0 | Always reads 0 (kept for compatibility with read-while-write devices) |
| 5 | SIGRD | R/W | 0 | Signature row read: with SPMEN, the next LPM within three cycles reads a signature row byte; an SPM within four cycles has no effect. Reserved for future use; should not be used. |
| 4 | RWWSRE | R/W | 0 | Subset of the ATmega88/168/328 function: writing it while filling the buffer clears the buffer and its data is lost |
| 3 | BLBSET | R/W | 0 | With SPMEN, an LPM within three cycles reads the lock bits or fuse bits (selected by Z) |
| 2 | PGWRT | R/W | 0 | With SPMEN, the next SPM within four cycles writes the buffer to the page addressed by the high part of Z; R1:R0 ignored. Auto-clears on completion or if no SPM within four cycles. CPU halted during the write. |
| 1 | PGERS | R/W | 0 | With SPMEN, the next SPM within four cycles erases the page addressed by the high part of Z; R1:R0 ignored. Auto-clears on completion or if no SPM within four cycles. CPU halted. |
| 0 | SPMEN | R/W | 0 | Enables SPM for the next four cycles. Alone, SPM stores R1:R0 in the buffer word addressed by Z (Z0 ignored). Auto-clears after the SPM or after four cycles; stays 1 during page erase and page write. |
```

Writing any lower-five-bit combination other than 10001, 01001, 00101, 00011 or 00001 has no
effect.

### Boot loader and read-while-write self-programming (ATmega88/168/328)

On the ATmega88A/88PA/168A/168PA/328/328P, a flash-resident boot loader can read and write
(program) the entire flash, including the boot loader itself (it may even erase itself), with
real read-while-write. The boot section size is set by fuses, and two sets of boot lock bits
protect the sections independently.

**Application and boot loader sections.** The BOOTSZ fuses split the flash into the
application section (protected by boot lock bits 0) and the boot loader section, BLS (boot
lock bits 1). SPM can start programming only when executed from the BLS, so boot loader code
must sit there; SPM from there reaches the whole flash.

**RWW and NRWW sections.** Independently, the flash has two fixed sections. Erasing or writing a
page in the read-while-write (RWW) section, code in the no-read-while-write (NRWW) section can
run; erasing or writing a page in the NRWW section halts the CPU for the whole operation. The
BLS always lies in the NRWW section. Software must never read the RWW section (call, jmp, lpm or
an interrupt vector) while it is being programmed, or execution may end in an unknown state:
disable interrupts or move the vectors to the BLS. RWWSB reads 1 while the RWW section is
blocked; after programming, software must clear it by writing RWWSRE before reading RWW code.

```table id=boot-rww caption="Read-while-write features"
| Z addresses during programming | Section readable during programming | CPU halted | Read-while-write |
|---|---|---|---|
| RWW section | NRWW section | No | Yes |
| NRWW section | None | Yes | No |
```

```table id=boot-sizes caption="Boot size configuration (word addresses)"
| Device | BOOTSZ1 | BOOTSZ0 | Boot size (words) | Pages | Application section | Boot loader section | End application | Boot reset address (start boot loader) |
|---|:-:|:-:|--:|--:|---|---|---|---|
| ATmega88A/88PA | 1 | 1 | 128 | 4 | 0x000–0xF7F | 0xF80–0xFFF | 0xF7F | 0xF80 |
| ^ | 1 | 0 | 256 | 8 | 0x000–0xEFF | 0xF00–0xFFF | 0xEFF | 0xF00 |
| ^ | 0 | 1 | 512 | 16 | 0x000–0xDFF | 0xE00–0xFFF | 0xDFF | 0xE00 |
| ^ | 0 | 0 | 1024 | 32 | 0x000–0xBFF | 0xC00–0xFFF | 0xBFF | 0xC00 |
| ATmega168A/168PA | 1 | 1 | 128 | 2 | 0x0000–0x1F7F | 0x1F80–0x1FFF | 0x1F7F | 0x1F80 |
| ^ | 1 | 0 | 256 | 4 | 0x0000–0x1EFF | 0x1F00–0x1FFF | 0x1EFF | 0x1F00 |
| ^ | 0 | 1 | 512 | 8 | 0x0000–0x1DFF | 0x1E00–0x1FFF | 0x1DFF | 0x1E00 |
| ^ | 0 | 0 | 1024 | 16 | 0x0000–0x1BFF | 0x1C00–0x1FFF | 0x1BFF | 0x1C00 |
| ATmega328/328P | 1 | 1 | 256 | 4 | 0x0000–0x3EFF | 0x3F00–0x3FFF | 0x3EFF | 0x3F00 |
| ^ | 1 | 0 | 512 | 8 | 0x0000–0x3DFF | 0x3E00–0x3FFF | 0x3DFF | 0x3E00 |
| ^ | 0 | 1 | 1024 | 16 | 0x0000–0x3BFF | 0x3C00–0x3FFF | 0x3BFF | 0x3C00 |
| ^ | 0 | 0 | 2048 | 32 | 0x0000–0x37FF | 0x3800–0x3FFF | 0x37FF | 0x3800 |
```

The boot section is at the top of flash. With BOOTSZ = 00 the application ends where the RWW
section ends; with larger BOOTSZ values the application section reaches into the NRWW section.

```table id=boot-rwwlimit caption="Read-while-write limits (word addresses)"
| Device | RWW pages | RWW addresses | NRWW pages | NRWW addresses |
|---|--:|---|--:|---|
| ATmega88A/88PA | 96 | 0x000–0xBFF | 32 | 0xC00–0xFFF |
| ATmega168A/168PA | 112 | 0x0000–0x1BFF | 16 | 0x1C00–0x1FFF |
| ATmega328/328P | 224 | 0x0000–0x37FF | 32 | 0x3800–0x3FFF |
```

**Boot lock bits.** They can be set by software (SPM) and in serial or parallel programming, but
cleared only by chip erase. The general write lock (lock mode 2) does not control SPM
programming, and the general read/write lock (mode 1) does not control LPM/SPM. 1 =
unprogrammed, 0 = programmed.

```table id=boot-blb0 caption="Boot lock bit 0 protection modes (application section)"
| BLB0 mode | BLB02 | BLB01 | Protection |
|--:|:-:|:-:|---|
| 1 | 1 | 1 | No restrictions for SPM or LPM accessing the application section |
| 2 | 1 | 0 | SPM may not write the application section |
| 3 | 0 | 0 | SPM may not write the application section, and LPM executing from the BLS may not read it. With the vectors in the BLS, interrupts are disabled while executing from the application section. |
| 4 | 0 | 1 | LPM executing from the BLS may not read the application section. With the vectors in the BLS, interrupts are disabled while executing from the application section. |
```

```table id=boot-blb1 caption="Boot lock bit 1 protection modes (boot loader section)"
| BLB1 mode | BLB12 | BLB11 | Protection |
|--:|:-:|:-:|---|
| 1 | 1 | 1 | No restrictions for SPM or LPM accessing the BLS |
| 2 | 1 | 0 | SPM may not write the BLS |
| 3 | 0 | 0 | SPM may not write the BLS, and LPM executing from the application section may not read it. With the vectors in the application section, interrupts are disabled while executing from the BLS. |
| 4 | 0 | 1 | LPM executing from the application section may not read the BLS. With the vectors in the application section, interrupts are disabled while executing from the BLS. |
```

**Entering the boot loader.** By a jump or call from the application (triggered, for example,
by a USART or SPI command), or by programming BOOTRST so the reset vector points to the boot
reset address. The MCU can't change its own fuses: once BOOTRST is programmed, it can be
changed only through serial or parallel programming.

```table id=boot-bootrst caption="Boot reset fuse"
| BOOTRST | Reset address |
|:-:|---|
| 1 (unprogrammed) | Application reset, 0x0000 |
| 0 (programmed) | Boot loader reset (boot reset address in the boot size table) |
```

**Addressing.** As on the ATmega48: Z selects page (PCPAGE) and word (PCWORD); page erase and
page write must address the same page. Once an operation starts, the address is latched and Z
is free. Setting the lock bits is the only SPM operation that ignores Z. LPM uses Z0 as byte
select.

```table id=boot-z caption="Program counter and Z-pointer mapping [1]"
| Variable | Meaning | ATmega88A/88PA | ATmega168A/168PA | ATmega328/328P |
|---|---|---|---|---|
| PCMSB | Most significant program counter bit | 11 (PC[11:0]) | 12 (PC[12:0]) | 13 (PC[13:0]) |
| PAGEMSB | Most significant bit addressing words in a page | 4 (32 words, PC[4:0]) | 5 (64 words, PC[5:0]) | 5 (64 words, PC[5:0]) |
| ZPCMSB | Z bit mapped to PCMSB (PCMSB + 1) | Z12 | Z13 | Z14 |
| ZPAGEMSB | Z bit mapped to PAGEMSB (PAGEMSB + 1) | Z5 | Z6 | Z6 |
| PCPAGE | Page select, for page erase and page write | PC[11:5] = Z12:Z6 | PC[12:6] = Z13:Z7 | PC[13:6] = Z14:Z7 |
| PCWORD | Word select, for filling the buffer (must be 0 during page write) | PC[4:0] = Z5:Z1 | PC[5:0] = Z6:Z1 | PC[5:0] = Z6:Z1 |
| Ignored Z bits | | Z15:Z13 | Z15:Z14 | Z15 |

[1] Z0 should be 0 for all SPM commands; it is the byte select for LPM.
```

**Self-programming.** Pages are erased and written as on the ATmega48 (both buffer-fill
alternatives, read-modify-write with alternative 1, buffer in any order). Each operation: set
up Z (and R0 or R1:R0), write SPMCSR, execute SPM within four clock cycles.

```table id=boot-ops caption="SPM operations, ATmega88/168/328"
| Operation | SPMCSR | Z-pointer | Data | During the operation |
|---|---|---|---|---|
| Page erase | X0000011 | PCPAGE; other bits ignored | R1:R0 ignored | RWW page: NRWW can be read. NRWW page: CPU halted. |
| Fill temporary buffer | 00000001 | PCWORD | R1:R0 = instruction word | Buffer auto-erases after page write, on writing RWWSRE and on reset; each address writable once per erase; an EEPROM write during page load loses all loaded data |
| Page write | X0000101 | PCPAGE; other bits must be 0 | R1:R0 ignored | RWW page: NRWW can be read. NRWW page: CPU halted. |
| Re-enable RWW section | X0010001 | ignored | – | After SPMEN clears |
| Set boot and lock bits | X0001001 | don't care (load 0x0001 for future compatibility) | R0 = 1, 1, BLB12, BLB11, BLB02, BLB01, LB2, LB1 | The whole flash can be read |
```

For setting the lock bits, a 0 in R0 bits 5:0 programs that lock bit; write bits 7:6 as 1 for
future compatibility.

With SPMIE set, the SPM ready interrupt fires continuously while SPMEN is 0, so it can replace
polling SPMCSR; move the vectors to the BLS so no interrupt reads the blocked RWW section.
During any page erase or write the RWW section is blocked for reading; move the vector table
to the BLS or disable interrupts, and clear RWWSB (write RWWSRE) before addressing the RWW
section again. If BLB11 is left unprogrammed, an accidental write to the boot loader can
corrupt it and make further updates impossible: program BLB11 unless the boot loader itself
must change.

An EEPROM write blocks SPM and the reading of fuse and lock bits: check that EEPE is clear
before writing SPMCSR.

**Reading fuse and lock bits** works as on the ATmega48 (BLBSET + SPMEN, LPM within three
cycles; programmed bits read 0):

```table id=boot-fuseread caption="Fuse and lock bit reading by LPM, ATmega88/168/328"
| Z | Byte read | Bits 7…0 |
|---|---|---|
| 0x0000 | Fuse low byte | FLB7…FLB0 |
| 0x0001 | Lock bits | –, –, BLB12, BLB11, BLB02, BLB01, LB2, LB1 |
| 0x0002 | Extended fuse byte | –, –, –, –, EFB3…EFB0 |
| 0x0003 | Fuse high byte | FHB7…FHB0 |
```

**Reading the signature row.** Load Z with the address below, set SIGRD and SPMEN, and execute
LPM within three CPU cycles. SIGRD and SPMEN auto-clear on completion or if no LPM follows
within three cycles.

```table id=boot-sigrow caption="Signature row addressing"
| Byte | Z |
|---|---|
| Device signature byte 1 | 0x0000 |
| Device signature byte 2 | 0x0002 |
| Device signature byte 3 | 0x0004 |
| RC oscillator calibration byte | 0x0001 |
| Reserved | all other addresses |
```

**Preventing flash corruption.** One of these is sufficient: program the boot lock bits if the
boot loader never needs updating; keep !{RESET} low while V~CC~ is insufficient (BOD or an
external circuit; a write in progress at reset completes if the supply is sufficient); or keep
the core in power-down sleep while V~CC~ is low.

Programming time: as in the [SPM programming time table](#spm48-time) (3.7 ms to 4.5 ms per
page erase, page write or lock-bit write).

#### SPMCSR – Store Program Memory Control and Status Register (ATmega88/168/328), 0x37 (0x57)

```table id=boot-spmcsr caption="SPMCSR bits, ATmega88/168/328"
| Bit | Name | Access | Reset | Description |
|---|---|---|---|---|
| 7 | SPMIE | R/W | 0 | SPM ready interrupt enable (with the I bit): the interrupt runs as long as SPMEN is 0 |
| 6 | RWWSB | R | 0 | RWW section busy: set by hardware when a page erase or write to the RWW section starts; the RWW section can't be accessed while set. Cleared by writing RWWSRE after the operation completes, or automatically when a page load starts. |
| 5 | SIGRD | R/W | 0 | Signature row read: with SPMEN, the next LPM within three cycles reads a signature row byte; an SPM within four cycles has no effect. Reserved for future use; should not be used. |
| 4 | RWWSRE | R/W | 0 | RWW section read enable: after programming completes (SPMEN = 0), writing it with SPMEN makes the next SPM within four cycles re-enable the RWW section. Can't re-enable while SPMEN is 1. Writing it during a buffer load aborts the load and loses the data. |
| 3 | BLBSET | R/W | 0 | With SPMEN, the next SPM within four cycles sets the boot and memory lock bits from R0 (R1 and Z ignored); auto-clears on completion or after four cycles. An LPM within three cycles instead reads the lock or fuse bits (selected by Z). |
| 2 | PGWRT | R/W | 0 | With SPMEN, the next SPM within four cycles writes the buffer to the page in the high part of Z; R1:R0 ignored. Auto-clears on completion or after four cycles. CPU halted if the NRWW section is addressed. |
| 1 | PGERS | R/W | 0 | With SPMEN, the next SPM within four cycles erases the page in the high part of Z; R1:R0 ignored. Auto-clears on completion or after four cycles. CPU halted if the NRWW section is addressed. |
| 0 | SPMEN | R/W | 0 | Enables SPM for the next four cycles. Alone, SPM stores R1:R0 in the buffer word addressed by Z (Z0 ignored). Auto-clears after the SPM or after four cycles; stays 1 during page erase and page write. |
```

Writing any lower-five-bit combination other than 10001, 01001, 00101, 00011 or 00001 has no
effect.

### Memory programming

Lock bits, fuse bits, signature and calibration bytes are described in the [memory map](#lock-bits).

#### Page size

```table id=prog-pagesize caption="Flash and EEPROM pages"
| Device | Flash size | Flash page | PCWORD | Flash pages | PCPAGE | PCMSB | EEPROM size | EEPROM page | EEPROM word select | EEPROM pages | EEPROM page select | EEAMSB |
|---|---|---|---|--:|---|--:|---|---|---|--:|---|--:|
| ATmega48A/48PA | 2K words (4 KB) | 32 words | PC[4:0] | 64 | PC[10:5] | 10 | 256 bytes | 4 bytes | EEA[1:0] | 64 | EEA[7:2] | 7 |
| ATmega88A/88PA | 4K words (8 KB) | 32 words | PC[4:0] | 128 | PC[11:5] | 11 | 512 bytes | 4 bytes | EEA[1:0] | 128 | EEA[8:2] | 8 |
| ATmega168A/168PA | 8K words (16 KB) | 64 words | PC[5:0] | 128 | PC[12:6] | 12 | 512 bytes | 4 bytes | EEA[1:0] | 128 | EEA[8:2] | 8 |
| ATmega328/328P | 16K words (32 KB) | 64 words | PC[5:0] | 256 | PC[13:6] | 13 | 1 KB | 4 bytes | EEA[1:0] | 256 | EEA[9:2] | 9 |
```

#### Parallel programming

Parallel (high-voltage) programming programs and verifies flash, EEPROM, lock bits and fuse
bits. Pulses are at least 250 ns unless noted. V~CC~ = 4.5 V to 5.5 V; AV~CC~ within
V~CC~ ± 0.3 V and always 4.5 V to 5.5 V; 12 V on !{RESET}; a clock on XTAL1.

```table id=prog-pins caption="Parallel programming signals"
| Signal | Pin | I/O | Function |
|---|---|---|---|
| RDY/!{BSY} | PD1 | O | 0 = busy programming, 1 = ready for a new command |
| !{OE} | PD2 | I | Output enable (active low) |
| !{WR} | PD3 | I | Write pulse (active low) |
| BS1 | PD4 | I | Byte select 1: 0 = low byte, 1 = high byte |
| XA0 | PD5 | I | XTAL action bit 0 |
| XA1 | PD6 | I | XTAL action bit 1 |
| PAGEL | PD7 | I | Program memory and EEPROM data page load |
| BS2 | PC2 | I | Byte select 2: 0 = low byte, 1 = second high byte |
| DATA | PC[1:0], PB[5:0] | I/O | Bidirectional data bus (output while !{OE} is low) |
```

Prog_enable[3:0] = PAGEL, XA1, XA0, BS1, all 0 to enter programming mode.

```table id=prog-xa caption="XA1/XA0: action on a positive XTAL1 pulse"
| XA1 | XA0 | Action |
|:-:|:-:|---|
| 0 | 0 | Load flash or EEPROM address (high or low byte by BS1) |
| 0 | 1 | Load data (high or low flash data byte by BS1) |
| 1 | 0 | Load command |
| 1 | 1 | No action, idle |
```

A pulse on !{WR} or !{OE} executes the loaded command:

```table id=prog-cmd caption="Command bytes"
| Command byte | Command |
|---|---|
| 1000 0000 | Chip erase |
| 0100 0000 | Write fuse bits |
| 0010 0000 | Write lock bits |
| 0001 0000 | Write flash |
| 0001 0001 | Write EEPROM |
| 0000 1000 | Read signature bytes and calibration byte |
| 0000 0100 | Read fuse and lock bits |
| 0000 0010 | Read flash |
| 0000 0011 | Read EEPROM |
| 0000 0000 | No operation |
```

**Entering programming mode.**

1. Set Prog_enable to 0000, !{RESET} to 0 V and V~CC~ to 0 V.
2. Apply 4.5 V to 5.5 V between V~CC~ and GND; V~CC~ must reach at least 1.8 V within 20 µs.
3. Wait 20 µs to 60 µs, then apply 11.5 V to 12.5 V to !{RESET}.
4. Keep Prog_enable unchanged for at least 10 µs after the high voltage is applied, so the
   Prog_enable signature is latched.
5. Wait at least 300 µs before giving any parallel programming command.
6. Exit programming mode by powering down or by bringing !{RESET} to 0 V.

If V~CC~ can't rise that fast: after step 2, monitor V~CC~ and apply 11.5 V to 12.5 V to
!{RESET} as soon as V~CC~ reaches 0.9 V to 1.1 V; keep Prog_enable unchanged for at least
10 µs; wait until V~CC~ reaches 4.5 V to 5.5 V before any command; exit as above.

**Efficient programming.** The loaded command and address are retained: load the command once
for many locations; skip writing 0xFF (the content of the whole flash, and of the EEPROM unless
EESAVE is programmed, after chip erase); load the address high byte only for a new 256-word
flash or 256-byte EEPROM window (also for signature reading).

**Loading steps** (letters as used below):

- A, load command: XA1, XA0 = 10; BS1 = 0; DATA = command; positive XTAL1 pulse.
- B, load address low byte: XA1, XA0 = 00; BS1 = 0; DATA = address low byte; positive XTAL1 pulse.
- C, load data low byte: XA1, XA0 = 01; DATA = data low byte; positive XTAL1 pulse.
- D, load data high byte: BS1 = 1; XA1, XA0 = 01; DATA = data high byte; positive XTAL1 pulse.
- E, latch data: BS1 = 1; positive PAGEL pulse.
- G, load address high byte: XA1, XA0 = 00; BS1 = 1; DATA = address high byte; positive XTAL1 pulse.

**Chip erase** erases flash, EEPROM (unless EESAVE is programmed) and lock bits; the lock bits
are not reset until the flash is completely erased; fuses are unchanged. Required before
reprogramming flash or EEPROM. A with 1000 0000, then a negative !{WR} pulse (RDY/!{BSY} goes
low); wait for RDY/!{BSY} high before the next command.

**Programming the flash** (page buffer, page sizes in the [page size table](#prog-pagesize)):

1. A: command 0001 0000.
2. B: address low byte.
3. C: data low byte.
4. D: data high byte.
5. E: latch data.
6. Repeat 2–5 until the buffer is full or the page's data is loaded. The low address bits
   select the word in the page; with fewer than 8 word bits (page < 256 words), the upper
   bits of the address low byte select the page for the page write.
7. G: address high byte.
8. Program the page: negative !{WR} pulse (RDY/!{BSY} goes low); wait for RDY/!{BSY} high.
9. Repeat 2–8 until all data is programmed.
10. End page programming: A with 0000 0000 (no operation), which resets the internal write
    signals.

**Programming the EEPROM** (page buffer): A with 0001 0001; G; then B, C, E for each byte until
the buffer is full; then BS1 = 0, negative !{WR} pulse, wait for RDY/!{BSY} high before the next
page.

**Reading.** Flash: A with 0000 0010, G, B; !{OE} = 0 and BS1 = 0 gives the word's low byte on
DATA, BS1 = 1 the high byte; then !{OE} = 1. EEPROM: A with 0000 0011, G, B; !{OE} = 0, BS1 = 0
gives the byte; !{OE} = 1.

**Programming fuses and lock bits.** Load A with 0100 0000 (fuses) or 0010 0000 (lock bits),
then C with the data (bit = 0 programs, 1 erases a fuse; 0 programs a lock bit), select the
byte with BS2/BS1, give a negative !{WR} pulse and wait for RDY/!{BSY} high; return BS1 or BS2
to 0 afterwards. With LB mode 3 programmed, the boot lock bits can't be programmed by any
external programming mode. Lock bits are cleared only by chip erase.

```table id=prog-bs caption="Byte selection for fuse and lock bit access"
| Byte | Write: command | Write: BS2 | Write: BS1 | Read (command 0000 0100, !{OE} = 0): BS2 | Read: BS1 |
|---|---|:-:|:-:|:-:|:-:|
| Fuse low byte | 0100 0000 | 0 | 0 | 0 | 0 |
| Fuse high byte | 0100 0000 | 0 | 1 | 1 | 1 |
| Extended fuse byte | 0100 0000 | 1 | 0 | 1 | 0 |
| Lock bits | 0010 0000 | | | 0 | 1 |
```

Read values: 0 = programmed. Signature bytes: A with 0000 1000, B with 0x00–0x02, !{OE} = 0 and
BS1 = 0 gives the byte. Calibration byte: A with 0000 1000, B with 0x00, !{OE} = 0 and
BS1 = 1.

```table id=prog-par-char caption="Parallel programming characteristics, V~CC~ = 5 V ± 10 %"
| Symbol | Parameter | Min | Typ | Max | Unit |
|---|---|--:|--:|--:|---|
| V~PP~ | Programming enable voltage | 11.5 | | 12.5 | V |
| I~PP~ | Programming enable current | | | 250 | µA |
| t~DVXH~ | Data and control valid before XTAL1 high | 67 | | | ns |
| t~XLXH~ | XTAL1 low to XTAL1 high | 200 | | | ns |
| t~XHXL~ | XTAL1 pulse width high | 150 | | | ns |
| t~XLDX~ | Data and control hold after XTAL1 low | 67 | | | ns |
| t~XLWL~ | XTAL1 low to !{WR} low | 0 | | | ns |
| t~XLPH~ | XTAL1 low to PAGEL high | 0 | | | ns |
| t~PLXH~ | PAGEL low to XTAL1 high | 150 | | | ns |
| t~BVPH~ | BS1 valid before PAGEL high | 67 | | | ns |
| t~PHPL~ | PAGEL pulse width high | 150 | | | ns |
| t~PLBX~ | BS1 hold after PAGEL low | 67 | | | ns |
| t~WLBX~ | BS2/1 hold after !{WR} low | 67 | | | ns |
| t~PLWL~ | PAGEL low to !{WR} low | 67 | | | ns |
| t~BVWL~ | BS1 valid to !{WR} low | 67 | | | ns |
| t~WLWH~ | !{WR} pulse width low | 150 | | | ns |
| t~WLRL~ | !{WR} low to RDY/!{BSY} low | 0 | | 1 | µs |
| t~WLRH~ | !{WR} low to RDY/!{BSY} high [1] | 3.7 | | 4.5 | ms |
| t~WLRH_CE~ | !{WR} low to RDY/!{BSY} high, chip erase | 7.5 | | 9 | ms |
| t~XLOL~ | XTAL1 low to !{OE} low | 0 | | | ns |
| t~BVDV~ | BS1 valid to DATA valid | 0 | | 250 | ns |
| t~OLDV~ | !{OE} low to DATA valid | | | 250 | ns |
| t~OHDZ~ | !{OE} high to DATA tri-stated | | | 250 | ns |

[1] Write flash, write EEPROM, write fuse bits and write lock bits commands.
```

```wavedrom id=prog-par-general caption="Parallel programming: general timing (DATA row = DATA, XA0/1, BS1, BS2)"
{"signal": [
  {"name": "XTAL1", "wave": "0.10.........", "node": "..AB"},
  {"name": "DATA", "wave": "x=..x=.....x.", "data": ["valid", "valid"], "node": ".C..DE.....F"},
  {"name": "PAGEL", "wave": "0.....10.....", "node": "......GH"},
  {"name": "WR#", "wave": "1........01..", "node": ".........IJ"},
  {"name": "RDY/BSY#", "wave": "1.........0.1", "node": "..........K.L"}
 ],
 "edge": ["C~>A tDVXH", "A<->B tXHXL", "B~>D tXLDX", "E~>G tBVPH", "G<->H tPHPL", "H~>I tPLWL", "B~>I tXLWL", "E~>I tBVWL", "I<->J tWLWH", "I~>F tWLBX", "I~>K tWLRL", "I~>L tWLRH"],
 "config": {"hscale": 3}}
```

```wavedrom id=prog-par-load caption="Parallel programming: loading address low, data low, data high, latch, next address"
{"signal": [
  {"name": "XTAL1", "wave": "010.10.10..10", "node": ".....A.BC..F"},
  {"name": "BS1", "wave": "0.....1...0.."},
  {"name": "PAGEL", "wave": "0........10..", "node": ".........DE"},
  {"name": "DATA", "wave": "=..=..=...=..", "data": ["ADDR0 low", "DATA low", "DATA high", "ADDR1 low"]},
  {"name": "XA0", "wave": "0..1......0.."},
  {"name": "XA1", "wave": "0............"}
 ],
 "edge": ["A~>B tXLXH", "C~>D tXLPH", "E~>F tPLXH"],
 "config": {"hscale": 2}}
```

```wavedrom id=prog-par-read caption="Parallel programming: reading within a page"
{"signal": [
  {"name": "XTAL1", "wave": "0.10.........10.", "node": "...A"},
  {"name": "BS1", "wave": "0.......1...0...", "node": "........D"},
  {"name": "OE#", "wave": "1....0....1.....", "node": ".....B....F"},
  {"name": "DATA", "wave": "x=..x.=..=.z=..x", "data": ["ADDR0 low", "DATA low", "DATA high", "ADDR1 low"], "node": "......C..E.G"},
  {"name": "XA0", "wave": "10..1.......0..1"},
  {"name": "XA1", "wave": "10..1.......0..1"}
 ],
 "edge": ["A~>B tXLOL", "B~>C tOLDV", "D~>E tBVDV", "F~>G tOHDZ"],
 "config": {"hscale": 2}}
```

t~DVXH~, t~XHXL~ and t~XLDX~ also apply to the loading and reading sequences.

#### Serial downloading

Flash and EEPROM can be programmed over the SPI pins while !{RESET} is held low: MOSI (PB3,
serial data in), MISO (PB4, serial data out), SCK (PB5, serial clock). V~CC~ = 1.8 V to 5.5 V;
AV~CC~ within V~CC~ ± 0.3 V and always 1.8 V to 5.5 V. The Programming Enable instruction must
be executed first after !{RESET} goes low. A valid clock must be present according to the
CKSEL fuses (no XTAL1 clock is needed with the internal oscillator). SCK low and high periods
must each be longer than 2 CPU clock cycles for f~ck~ < 12 MHz, 3 for f~ck~ ≥ 12 MHz. Input
data is clocked on the rising edge of SCK, output data on the falling edge.

In serial mode only, EEPROM programming includes an auto-erase cycle, so no chip erase is
needed first. Chip erase turns every flash and EEPROM location into 0xFF.

**Algorithm:**

1. Power-up: apply power with !{RESET} and SCK at 0. If the programmer can't hold SCK low during
   power-up, give !{RESET} a positive pulse of at least two CPU clock cycles after SCK is 0.
2. Wait at least 20 ms, then send Programming Enable on MOSI.
3. Instructions only work in sync: when in sync, the second byte (0x53) echoes back while the
   third byte is sent. Send all four bytes either way; if 0x53 did not echo, pulse !{RESET}
   and send Programming Enable again.
4. Flash is programmed a page at a time: load each byte with Load Program Memory Page (the
   6 LSBs of the address and the data; low byte before high byte for each address), then store
   the page with Write Program Memory Page and the 7 MSBs of the address. Without RDY/!{BSY}
   polling, wait at least t~WD_FLASH~ before the next page; accessing the interface before the
   write completes can corrupt the programming.
5. EEPROM, by byte: Write EEPROM Memory with address and data (the location is erased first);
   without polling, wait t~WD_EEPROM~ before the next byte. Or by page: Load EEPROM Memory Page
   per byte (6 address LSBs and data), then Write EEPROM Memory Page with the 7 address MSBs;
   only the loaded locations change; without polling, wait t~WD_EEPROM~. In a chip-erased
   device, 0xFF need not be programmed.
6. Verify any location with the read instructions (data on MISO).
7. Set !{RESET} high to start normal operation.
8. Power-off (if needed): set !{RESET} to 1, then turn V~CC~ off.

```table id=prog-wd caption="Minimum wait delay before writing the next flash or EEPROM location"
| Symbol | Min | Unit |
|---|--:|---|
| t~WD_FLASH~ | 4.5 | ms |
| t~WD_EEPROM~ | 3.6 | ms |
| t~WD_ERASE~ | 9.0 | ms |
| t~WD_FUSE~ | 4.5 | ms |
```

```table id=prog-serial caption="Serial programming instruction set (hex) [1][2][3][4]"
| Instruction | Byte 1 | Byte 2 | Byte 3 | Byte 4 |
|---|---|---|---|---|
| Programming enable | AC | 53 | 00 | 00 |
| Chip erase (program memory and EEPROM) | AC | 80 | 00 | 00 |
| Poll RDY/!{BSY} [5] | F0 | 00 | 00 | data out |
| Load | < | < | < | < |
| Load extended address byte | 4D | 00 | extended address | 00 |
| Load program memory page, high byte [6][7] | 48 | 00 | address LSB | high data in |
| Load program memory page, low byte [6][7] | 40 | 00 | address LSB | low data in |
| Load EEPROM memory page (page access) | C1 | 00 | 0000 00aa | data in |
| Read | < | < | < | < |
| Read program memory, high byte [6] | 28 | address MSB | address LSB | high data out |
| Read program memory, low byte [6] | 20 | address MSB | address LSB | low data out |
| Read EEPROM memory | A0 | 0000 00aa | aaaa aaaa | data out |
| Read lock bits | 58 | 00 | 00 | data out |
| Read signature byte | 30 | 00 | 0000 00aa | data out |
| Read fuse bits | 50 | 00 | 00 | data out |
| Read fuse high bits | 58 | 08 | 00 | data out |
| Read extended fuse bits | 50 | 08 | 00 | data out |
| Read calibration byte | 38 | 00 | 00 | data out |
| Write | < | < | < | < |
| Write program memory page [6] | 4C | address MSB | address LSB | 00 |
| Write EEPROM memory | C0 | 0000 00aa | aaaa aaaa | data in |
| Write EEPROM memory page (page access) [8] | C2 | 0000 00aa | aaaa aa00 | 00 |
| Write lock bits | AC | E0 | 00 | data in |
| Write fuse bits | AC | A0 | 00 | data in |
| Write fuse high bits | AC | A8 | 00 | data in |
| Write extended fuse bits | AC | A4 | 00 | data in |

[1] Not all instructions apply to all parts. a = address bit.
[2] Bits are programmed 0, unprogrammed 1; keep unused fuse and lock bits unprogrammed (1) for future compatibility.
[3] Fuse and lock bits, calibration and signature bytes and page sizes: see the sections above.
[4] Byte 3 of Load EEPROM memory page and Read signature byte is printed "0000 000aa" in the source (nine digits).
[5] If the LSB of the data out is 1, a programming operation is still pending; wait until it is 0 before the next instruction.
[6] Program memory instructions use a word address, which may be random within the page.
[7] Within a page, load the low data byte before the high data byte. The low address bits are the page offset into the page buffer; the write instruction's address MSB/LSB give the page number.
[8] Load the page buffer first, then write the EEPROM page.
```

```wavedrom id=prog-serial-wave caption="Serial programming waveforms: MOSI sampled on SCK rising edges, MISO changes on falling edges"
{"signal": [
  {"name": "MOSI", "wave": "=.=.|=.=.x", "data": ["MSB", "", "", "LSB"]},
  {"name": "MISO", "wave": "=.=.|=.=.x", "data": ["MSB", "", "", "LSB"]},
  {"name": "SCK", "wave": "0101|0101."}
 ],
 "config": {"hscale": 2}}
```

SPI timing for serial programming: see the SPI timing characteristics.

## Registers and commands

Each register's bits are described in its subsection of
[Peripherals](#peripherals) (or [Programming model](#programming-model)
for SREG and the stack pointer). Addresses: I/O address, data-space
address in parentheses; extended I/O registers (0x60–0xFF) have only a
data-space address. Reserved addresses are omitted.

```table id=reg-summary caption="Register summary [1][2][3][4]"
| Address | Name | Bit 7 | Bit 6 | Bit 5 | Bit 4 | Bit 3 | Bit 2 | Bit 1 | Bit 0 | Access | Reset |
|---|---|---|---|---|---|---|---|---|---|---|---|
| (0xC6) | UDR0 | USART I/O data register | < | < | < | < | < | < | < | R/W | 0x00 |
| (0xC5) | UBRR0H | USART baud rate register high | < | < | < | < | < | < | < | R/W | 0x00 |
| (0xC4) | UBRR0L | USART baud rate register low | < | < | < | < | < | < | < | R/W | 0x00 |
| (0xC2) | UCSR0C | UMSEL01 | UMSEL00 | UPM01 | UPM00 | USBS0 | UCSZ01 / UDORD0 | UCSZ00 / UCPHA0 | UCPOL0 | per bit | per bit |
| (0xC1) | UCSR0B | RXCIE0 | TXCIE0 | UDRIE0 | RXEN0 | TXEN0 | UCSZ02 | RXB80 | TXB80 | per bit | per bit |
| (0xC0) | UCSR0A | RXC0 | TXC0 | UDRE0 | FE0 | DOR0 | UPE0 | U2X0 | MPCM0 | per bit | per bit |
| (0xBD) | TWAMR | TWAM6 | TWAM5 | TWAM4 | TWAM3 | TWAM2 | TWAM1 | TWAM0 | – | R/W | 0x00 |
| (0xBC) | TWCR | TWINT | TWEA | TWSTA | TWSTO | TWWC | TWEN | – | TWIE | per bit | per bit |
| (0xBB) | TWDR | 2-wire serial interface data register | < | < | < | < | < | < | < | R/W | 0xFF |
| (0xBA) | TWAR | TWA6 | TWA5 | TWA4 | TWA3 | TWA2 | TWA1 | TWA0 | TWGCE | per bit | per bit |
| (0xB9) | TWSR | TWS7 | TWS6 | TWS5 | TWS4 | TWS3 | – | TWPS1 | TWPS0 | per bit | per bit |
| (0xB8) | TWBR | 2-wire serial interface bit rate register | < | < | < | < | < | < | < | R/W | 0x00 |
| (0xB6) | ASSR | – | EXCLK | AS2 | TCN2UB | OCR2AUB | OCR2BUB | TCR2AUB | TCR2BUB | per bit | per bit |
| (0xB4) | OCR2B | Timer/Counter2 output compare register B | < | < | < | < | < | < | < | R/W | 0x00 |
| (0xB3) | OCR2A | Timer/Counter2 output compare register A | < | < | < | < | < | < | < | R/W | 0x00 |
| (0xB2) | TCNT2 | Timer/Counter2 (8-bit) | < | < | < | < | < | < | < | R/W | 0x00 |
| (0xB1) | TCCR2B | FOC2A | FOC2B | – | – | WGM22 | CS22 | CS21 | CS20 | per bit | per bit |
| (0xB0) | TCCR2A | COM2A1 | COM2A0 | COM2B1 | COM2B0 | – | – | WGM21 | WGM20 | R/W | 0x00 |
| (0x8B) | OCR1BH | Timer/Counter1 output compare register B high byte | < | < | < | < | < | < | < | R/W | 0x00 |
| (0x8A) | OCR1BL | Timer/Counter1 output compare register B low byte | < | < | < | < | < | < | < | R/W | 0x00 |
| (0x89) | OCR1AH | Timer/Counter1 output compare register A high byte | < | < | < | < | < | < | < | R/W | 0x00 |
| (0x88) | OCR1AL | Timer/Counter1 output compare register A low byte | < | < | < | < | < | < | < | R/W | 0x00 |
| (0x87) | ICR1H | Timer/Counter1 input capture register high byte | < | < | < | < | < | < | < | R/W | 0x00 |
| (0x86) | ICR1L | Timer/Counter1 input capture register low byte | < | < | < | < | < | < | < | R/W | 0x00 |
| (0x85) | TCNT1H | Timer/Counter1 counter register high byte | < | < | < | < | < | < | < | R/W | 0x00 |
| (0x84) | TCNT1L | Timer/Counter1 counter register low byte | < | < | < | < | < | < | < | R/W | 0x00 |
| (0x82) | TCCR1C | FOC1A | FOC1B | – | – | – | – | – | – | R/W | 0x00 |
| (0x81) | TCCR1B | ICNC1 | ICES1 | – | WGM13 | WGM12 | CS12 | CS11 | CS10 | R/W | 0x00 |
| (0x80) | TCCR1A | COM1A1 | COM1A0 | COM1B1 | COM1B0 | – | – | WGM11 | WGM10 | R/W | 0x00 |
| (0x7F) | DIDR1 | – | – | – | – | – | – | AIN1D | AIN0D | R/W | 0x00 |
| (0x7E) | DIDR0 | – | – | ADC5D | ADC4D | ADC3D | ADC2D | ADC1D | ADC0D | R/W | 0x00 |
| (0x7C) | ADMUX | REFS1 | REFS0 | ADLAR | – | MUX3 | MUX2 | MUX1 | MUX0 | per bit | per bit |
| (0x7B) | ADCSRB | – | ACME | – | – | – | ADTS2 | ADTS1 | ADTS0 | per bit | per bit |
| (0x7A) | ADCSRA | ADEN | ADSC | ADATE | ADIF | ADIE | ADPS2 | ADPS1 | ADPS0 | per bit | per bit |
| (0x79) | ADCH | ADC data register high byte | < | < | < | < | < | < | < | R | 0x00 |
| (0x78) | ADCL | ADC data register low byte | < | < | < | < | < | < | < | R | 0x00 |
| (0x70) | TIMSK2 | – | – | – | – | – | OCIE2B | OCIE2A | TOIE2 | R/W | 0x00 |
| (0x6F) | TIMSK1 | – | – | ICIE1 | – | – | OCIE1B | OCIE1A | TOIE1 | R/W | 0x00 |
| (0x6E) | TIMSK0 | – | – | – | – | – | OCIE0B | OCIE0A | TOIE0 | R/W | 0x00 |
| (0x6D) | PCMSK2 | PCINT23 | PCINT22 | PCINT21 | PCINT20 | PCINT19 | PCINT18 | PCINT17 | PCINT16 | R/W | 0x00 |
| (0x6C) | PCMSK1 | – | PCINT14 | PCINT13 | PCINT12 | PCINT11 | PCINT10 | PCINT9 | PCINT8 | R/W | 0x00 |
| (0x6B) | PCMSK0 | PCINT7 | PCINT6 | PCINT5 | PCINT4 | PCINT3 | PCINT2 | PCINT1 | PCINT0 | R/W | 0x00 |
| (0x69) | EICRA | – | – | – | – | ISC11 | ISC10 | ISC01 | ISC00 | R/W | 0x00 |
| (0x68) | PCICR | – | – | – | – | – | PCIE2 | PCIE1 | PCIE0 | R/W | 0x00 |
| (0x66) | OSCCAL | Oscillator calibration register | < | < | < | < | < | < | < | R/W | device specific |
| (0x64) | PRR | PRTWI | PRTIM2 | PRTIM0 | – | PRTIM1 | PRSPI | PRUSART0 | PRADC | R/W | 0x00 |
| (0x61) | CLKPR | CLKPCE | – | – | – | CLKPS3 | CLKPS2 | CLKPS1 | CLKPS0 | per bit | per bit |
| (0x60) | WDTCSR | WDIF | WDIE | WDP3 | WDCE | WDE | WDP2 | WDP1 | WDP0 | per bit | per bit |
| 0x3F (0x5F) | SREG | I | T | H | S | V | N | Z | C | R/W | 0x00 |
| 0x3E (0x5E) | SPH | – | – | – | – | – | SP10 [5] | SP9 | SP8 | R/W | RAMEND |
| 0x3D (0x5D) | SPL | SP7 | SP6 | SP5 | SP4 | SP3 | SP2 | SP1 | SP0 | R/W | RAMEND |
| 0x37 (0x57) | SPMCSR | SPMIE | RWWSB [5] | SIGRD | RWWSRE [5] | BLBSET | PGWRT | PGERS | SPMEN | per bit | per bit |
| 0x35 (0x55) | MCUCR | – | BODS [6] | BODSE [6] | PUD | – | – | IVSEL | IVCE | R/W | 0x00 |
| 0x34 (0x54) | MCUSR | – | – | – | – | WDRF | BORF | EXTRF | PORF | R/W | see text |
| 0x33 (0x53) | SMCR | – | – | – | – | SM2 | SM1 | SM0 | SE | per bit | per bit |
| 0x30 (0x50) | ACSR | ACD | ACBG | ACO | ACI | ACIE | ACIC | ACIS1 | ACIS0 | per bit | per bit |
| 0x2E (0x4E) | SPDR | SPI data register | < | < | < | < | < | < | < | R/W | X |
| 0x2D (0x4D) | SPSR | SPIF | WCOL | – | – | – | – | – | SPI2X | per bit | per bit |
| 0x2C (0x4C) | SPCR | SPIE | SPE | DORD | MSTR | CPOL | CPHA | SPR1 | SPR0 | R/W | 0x00 |
| 0x2B (0x4B) | GPIOR2 | General purpose I/O register 2 | < | < | < | < | < | < | < | R/W | 0x00 |
| 0x2A (0x4A) | GPIOR1 | General purpose I/O register 1 | < | < | < | < | < | < | < | R/W | 0x00 |
| 0x28 (0x48) | OCR0B | Timer/Counter0 output compare register B | < | < | < | < | < | < | < | R/W | 0x00 |
| 0x27 (0x47) | OCR0A | Timer/Counter0 output compare register A | < | < | < | < | < | < | < | R/W | 0x00 |
| 0x26 (0x46) | TCNT0 | Timer/Counter0 (8-bit) | < | < | < | < | < | < | < | R/W | 0x00 |
| 0x25 (0x45) | TCCR0B | FOC0A | FOC0B | – | – | WGM02 | CS02 | CS01 | CS00 | per bit | per bit |
| 0x24 (0x44) | TCCR0A | COM0A1 | COM0A0 | COM0B1 | COM0B0 | – | – | WGM01 | WGM00 | R/W | 0x00 |
| 0x23 (0x43) | GTCCR | TSM | – | – | – | – | – | PSRASY | PSRSYNC | R/W | 0x00 |
| 0x22 (0x42) | EEARH | EEPROM address register high byte [5] | < | < | < | < | < | < | < | R/W | X |
| 0x21 (0x41) | EEARL | EEPROM address register low byte | < | < | < | < | < | < | < | R/W | X |
| 0x20 (0x40) | EEDR | EEPROM data register | < | < | < | < | < | < | < | R/W | 0x00 |
| 0x1F (0x3F) | EECR | – | – | EEPM1 | EEPM0 | EERIE | EEMPE | EEPE | EERE | per bit | per bit |
| 0x1E (0x3E) | GPIOR0 | General purpose I/O register 0 | < | < | < | < | < | < | < | R/W | 0x00 |
| 0x1D (0x3D) | EIMSK | – | – | – | – | – | – | INT1 | INT0 | R/W | 0x00 |
| 0x1C (0x3C) | EIFR | – | – | – | – | – | – | INTF1 | INTF0 | R/W | 0x00 |
| 0x1B (0x3B) | PCIFR | – | – | – | – | – | PCIF2 | PCIF1 | PCIF0 | R/W | 0x00 |
| 0x17 (0x37) | TIFR2 | – | – | – | – | – | OCF2B | OCF2A | TOV2 | R/W | 0x00 |
| 0x16 (0x36) | TIFR1 | – | – | ICF1 | – | – | OCF1B | OCF1A | TOV1 | R/W | 0x00 |
| 0x15 (0x35) | TIFR0 | – | – | – | – | – | OCF0B | OCF0A | TOV0 | R/W | 0x00 |
| 0x0B (0x2B) | PORTD | PORTD7 | PORTD6 | PORTD5 | PORTD4 | PORTD3 | PORTD2 | PORTD1 | PORTD0 | R/W | 0x00 |
| 0x0A (0x2A) | DDRD | DDD7 | DDD6 | DDD5 | DDD4 | DDD3 | DDD2 | DDD1 | DDD0 | R/W | 0x00 |
| 0x09 (0x29) | PIND | PIND7 | PIND6 | PIND5 | PIND4 | PIND3 | PIND2 | PIND1 | PIND0 | R/W | N/A |
| 0x08 (0x28) | PORTC | – | PORTC6 | PORTC5 | PORTC4 | PORTC3 | PORTC2 | PORTC1 | PORTC0 | R/W | 0x00 |
| 0x07 (0x27) | DDRC | – | DDC6 | DDC5 | DDC4 | DDC3 | DDC2 | DDC1 | DDC0 | R/W | 0x00 |
| 0x06 (0x26) | PINC | – | PINC6 | PINC5 | PINC4 | PINC3 | PINC2 | PINC1 | PINC0 | R/W | N/A |
| 0x05 (0x25) | PORTB | PORTB7 | PORTB6 | PORTB5 | PORTB4 | PORTB3 | PORTB2 | PORTB1 | PORTB0 | R/W | 0x00 |
| 0x04 (0x24) | DDRB | DDB7 | DDB6 | DDB5 | DDB4 | DDB3 | DDB2 | DDB1 | DDB0 | R/W | 0x00 |
| 0x03 (0x23) | PINB | PINB7 | PINB6 | PINB5 | PINB4 | PINB3 | PINB2 | PINB1 | PINB0 | R/W | N/A |

[1] For compatibility with future devices, write reserved bits to 0 if accessed. Never write reserved I/O addresses.
[2] I/O registers 0x00–0x1F are bit-accessible with SBI and CBI, and their bits can be tested with SBIS and SBIC.
[3] Some status flags are cleared by writing 1 to them. Unlike most other AVRs, CBI and SBI operate only on the specified bit, so they can be used on registers holding such flags (0x00–0x1F only).
[4] IN and OUT use I/O addresses 0x00–0x3F; LD and ST address them as data space, 0x20 higher. Extended I/O (0x60–0xFF) is reachable only with ST/STS/STD and LD/LDS/LDD.
[5] Only in ATmega88A/88PA/168A/168PA/328/328P.
[6] BODS and BODSE only in the picoPower devices ATmega48PA/88PA/168PA/328P.
```

## Absolute maximum ratings

```table id=abs-max caption="Absolute maximum ratings [1]"
| Parameter | Conditions | Min | Max | Unit |
|---|---|--:|--:|---|
| Operating temperature | | -55 | 125 | °C |
| Storage temperature | | -65 | 150 | °C |
| Voltage on any pin except !{RESET} | With respect to ground | -0.5 | V~CC~ + 0.5 | V |
| Voltage on !{RESET} | With respect to ground | -0.5 | 13.0 | V |
| Maximum operating voltage | | | 6.0 | V |
| DC current per I/O pin | | | 40.0 | mA |
| DC current, V~CC~ and GND pins | | | 200.0 | mA |

[1] Same for the -40 °C to 85 °C and -40 °C to 105 °C devices. Stresses beyond these may cause permanent damage; functional operation at these or other conditions beyond the operating sections is not implied, and long exposure may affect reliability.
```

## Recommended operating conditions

```table id=roc caption="Operating conditions"
| Symbol | Parameter | Conditions | Min | Max | Unit |
|---|---|---|--:|--:|---|
| V~CC~ | Supply voltage | All devices | 1.8 | 5.5 | V |
| T~A~ | Operating temperature | Industrial, all devices (ordering codes -xU, -MMH) | -40 | 85 | °C |
| ^ | ^ | Industrial, ATmega48PA/88PA/168PA/328P only (ordering codes -xN, -MMN) | -40 | 105 | °C |
```

Maximum clock frequency depends on V~CC~: linear between 1.8 V and
2.7 V, and between 2.7 V and 4.5 V. Below the line is the safe
operating area.

```table id=roc-speed caption="Speed grades: maximum frequency vs V~CC~"
| V~CC~ | Max | Unit |
|---|--:|---|
| 1.8 V | 4 | MHz |
| 2.7 V | 10 | MHz |
| 4.5 V to 5.5 V | 20 | MHz |
```

## Electrical characteristics

```table id=dc caption="Common DC characteristics" conditions="T~A~ = -40 °C to 105 °C (-40 °C to 85 °C for devices so rated), V~CC~ = 1.8 V to 5.5 V, unless noted"
| Symbol | Parameter | Conditions | Min | Typ | Max | Unit |
|---|---|---|--:|--:|--:|---|
| V~IL~ | Input low voltage, except XTAL1 and !{RESET} [1] | V~CC~ = 1.8 V to 2.4 V | -0.5 | | 0.2 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | -0.5 | | 0.3 × V~CC~ | V |
| V~IH~ | Input high voltage, except XTAL1 and !{RESET} [2] | V~CC~ = 1.8 V to 2.4 V | 0.7 × V~CC~ | | V~CC~ + 0.5 | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | 0.6 × V~CC~ | | V~CC~ + 0.5 | V |
| V~IL1~ | Input low voltage, XTAL1 [1] | V~CC~ = 1.8 V to 5.5 V | -0.5 | | 0.1 × V~CC~ | V |
| V~IH1~ | Input high voltage, XTAL1 [2] | V~CC~ = 1.8 V to 2.4 V | 0.8 × V~CC~ | | V~CC~ + 0.5 | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | 0.7 × V~CC~ | | V~CC~ + 0.5 | V |
| V~IL2~ | Input low voltage, !{RESET} [1] | V~CC~ = 1.8 V to 5.5 V | -0.5 | | 0.1 × V~CC~ | V |
| V~IH2~ | Input high voltage, !{RESET} [2] | V~CC~ = 1.8 V to 5.5 V | 0.9 × V~CC~ | | V~CC~ + 0.5 | V |
| V~IL3~ | Input low voltage, !{RESET} as I/O [1] | V~CC~ = 1.8 V to 2.4 V | -0.5 | | 0.2 × V~CC~ | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | -0.5 | | 0.3 × V~CC~ | V |
| V~IH3~ | Input high voltage, !{RESET} as I/O [2] | V~CC~ = 1.8 V to 2.4 V | 0.7 × V~CC~ | | V~CC~ + 0.5 | V |
| ^ | ^ | V~CC~ = 2.4 V to 5.5 V | 0.6 × V~CC~ | | V~CC~ + 0.5 | V |
| V~OL~ | Output low voltage, except !{RESET} [4] | I~OL~ = 20 mA, V~CC~ = 5 V, T~A~ = 85 °C | | | 0.9 | V |
| ^ | ^ | I~OL~ = 20 mA, V~CC~ = 5 V, T~A~ = 105 °C | | | 1.0 | V |
| ^ | ^ | I~OL~ = 10 mA, V~CC~ = 3 V, T~A~ = 85 °C | | | 0.6 | V |
| ^ | ^ | I~OL~ = 10 mA, V~CC~ = 3 V, T~A~ = 105 °C | | | 0.7 | V |
| V~OH~ | Output high voltage, except !{RESET} [3] | I~OH~ = -20 mA, V~CC~ = 5 V, T~A~ = 85 °C | 4.2 | | | V |
| ^ | ^ | I~OH~ = -20 mA, V~CC~ = 5 V, T~A~ = 105 °C | 4.1 | | | V |
| ^ | ^ | I~OH~ = -10 mA, V~CC~ = 3 V, T~A~ = 85 °C | 2.3 | | | V |
| ^ | ^ | I~OH~ = -10 mA, V~CC~ = 3 V, T~A~ = 105 °C | 2.1 | | | V |
| I~IL~ | Input leakage current, I/O pin, pin low (absolute value) | V~CC~ = 5.5 V | | | 1 | µA |
| I~IH~ | Input leakage current, I/O pin, pin high (absolute value) | V~CC~ = 5.5 V | | | 1 | µA |
| R~RST~ | !{RESET} pull-up resistor | | 30 | | 60 | kΩ |
| R~PU~ | I/O pin pull-up resistor | | 20 | | 50 | kΩ |
| V~ACIO~ | Analog comparator input offset voltage | V~CC~ = 5 V, V~in~ = V~CC~/2 [6] | | | 40 | mV |
| ^ | ^ | V~CC~ < 3.6 V, V~in~ < 0.5 V [7] | | | 60 [5] | mV |
| ^ | ^ | V~CC~ > 3.6 V, V~in~ < 0.5 V [7] | | | 500 [5] | mV |
| I~ACLK~ | Analog comparator input leakage current | V~CC~ = 5 V, V~in~ = V~CC~/2 | -50 | | 50 | nA |
| t~ACID~ | Analog comparator propagation delay | V~CC~ = 2.7 V | | 750 | | ns |
| ^ | ^ | V~CC~ = 4.0 V | | 500 | | ns |

[1] Max is the highest voltage at which the pin is guaranteed to read low.
[2] Min is the lowest voltage at which the pin is guaranteed to read high.
[3] Each port can source more than the test conditions (20 mA at V~CC~ = 5 V, 10 mA at 3 V) in steady state, but: the sum of all I~OH~ for C0–C5, D0–D4, ADC7 and !{RESET} must not exceed 150 mA; the sum for B0–B5, D5–D7, ADC6, XTAL1 and XTAL2 must not exceed 150 mA. Above the test condition V~OH~ may exceed the specification; pins are not guaranteed to source more than the test current.
[4] Each port can sink more than the test conditions (20 mA at V~CC~ = 5 V, 10 mA at 3 V) in steady state, but: the sum of all I~OL~ for C0–C5, ADC7 and ADC6 must not exceed 100 mA; for B0–B5, D5–D7, XTAL1 and XTAL2 100 mA; for D0–D4 and !{RESET} 100 mA. Above the test condition V~OL~ may exceed the specification; pins are not guaranteed to sink more than the test current.
[5] Based on characterization; not tested in production.
[6] Typically below 10 mV.
[7] Typically below 15 mV.
```

```table id=dc-icc85 caption="Supply current, T~A~ = -40 °C to 85 °C, V~CC~ = 1.8 V to 5.5 V [1][2][3]"
| Parameter | Conditions | ATmega48A | < | ATmega48PA | < | ATmega88A | < | ATmega88PA | < | ATmega168A | < | ATmega168PA | < | ATmega328 | < | ATmega328P | < | Unit |
| ^ | ^ | Typ | Max | Typ | Max | Typ | Max | Typ | Max | Typ | Max | Typ | Max | Typ | Max | Typ | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|--:|---|
| Active | 1 MHz, V~CC~ = 2 V | 0.2 | 0.55 | 0.2 | 0.5 | 0.2 | 0.55 | 0.2 | 0.5 | 0.2 | 0.55 | 0.2 | 0.5 | 0.3 | 0.55 | 0.3 | 0.5 | mA |
| ^ | 4 MHz, V~CC~ = 3 V | 1.2 | 3.5 | 1.2 | 2.5 | 1.2 | 3.5 | 1.2 | 2.5 | 1.2 | 3.5 | 1.2 | 2.5 | 1.7 | 3.5 | 1.7 | 2.5 | mA |
| ^ | 8 MHz, V~CC~ = 5 V | 4.0 | 12 | 4.0 | 9 | 4.1 | 12 | 4.1 | 9 | 4.2 | 12 | 4.2 | 9 | 5.2 | 12 | 5.2 | 9 | mA |
| Idle | 1 MHz, V~CC~ = 2 V | 0.03 | 0.5 | 0.03 | 0.15 | 0.03 | 0.5 | 0.03 | 0.15 | 0.03 | 0.5 | 0.03 | 0.15 | 0.04 | 0.5 | 0.04 | 0.15 | mA |
| ^ | 4 MHz, V~CC~ = 3 V | 0.21 | 1.5 | 0.21 | 0.7 | 0.18 | 1.5 | 0.18 | 0.7 | 0.2 | 1.5 | 0.2 | 0.7 | 0.3 | 1.5 | 0.3 | 0.7 | mA |
| ^ | 8 MHz, V~CC~ = 5 V | 0.9 | 5.5 | 0.9 | 2.7 | 0.8 | 5.5 | 0.8 | 2.7 | 0.9 | 5.5 | 0.9 | 2.7 | 1.2 | 5.5 | 1.2 | 2.7 | mA |
| Power-save | 32 kHz TOSC enabled, V~CC~ = 1.8 V | 0.75 | | 0.75 | | 0.8 | | 0.8 | | 0.75 | | 0.75 | | 0.8 | | 0.8 | | µA |
| ^ | 32 kHz TOSC enabled, V~CC~ = 3 V | 0.9 | | 0.9 | | 0.9 | | 0.9 | | 0.83 | | 0.83 | | 0.9 | | 0.9 | | µA |
| Power-down | WDT enabled, V~CC~ = 3 V | 3.9 | 15 | 3.9 | 8 | 3.9 | 15 | 3.9 | 8 | 4.1 | 15 | 4.1 | 8 | 4.2 | 15 | 4.2 | 8 | µA |
| ^ | WDT disabled, V~CC~ = 3 V | 0.1 | 2 | 0.1 | 2 | 0.1 | 2 | 0.1 | 2 | 0.1 | 2 | 0.1 | 2 | 0.1 | 2 | 0.1 | 2 | µA |

[1] With all modules disabled through the power reduction register (PRR = 0xFF), as in "minimizing power consumption".
[2] Typical values at 25 °C. Maximum values are production test limits (stated for every device except the ATmega48A).
[3] Power-save and power-down values include input leakage current.
```

```table id=dc-icc105 caption="Supply current, T~A~ = -40 °C to 105 °C devices, V~CC~ = 1.8 V to 5.5 V [1][2][3]"
| Parameter | Conditions | ATmega48PA | < | ATmega88PA | < | ATmega168PA | < | ATmega328P | < | Unit |
| ^ | ^ | Typ | Max | Typ | Max | Typ | Max | Typ | Max | ^ |
|---|---|--:|--:|--:|--:|--:|--:|--:|--:|---|
| Active | 1 MHz, V~CC~ = 2 V | 0.22 | 0.55 | 0.2 | 0.6 | 0.3 | 0.6 | 0.3 | 0.5 | mA |
| ^ | 4 MHz, V~CC~ = 3 V | 1.15 | 2.65 | 1.2 | 2.75 | 1.8 | 2.75 | 1.7 | 2.5 | mA |
| ^ | 8 MHz, V~CC~ = 5 V | 4.1 | 9.5 | 4.1 | 10 | 6.7 | 10 | 5.2 | 9.0 | mA |
| Idle | 1 MHz, V~CC~ = 2 V | 0.024 | 0.16 | 0.03 | 0.17 | 0.06 | 0.2 | 0.04 | 0.15 | mA |
| ^ | 4 MHz, V~CC~ = 3 V | 0.2 | 0.75 | 0.18 | 0.8 | 0.4 | 0.8 | 0.3 | 0.7 | mA |
| ^ | 8 MHz, V~CC~ = 5 V | 0.78 | 2.8 | 0.8 | 3 | 1.7 | 3 | 1.2 | 2.7 | mA |
| Power-save | 32 kHz TOSC enabled, V~CC~ = 1.8 V | 0.75 | | 0.8 | | 0.8 | | 0.8 | | µA |
| ^ | 32 kHz TOSC enabled, V~CC~ = 3 V | 0.9 | | 0.9 | | 0.9 | | 0.9 | | µA |
| Power-down | WDT enabled, V~CC~ = 3 V | 3.9 | 10 | 3.9 | 10 | 4.6 | 10 | 4.2 | 10 | µA |
| ^ | WDT disabled, V~CC~ = 3 V | 0.1 | 5 | 0.1 | 5 | 0.1 | 5 | 0.1 | 5 | µA |

[1] PRR = 0xFF.
[2] Typical values at 25 °C. Maximum values are production test limits.
[3] Power-save and power-down values include input leakage current.
```

## Switching characteristics

Timing is given with the hardware it belongs to, in
[Peripherals](#peripherals): clock characteristics (calibrated RC
oscillator accuracy, external clock drive) under [clock system](#clock-system);
reset, brown-out and internal voltage reference characteristics under
[reset and watchdog](#reset-and-watchdog); [SPI](#spi-timing),
[two-wire serial interface](#2-wire-serial-interface-twi) and [ADC](#adc)
characteristics with those peripherals; parallel programming
characteristics under [memory programming](#prog-par-char).

## Typical characteristics

ATmega328P curves (source section 31.8), typical and not tested in
production; the other seven devices' curves are dropped. Current is
measured with all I/O pins inputs with pull-ups on, a rail-to-rail
square-wave clock, all PRR bits set and the analog comparator off. The
power-down current doesn't depend on the clock selection; power-down
with minus without the watchdog is the watchdog's current. A pin
driving a load C~L~ adds about C~L~ × V~CC~ × f. Parts are
characterized above their tested frequency but are only guaranteed up
to the frequency of their ordering code.

```curve id=tc-active caption="Active supply current vs frequency, 1–20 MHz"
{"x": {"label": "f", "unit": "MHz", "range": [0, 20]},
 "y": {"label": "ICC", "unit": "mA", "range": [0, 14]},
 "series": [
   {"label": "5.5 V", "points": [[1, 1.1], [2, 1.9], [4, 3.6], [5, 4.3], [8, 6.2], [10, 7.4], [12, 8.6], [14, 9.9], [16, 11.1], [18, 12.4], [20, 13.6]]},
   {"label": "5.0 V", "points": [[1, 0.95], [2, 1.7], [4, 3.2], [5, 3.8], [8, 5.4], [10, 6.4], [12, 7.5], [14, 8.5], [16, 9.6], [18, 10.7], [20, 11.8]]},
   {"label": "4.5 V", "points": [[1, 0.8], [2, 1.5], [4, 2.8], [5, 3.3], [8, 4.6], [10, 5.6], [12, 6.5], [14, 7.4], [16, 8.3], [18, 9.2], [20, 10.1]]},
   {"label": "4.0 V", "points": [[1, 0.7], [2, 1.3], [4, 2.4], [5, 2.8], [8, 3.9], [10, 4.6], [12, 5.4], [14, 6.1], [16.7, 7.2]]},
   {"label": "3.3 V", "points": [[1, 0.5], [2, 0.9], [4, 1.7], [5, 2.1], [8, 3.0], [10, 3.6], [12.5, 4.5]]},
   {"label": "2.7 V", "points": [[1, 0.4], [2, 0.7], [4, 1.3], [5, 1.6], [8, 2.1], [10, 2.5]]},
   {"label": "1.8 V", "points": [[1, 0.25], [2, 0.45], [4, 0.8]]}
 ]}
```

```curve id=tc-idle caption="Idle supply current vs frequency, 1–20 MHz"
{"x": {"label": "f", "unit": "MHz", "range": [0, 20]},
 "y": {"label": "ICC", "unit": "mA", "range": [0, 3.5]},
 "series": [
   {"label": "5.5 V", "points": [[1, 0.19], [5, 0.88], [10, 1.75], [15, 2.63], [20, 3.5]]},
   {"label": "5.0 V", "points": [[1, 0.16], [5, 0.76], [10, 1.51], [15, 2.26], [20, 3.01]]},
   {"label": "4.5 V", "points": [[1, 0.14], [5, 0.66], [10, 1.3], [15, 1.95], [20, 2.59]]},
   {"label": "4.0 V", "points": [[1, 0.12], [5, 0.54], [10, 1.1], [16.7, 1.85]]},
   {"label": "3.6 V", "points": [[1, 0.1], [5, 0.46], [10, 0.92], [14.4, 1.33]]},
   {"label": "2.7 V", "points": [[1, 0.08], [5, 0.33], [10, 0.64]]},
   {"label": "1.8 V", "points": [[1, 0.05], [4, 0.16]]}
 ]}
```

```curve id=tc-pd caption="Power-down supply current vs V~CC~, watchdog disabled"
{"x": {"label": "VCC", "unit": "V", "range": [1.5, 5.5]},
 "y": {"label": "ICC", "unit": "µA", "range": [0, 3]},
 "series": [
   {"label": "105 °C", "points": [[1.8, 1.22], [2.5, 1.43], [3, 1.55], [3.5, 1.65], [4, 1.87], [4.5, 2.06], [5, 2.34], [5.5, 2.7]]},
   {"label": "85 °C", "points": [[1.8, 0.45], [2.5, 0.49], [3, 0.55], [3.5, 0.64], [4, 0.72], [4.5, 0.69], [5, 0.84], [5.5, 1.06]]},
   {"label": "25 °C", "points": [[1.8, 0.09], [3, 0.11], [4, 0.12], [5, 0.14], [5.5, 0.12]]},
   {"label": "-40 °C", "points": [[1.8, 0.03], [3, 0.06], [4, 0.08], [5, 0.08], [5.5, 0.05]]}
 ]}
```

```curve id=tc-pd-wdt caption="Power-down supply current vs V~CC~, watchdog enabled"
{"x": {"label": "VCC", "unit": "V", "range": [1.5, 5.5]},
 "y": {"label": "ICC", "unit": "µA", "range": [2, 10]},
 "series": [
   {"label": "105 °C", "points": [[1.8, 3.7], [2.5, 4.2], [3, 4.7], [3.5, 5.2], [4, 5.9], [4.5, 6.9], [5, 8.1], [5.5, 9.6]]},
   {"label": "-40 °C", "points": [[1.8, 4.4], [2, 4.8], [2.5, 5.15], [3, 5.4], [3.5, 5.75], [4, 6.2], [4.5, 6.75], [5, 7.45], [5.5, 8.5]]},
   {"label": "85 °C", "points": [[1.8, 3.75], [2.5, 4.0], [3, 4.3], [3.5, 4.75], [4, 5.2], [4.5, 5.8], [5, 6.6], [5.5, 8.0]]},
   {"label": "25 °C", "points": [[1.8, 3.25], [2.5, 3.65], [3, 4.0], [3.5, 4.45], [4, 5.0], [4.5, 5.8], [5, 6.5], [5.5, 7.6]]}
 ]}
```

```curve id=tc-sink3 caption="I/O pin output voltage vs sink current, V~CC~ = 3 V"
{"x": {"label": "IOL", "unit": "mA", "range": [0, 20]},
 "y": {"label": "VOL", "unit": "V", "range": [0, 1]},
 "series": [
   {"label": "105 °C", "points": [[0, 0], [5, 0.2], [10, 0.42], [15, 0.66], [20, 0.96]]},
   {"label": "85 °C", "points": [[0, 0], [5, 0.19], [10, 0.4], [15, 0.62], [20, 0.88]]},
   {"label": "25 °C", "points": [[0, 0], [5, 0.16], [10, 0.32], [15, 0.5], [20, 0.69]]},
   {"label": "-40 °C", "points": [[0, 0], [5, 0.12], [10, 0.26], [15, 0.39], [20, 0.54]]}
 ]}
```

```curve id=tc-sink5 caption="I/O pin output voltage vs sink current, V~CC~ = 5 V"
{"x": {"label": "IOL", "unit": "mA", "range": [0, 20]},
 "y": {"label": "VOL", "unit": "V", "range": [0, 0.7]},
 "series": [
   {"label": "105 °C", "points": [[0, 0], [5, 0.145], [10, 0.3], [15, 0.44], [20, 0.59]]},
   {"label": "85 °C", "points": [[0, 0], [5, 0.14], [10, 0.28], [15, 0.42], [20, 0.56]]},
   {"label": "25 °C", "points": [[0, 0], [5, 0.12], [10, 0.24], [15, 0.35], [20, 0.47]]},
   {"label": "-40 °C", "points": [[0, 0], [5, 0.1], [10, 0.2], [15, 0.3], [20, 0.39]]}
 ]}
```

```curve id=tc-src3 caption="I/O pin output voltage vs source current, V~CC~ = 3 V"
{"x": {"label": "IOH", "unit": "mA", "range": [0, 20]},
 "y": {"label": "VOH", "unit": "V", "range": [1.9, 3.1]},
 "series": [
   {"label": "-40 °C", "points": [[0, 3.0], [5, 2.86], [10, 2.7], [15, 2.53], [20, 2.34]]},
   {"label": "25 °C", "points": [[0, 3.0], [5, 2.83], [10, 2.63], [15, 2.42], [20, 2.17]]},
   {"label": "85 °C", "points": [[0, 3.0], [5, 2.8], [10, 2.57], [15, 2.29], [20, 1.97]]},
   {"label": "105 °C", "points": [[0, 3.0], [5, 2.79], [10, 2.55], [15, 2.27], [20, 1.93]]}
 ]}
```

```curve id=tc-src5 caption="I/O pin output voltage vs source current, V~CC~ = 5 V"
{"x": {"label": "IOH", "unit": "mA", "range": [0, 20]},
 "y": {"label": "VOH", "unit": "V", "range": [4.3, 5.1]},
 "series": [
   {"label": "-40 °C", "points": [[0, 5.0], [5, 4.89], [10, 4.78], [15, 4.67], [20, 4.56]]},
   {"label": "25 °C", "points": [[0, 5.0], [5, 4.87], [10, 4.74], [15, 4.61], [20, 4.47]]},
   {"label": "85 °C", "points": [[0, 5.0], [5, 4.85], [10, 4.7], [15, 4.55], [20, 4.4]]},
   {"label": "105 °C", "points": [[0, 5.0], [5, 4.84], [10, 4.69], [15, 4.53], [20, 4.38]]}
 ]}
```

```curve id=tc-rc-vcc caption="Calibrated 8 MHz RC oscillator frequency vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [2, 5.5]},
 "y": {"label": "fRC", "unit": "MHz", "range": [7.4, 8.6]},
 "series": [
   {"label": "105 °C", "points": [[2, 8.17], [2.5, 8.23], [3, 8.27], [3.5, 8.31], [4, 8.33], [4.5, 8.35], [5, 8.37], [5.5, 8.4]]},
   {"label": "85 °C", "points": [[2, 8.09], [2.5, 8.16], [3, 8.2], [3.5, 8.24], [4, 8.26], [4.5, 8.28], [5, 8.3], [5.5, 8.33]]},
   {"label": "25 °C", "points": [[2, 7.86], [2.5, 7.94], [3, 7.99], [3.5, 8.03], [4, 8.06], [4.5, 8.08], [5, 8.09], [5.5, 8.12]]},
   {"label": "-40 °C", "points": [[2, 7.54], [2.5, 7.62], [3, 7.68], [3.5, 7.72], [4, 7.76], [4.5, 7.77], [5, 7.79], [5.5, 7.81]]}
 ]}
```

```curve id=tc-rc-temp caption="Calibrated 8 MHz RC oscillator frequency vs temperature"
{"x": {"label": "T", "unit": "°C", "range": [-40, 110]},
 "y": {"label": "fRC", "unit": "MHz", "range": [7.5, 8.4]},
 "series": [
   {"label": "5.5 V", "points": [[-40, 7.86], [-20, 7.94], [0, 8.03], [20, 8.12], [40, 8.19], [60, 8.23], [80, 8.26], [105, 8.3]]},
   {"label": "5.0 V", "points": [[-40, 7.85], [0, 8.02], [20, 8.1], [40, 8.17], [60, 8.21], [80, 8.24], [105, 8.29]]},
   {"label": "4.5 V", "points": [[-40, 7.84], [0, 8.0], [20, 8.09], [40, 8.15], [60, 8.2], [80, 8.23], [105, 8.27]]},
   {"label": "4.0 V", "points": [[-40, 7.83], [0, 7.99], [20, 8.07], [40, 8.14], [60, 8.18], [80, 8.22], [105, 8.26]]},
   {"label": "3.6 V", "points": [[-40, 7.81], [0, 7.97], [20, 8.05], [40, 8.12], [60, 8.17], [80, 8.21], [105, 8.24]]},
   {"label": "2.7 V", "points": [[-40, 7.73], [-20, 7.81], [0, 7.89], [20, 7.96], [40, 8.03], [60, 8.08], [80, 8.13], [105, 8.17]]},
   {"label": "1.8 V", "points": [[-40, 7.5], [-20, 7.59], [0, 7.68], [20, 7.76], [40, 7.84], [60, 7.91], [80, 7.97], [105, 8.04]]}
 ]}
```

```curve id=tc-bod caption="BOD thresholds vs temperature, BODLEVEL 1.8 V, 2.7 V and 4.3 V"
{"x": {"label": "T", "unit": "°C", "range": [-40, 110]},
 "y": {"label": "Threshold", "unit": "V", "range": [1.7, 4.4]},
 "series": [
   {"label": "4.3 V, rising VCC", "points": [[-40, 4.362], [0, 4.356], [25, 4.351], [50, 4.345], [85, 4.334], [105, 4.324]]},
   {"label": "4.3 V, falling VCC", "points": [[-40, 4.285], [0, 4.279], [25, 4.273], [50, 4.267], [85, 4.258], [105, 4.249]]},
   {"label": "2.7 V, rising VCC", "points": [[-40, 2.769], [0, 2.765], [25, 2.761], [50, 2.757], [85, 2.75], [105, 2.742]]},
   {"label": "2.7 V, falling VCC", "points": [[-40, 2.694], [0, 2.69], [25, 2.688], [50, 2.684], [85, 2.679], [105, 2.673]]},
   {"label": "1.8 V, rising VCC", "points": [[-40, 1.834], [0, 1.831], [25, 1.829], [50, 1.826], [85, 1.821], [105, 1.8175]]},
   {"label": "1.8 V, falling VCC", "points": [[-40, 1.801], [0, 1.799], [25, 1.797], [50, 1.796], [85, 1.793], [105, 1.789]]}
 ]}
```

```curve id=tc-bandgap caption="Calibrated bandgap voltage vs V~CC~"
{"x": {"label": "VCC", "unit": "V", "range": [1.5, 5.5]},
 "y": {"label": "VBG", "unit": "V", "range": [1.121, 1.139]},
 "series": [
   {"label": "25 °C", "points": [[1.8, 1.137], [3, 1.1368], [4, 1.1364], [4.5, 1.1362], [5, 1.1355], [5.5, 1.1339]]},
   {"label": "85 °C", "points": [[1.8, 1.1304], [3, 1.1299], [4, 1.1297], [4.8, 1.1295], [5, 1.1288], [5.5, 1.1265]]},
   {"label": "-40 °C", "points": [[1.8, 1.1271], [3, 1.1268], [4, 1.1266], [4.5, 1.1264], [5, 1.126], [5.5, 1.1257]]},
   {"label": "105 °C", "points": [[1.8, 1.1262], [2.5, 1.1268], [3, 1.1271], [3.5, 1.1274], [4, 1.1271], [4.5, 1.1265], [5, 1.1255], [5.5, 1.1232]]}
 ]}
```

### Supply current of I/O modules

Typical extra current, in active and idle mode, of each module that a
PRR bit turns off (ATmega328P; the source has the same two tables for
each of the other seven devices).

```table id=tc-prr-abs caption="Additional current per I/O module, ATmega328P (typical)" values=Typ
| PRR bit | V~CC~ = 2 V, f = 1 MHz | V~CC~ = 3 V, f = 4 MHz | V~CC~ = 5 V, f = 8 MHz | Unit |
|---|--:|--:|--:|---|
| PRUSART0 | 3.20 | 22.17 | 100.25 | µA |
| PRTWI | 7.34 | 46.55 | 199.25 | µA |
| PRTIM2 | 7.34 | 50.79 | 224.25 | µA |
| PRTIM1 | 6.19 | 41.25 | 176.25 | µA |
| PRTIM0 | 1.89 | 14.28 | 61.13 | µA |
| PRSPI | 6.94 | 43.84 | 186.50 | µA |
| PRADC | 8.66 | 61.80 | 295.38 | µA |
```

```table id=tc-prr-pct caption="Additional current per I/O module relative to the module-less supply current, ATmega328P (typical)" values=Typ
| PRR bit | Active, external clock | Idle, external clock | Unit |
|---|--:|--:|---|
| PRUSART0 | 1.4 | 7.8 | % |
| PRTWI | 3.0 | 16.6 | % |
| PRTIM2 | 3.3 | 17.8 | % |
| PRTIM1 | 2.7 | 14.5 | % |
| PRTIM0 | 0.9 | 4.8 | % |
| PRSPI | 2.9 | 15.7 | % |
| PRADC | 4.1 | 22.1 | % |
```

The percentages scale the active or idle current of the curves to
other V~CC~ and frequencies. Example: idle at V~CC~ = 2.0 V, 1 MHz,
with Timer/Counter1, the ADC and SPI on:

```equation
I~CC total~ ≈ 0.045 mA × (1 + 0.145 + 0.221 + 0.157) ≈ 0.069 mA
```

## Package

```table id=pkg caption="Packages (Microchip drawings) [1]"
| Package | Type | Drawing | Pins | Body | Pitch | Overall height (A) |
|---|---|---|--:|---|---|---|
| 32A | 32-lead plastic thin quad flatpack (TQFP), gull wing | C04-074B | 32 | 7.00 × 7.00 mm (9.00 × 9.00 mm over leads) | 0.80 mm | 1.20 mm max |
| 28M1 | 28-pad very thin plastic quad flat no-lead (VQFN), 2.40 mm exposed pad, saw singulated | C04-21388 rev A | 28 | 4.00 × 4.00 mm | 0.45 mm | 0.80 / 0.90 / 1.00 mm |
| 32M1-A | 32-pad thin plastic quad flat no-lead (VQFN), 3.10 mm exposed pad, punch singulated | C04-21400 rev A | 32 | 5.00 × 5.00 mm | 0.50 mm | 0.80 / 0.85 / 1.00 mm |
| 28P3 | 28-lead skinny plastic dual in-line (SPDIP), 0.300 in | C04-070B | 28 | 1.365 × 0.285 in nominal | 0.100 in | 0.200 in max |

[1] Height as min / nom / max. Dimensioning and tolerancing per ASME Y14.5M.
```

```table id=pkg-32a caption="32A (TQFP) dimensions [1]"
| Dimension | Min | Nom | Max | Unit |
|---|--:|--:|--:|---|
| Standoff (A1) | 0.05 | | 0.15 | mm |
| Molded package thickness (A2) | 0.95 | 1.00 | 1.05 | mm |
| Foot length (L) | 0.45 | 0.60 | 0.75 | mm |
| Footprint (L1), reference | | 1.00 | | mm |
| Foot angle (φ) | 0 | 3.5 | 7 | ° |
| Lead thickness (c) | 0.09 | | 0.20 | mm |
| Lead width (b) | 0.30 | 0.37 | 0.45 | mm |
| Mold draft angle, top and bottom (α, β) | 11 | 12 | 13 | ° |

[1] Body dimensions D1 and E1 exclude mold flash or protrusions, at most 0.25 mm per side.
```

```table id=pkg-qfn caption="28M1 and 32M1-A (VQFN) dimensions"
| Dimension | 28M1 | < | < | 32M1-A | < | < | Unit |
| ^ | Min | Nom | Max | Min | Nom | Max | ^ |
|---|--:|--:|--:|--:|--:|--:|---|
| Standoff (A1) | 0.00 | 0.02 | 0.05 | 0.00 | 0.02 | 0.05 | mm |
| Mold cap thickness (A2) | | | | | 0.65 | 0.70 | mm |
| Terminal thickness (A3), reference | | 0.20 | | | 0.20 | | mm |
| Mold cap length and width (D1, E1) | | | | | 4.75 | | mm |
| Exposed pad length and width (D2, E2) | 2.35 | 2.40 | 2.45 | 2.95 | 3.10 | 3.25 | mm |
| Terminal width (b) | 0.17 | 0.22 | 0.27 | 0.18 | 0.23 | 0.30 | mm |
| Terminal length (L) | 0.35 | 0.40 | 0.45 | 0.30 | 0.40 | 0.50 | mm |
| Terminal to exposed pad (K) | | 0.20 | | 0.20 | | | mm |
```

```table id=pkg-land caption="Recommended land patterns [1]"
| Dimension | 32A | 28M1 | 32M1-A | Unit |
|---|--:|--:|--:|---|
| Contact pitch (E) | 0.80 | 0.45 | 0.50 | mm |
| Contact pad spacing (C1, C2), nom | 8.50 | 4.00 | 4.90 | mm |
| Contact pad width (X1), max | 0.55 [2] | 0.25 | 0.30 | mm |
| Contact pad length (Y1), max | 1.60 | 0.80 | 0.85 | mm |
| Optional center pad width and length (X2, Y2), max | | 2.45 | 3.25 | mm |
| Contact pad to center pad (G1), min | | 0.38 | 0.40 | mm |
| Distance between pads (G, G2), min | 0.25 | 0.20 | 0.20 | mm |
| Thermal via diameter (V), nom | | 0.33 | 0.33 | mm |
| Thermal via pitch (EV), nom | | 1.20 | 0.20 [3] | mm |

[1] Thermal vias, if used, should be filled or tented to avoid solder loss during reflow.
[2] The source labels the 32A pad rows "(X28)" for a 32-lead package.
[3] As printed; the 28M1 pattern gives 1.20 mm.
```

```table id=pkg-28p3 caption="28P3 (SPDIP) dimensions [1]"
| Dimension | Min | Nom | Max | Unit |
|---|--:|--:|--:|---|
| Molded package thickness (A2) | 0.120 | 0.135 | 0.150 | in |
| Base to seating plane (A1) | 0.015 | | | in |
| Shoulder to shoulder width (E) | 0.290 | 0.310 | 0.335 | in |
| Molded package width (E1) | 0.240 | 0.285 | 0.295 | in |
| Overall length (D) | 1.345 | 1.365 | 1.400 | in |
| Tip to seating plane (L) | 0.110 | 0.130 | 0.150 | in |
| Lead thickness (c) | 0.008 | 0.010 | 0.015 | in |
| Upper lead width (b1) | 0.040 | 0.050 | 0.070 | in |
| Lower lead width (b) | 0.014 | 0.018 | 0.022 | in |
| Overall row spacing (eB) | | | 0.430 | in |

[1] D and E1 exclude mold flash or protrusions, at most 0.010 in per side.
```

## Ordering information

All devices: 20 MHz speed grade (see [speed grades](#recommended-operating-conditions)),
1.8 V to 5.5 V supply, Pb-free and halide free (RoHS). Each code is
also offered in tape and reel with an `R` appended (`ATmega328P-AUR`).
The devices can also be supplied in wafer form.

```table id=ord caption="Ordering codes"
| Device | -40 °C to 85 °C | -40 °C to 105 °C |
|---|---|---|
| ATmega48A [1] | ATmega48A-AU, -MMH [2], -MU, -PU | |
| ATmega48PA | ATmega48PA-AU, -MMH [2], -MU, -PU | ATmega48PA-AN, -MMN [2], -MN, -PN |
| ATmega88A [1] | ATmega88A-AU, -MMH [2], -MU, -PU | |
| ATmega88PA | ATmega88PA-AU, -MMH [2], -MU, -PU | ATmega88PA-AN, -MMN [2], -MN, -PN |
| ATmega168A [1] | ATmega168A-AU, -MMH [2], -MU, -PU | |
| ATmega168PA | ATmega168PA-AU, -MMH [2], -MU, -PU | ATmega168PA-AN, -MN, -PN |
| ATmega328 [1] | ATmega328-AU, -MMH [2], -MU, -PU | |
| ATmega328P | ATmega328P-AU, -MMH [2], -MU, -PU | ATmega328P-AN, -MN, -PN |

[1] Not offered for -40 °C to 105 °C; use the PA (or 328P) device.
[2] NiPdAu lead finish.
```

```table id=ord-key caption="Package code in the ordering code"
| Suffix | Package |
|---|---|
| -AU, -AN | 32A: 32-lead TQFP, 7 × 7 mm, 1.0 mm thick |
| -MMH, -MMN | 28M1: 28-pad VQFN, 4 × 4 × 1.0 mm, 0.45 mm pitch |
| -MU, -MN | 32M1-A: 32-pad VQFN, 5 × 5 × 1.0 mm, 0.50 mm pitch |
| -PU, -PN | 28P3: 28-lead SPDIP, 0.300 in wide |
```

## Conversion notes

- Pages converted: 1, 12–325 and 621–645 (text and tables); 326–620 are typical characteristics, of which 14 figures feed the 12 kept curves. Dropped as standard: feature lists, table of contents, revision history (646–649), legal pages (650–653), package outline drawings (dimensions kept as tables).
- Curves: chapters 31–35 hold 563 typical-characteristic figures; 549 dropped. Kept (ATmega328P, 31.8): active and idle I~CC~ vs f (1–20 MHz), power-down I~CC~ vs V~CC~ (watchdog off/on), V~OL~ vs sink and V~OH~ vs source current at 3 V and 5 V, 8 MHz RC vs V~CC~ and temperature, BOD thresholds vs temperature (figures 31-362/363/364 merged into one curve, six series), bandgap vs V~CC~ (the source has no bandgap-vs-temperature figure). Dropped: low-frequency and RC-oscillator supply currents, power-save/standby, pull-up currents, input thresholds and hysteresis, watchdog oscillator, OSCCAL, module and reset currents, reset pulse width, every curve of the other seven devices, all of chapters 32–35 (105 °C). Digitized from 110 dpi renders to about 2 % of full scale; power-down and bandgap lines are least certain. The ATmega328P supply-current-per-module tables are kept as facts; the 14 equivalent per-device tables are dropped with the curves.
- Code examples dropped (EEPROM, watchdog, interrupt move, port set-up, 16-bit access, SPI, USART, TWI, both boot-loader listings, vector set-up); timed sequences kept as numbered steps.
- Block diagrams, pin-logic schematics, connection figures, flash-addressing and memory-section figures, ADC accuracy figures and the TWI per-mode state-flow figures are described in text or replaced by tables.
- Redrawn as WaveDrom (digital approximations): reset timing (V~CC~ ramps drawn as "V~CC~ ≥ threshold" levels), pin change and port synchronizer timing (shaded transparent-latch region as x), timer cycle timing (no-prescaler versions; prescaled ones in a sentence), SPI and MSPIM transfer formats, SPI master/slave timing (edges labelled with the source's parameter numbers 1–18, which have no symbols), USART frame, TWI conditions and timing, ADC conversion timing (an "event" lane instead of cycle numbers), parallel and serial programming waveforms (RDY/!{BSY} named RDY/BSY#). Not redrawn: timer mode histograms (WaveDrom can't draw a sawtooth/triangle counter; the behavior is in prose and mode tables).
- Merged tables: OC0A/OC0B, OC1A/OC1B and OC2A/OC2B COM tables; low power and full swing crystal start-up tables; RC and 128 kHz start-up tables; per-device boot-size, RWW, Z-pointer, fuse-byte, page-size, supply-current and ordering tables (one table each with a Device column or column group); the TWCR "value to write" mini-tables; the 105 °C absolute maximum ratings (identical).
- Instruction set: the source summary has no opcode patterns, so that column is absent (not invented); Words derived (2 for JMP, CALL, LDS, STS); indirect LD/ST rows folded to X/Y/Z rows and SEx/CLx pairs to one row each. The addressing-mode table is derived from the data-memory text (the source has none).
- Register summary: 138 reserved-address rows and the page column dropped; Access and Reset added per register (from the bit pictures), "per bit" where the bits differ. Reserved bits left out of the bit tables.
- MCUCR, GTCCR, ADCSRB and SPMCSR are described in two or three subsections (each by the bits that belong there); the "one register, described once" rule can't hold for registers whose bits belong to several peripherals.
- The 85 °C electrical chapter (29.2) of this revision has only supply-current tables; the common DC characteristics come from the 105 °C chapter (table 30-1) and are used for all devices.
- Source errors and inconsistencies (kept as printed unless noted): JMP/CALL noted as only in ATmega168PA/328P (they are in all 16K/32K parts); 8.2 says the boot section exists only in 88PA/168PA; "five" sleep modes in 2 and 10.11 vs six listed; SM = 111 called "External Standby"; EEPROM write time 3.4 ms (table 8-1) vs 3.3 ms (8-2); low-frequency crystal text gives CKSEL 0110/0111 (tables: 0100/0101, used); 128 kHz text CKSEL "11" (0011); the prescaler text points to the wrong table; PD2 DIEOE "PCIE1" (should be PCIE2); PD3 text calls OC2B a Timer/Counter0 output; PD7 heading lists OC2B; port D tables print "PUO"; tables 18-2/18-3 say OC0A for OC2A; "MGM2:0" and wrong mode numbers in the Timer/Counter2 PWM text (converted as modes 1 and 5); FOC2B mentions OCR2B as TOP; TIFR2 text names "TOIE2A"; ICNC1 "four oscillator cycles" vs "four system clock cycles"; table 20-3 "104,35"; MSPIM UCSR0A/B reset values contradict the USART ones; the ADC overview says "Port A" (Port C plus ADC6/7); slave transmitter entered with the "write bit" (read bit); "Salve Address"; DWDR has no address in the source (summary lists 0x31 as reserved); ATmega48 extended fuse read labelled FHB; serial instruction "0000 000aa" (nine digits); ordering note 6 copy-pasted from ATmega48A; 28M1 exposed pad 2.45 mm (land pattern) vs 2.40 mm (outline); the 328P idle example reads 0.055 mA and computes with 0.045 mA; table 31-1 labelled ATmega48PA in the 48A section.
- Format bends: a pending Pinout table has only number columns, so port alternate functions are in the pins block; SPI timing uses a "No." column instead of Symbol; bold "< 0.5 % error" baud settings carried as a note; Errata subsection only points to the separate errata document DS80000855 (this PDF lists none); package land patterns kept as a table since no catalog footprint exists yet.
